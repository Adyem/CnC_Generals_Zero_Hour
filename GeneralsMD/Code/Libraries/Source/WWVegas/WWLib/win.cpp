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

#include "always.h"
#include "win.h"
#include "wwdebug.h"

#include <SFML/Window/Window.hpp>

#include <atomic>
#include <memory>
#include <mutex>

HINSTANCE ProgramInstance = nullptr;
HWND MainWindow = nullptr;
bool GameInFocus = false;

namespace
{

std::shared_ptr<WWLib::SfmlMessagePump> &Message_Pump_Instance()
{
        static std::shared_ptr<WWLib::SfmlMessagePump> pump;
        return pump;
}

std::mutex &Message_Pump_Mutex()
{
        static std::mutex mutex;
        return mutex;
}

std::atomic<bool> &Game_Focus_Flag()
{
        static std::atomic<bool> focused{false};
        return focused;
}

std::shared_ptr<WWLib::SfmlMessagePump> Clone_Pump()
{
        std::lock_guard<std::mutex> lock(Message_Pump_Mutex());
        return Message_Pump_Instance();
}

} // namespace

namespace WWLib
{

void WindowSystem::Initialize(std::unique_ptr<SfmlMessagePump> pump)
{
        std::lock_guard<std::mutex> lock(Message_Pump_Mutex());
        Message_Pump_Instance() = std::shared_ptr<SfmlMessagePump>(std::move(pump));
        Game_Focus_Flag() = Message_Pump_Instance() ? Message_Pump_Instance()->HasFocus() : false;
        MainWindow = Message_Pump_Instance() ? reinterpret_cast<HWND>(&Message_Pump_Instance()->GetWindow()) : nullptr;
        GameInFocus = Game_Focus_Flag().load();
}

void WindowSystem::Shutdown()
{
        std::lock_guard<std::mutex> lock(Message_Pump_Mutex());
        Message_Pump_Instance().reset();
        Game_Focus_Flag() = false;
        MainWindow = nullptr;
        GameInFocus = false;
}

bool WindowSystem::Is_Initialized()
{
        std::lock_guard<std::mutex> lock(Message_Pump_Mutex());
        return static_cast<bool>(Message_Pump_Instance());
}

SfmlMessagePump &WindowSystem::Message_Pump()
{
        std::lock_guard<std::mutex> lock(Message_Pump_Mutex());
        if (!Message_Pump_Instance()) {
                Message_Pump_Instance() = std::make_shared<SfmlMessagePump>();
        }
        MainWindow = reinterpret_cast<HWND>(&Message_Pump_Instance()->GetWindow());
        return *Message_Pump_Instance();
}

sf::Window &WindowSystem::Window()
{
        return Message_Pump().GetWindow();
}

void WindowSystem::Pump_Events()
{
        std::shared_ptr<SfmlMessagePump> pump = Clone_Pump();
        if (pump) {
                pump->PumpEvents();
                Game_Focus_Flag() = pump->HasFocus();
                GameInFocus = Game_Focus_Flag().load();
        }
}

void WindowSystem::Set_Game_Focus(bool has_focus)
{
        Game_Focus_Flag() = has_focus;
        GameInFocus = has_focus;
}

bool WindowSystem::Has_Game_Focus()
{
        return Game_Focus_Flag().load();
}

} // namespace WWLib
