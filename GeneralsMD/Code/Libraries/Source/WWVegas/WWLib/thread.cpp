/*
**      Command & Conquer Generals Zero Hour(tm)
**      Copyright 2025 Electronic Arts Inc.
**
**      This program is free software: you can redistribute it and/or modify
**      it under the terms of the GNU General Public License as published by
**      the Free Software Foundation, either version 3 of the License, or
**      (at your option) any later version.
**
**      This program is distributed in the hope that it will be useful,
**      but WITHOUT ANY WARRANTY; without even the implied warranty of
**      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**      GNU General Public License for more details.
**
**      You should have received a copy of the GNU General Public License
**      along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "thread.h"
#include "except.h"
#include "wwdebug.h"

#include <chrono>
#include <cstring>
#include <functional>

#if defined(_WIN32)
#include <windows.h>
#endif

ThreadClass::ThreadClass(const char *thread_name, ExceptionHandlerType exception_handler) : running(false), thread_active(false), thread_priority(0), ThreadID(0), ExceptionHandler(exception_handler)
{
        if (thread_name) {
                assert(strlen(thread_name) < sizeof(ThreadName) - 1);
                strcpy(ThreadName, thread_name);
        } else {
                strcpy(ThreadName, "No name");
        }
}

ThreadClass::~ThreadClass()
{
        Stop();
}

void ThreadClass::Internal_Thread_Function()
{
        running = true;
        ThreadID = _Get_Current_Thread_ID();
        Register_Thread_ID(ThreadID, ThreadName);

        struct Cleanup
        {
                ThreadClass *self;
                ~Cleanup()
                {
                        self->running = false;
                        {
                                std::lock_guard<std::mutex> lock(self->thread_mutex);
                                self->thread_active = false;
                        }
                        self->thread_cv.notify_all();
                        Unregister_Thread_ID(self->ThreadID, self->ThreadName);
                        self->ThreadID = 0;
                }
        } cleanup{this};

#if defined(_WIN32)
        if (ExceptionHandler != NULL) {
                __try {
                        Thread_Function();
                } __except(ExceptionHandler(GetExceptionCode(), GetExceptionInformation())) {}
        } else {
                Thread_Function();
        }
#else
        Thread_Function();
#endif
}

void ThreadClass::Execute()
{
        std::lock_guard<std::mutex> lock(thread_mutex);
        WWASSERT(!thread_handle.joinable());
        thread_active = true;
        try {
                thread_handle = std::thread(&ThreadClass::Internal_Thread_Function, this);
        } catch (...) {
                thread_active = false;
                thread_cv.notify_all();
                throw;
        }

        WWDEBUG_SAY(("ThreadClass::Execute: Started thread %s, thread hash is %X\n", ThreadName,
                static_cast<unsigned>(std::hash<std::thread::id>{}(thread_handle.get_id()))));
}

void ThreadClass::Set_Priority(int priority)
{
        thread_priority = priority;
}

void ThreadClass::Stop(unsigned ms)
{
        running = false;
        std::thread local_thread;
        {
                std::unique_lock<std::mutex> lock(thread_mutex);
                if (!thread_handle.joinable()) {
                        thread_active = false;
                        return;
                }

                auto predicate = [this]() { return !thread_active; };
                bool finished = true;
                if (ms == 0) {
                        thread_cv.wait(lock, predicate);
                } else {
                        finished = thread_cv.wait_for(lock, std::chrono::milliseconds(ms), predicate);
                }
                WWASSERT(finished);

                local_thread = std::move(thread_handle);
                thread_active = false;
        }

        if (local_thread.joinable()) {
                local_thread.join();
        }
}

void ThreadClass::Sleep_Ms(unsigned ms)
{
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void ThreadClass::Switch_Thread()
{
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// Return calling thread's unique thread id
unsigned ThreadClass::_Get_Current_Thread_ID()
{
        const auto value = std::hash<std::thread::id>{}(std::this_thread::get_id());
        return static_cast<unsigned>(value ^ (value >> 32));
}

bool ThreadClass::Is_Running()
{
        return running.load();
}
