/*
**      Command & Conquer Generals(tm)
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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/msgloop.cpp                               $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Windows_Message_Handler -- Handles windows message.                                       *
 *   Add_Modeless_Dialog -- Adds a modeless dialog hook to the message handler.                *
 *   Remove_Modeless_Dialog -- Removes the dialog hook from the message tracking handler.      *
 *   Add_Accelerator -- Registers an accelerator callback with the message handler.            *
 *   Remove_Accelerator -- Removes an accelerator from the message processor.                  *
 *   Push_Message_Interceptor -- Registers an interceptor that can consume events.             *
 *   Remove_Message_Interceptor -- Removes a previously registered interceptor.                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "msgloop.h"
#include "win.h"

#include <mutex>
#include <utility>

namespace
{

std::mutex &Message_Loop_Mutex()
{
        static std::mutex mutex;
        return mutex;
}

} // namespace

namespace WWLib
{

void Windows_Message_Handler()
{
        WindowSystem::Pump_Events();
}

SfmlMessagePump::DialogHookId Add_Modeless_Dialog(SfmlMessagePump::DialogHook hook)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        return WindowSystem::Message_Pump().AddDialogHook(std::move(hook));
}

void Remove_Modeless_Dialog(SfmlMessagePump::DialogHookId id)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        WindowSystem::Message_Pump().RemoveDialogHook(id);
}

SfmlMessagePump::AcceleratorId Add_Accelerator(const AcceleratorKey &key, SfmlMessagePump::AcceleratorCallback callback)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        return WindowSystem::Message_Pump().RegisterAccelerator(key, std::move(callback));
}

void Remove_Accelerator(SfmlMessagePump::AcceleratorId id)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        WindowSystem::Message_Pump().UnregisterAccelerator(id);
}

SfmlMessagePump::InterceptorId Push_Message_Interceptor(SfmlMessagePump::Interceptor interceptor)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        return WindowSystem::Message_Pump().PushInterceptor(std::move(interceptor));
}

void Remove_Message_Interceptor(SfmlMessagePump::InterceptorId id)
{
        std::lock_guard<std::mutex> lock(Message_Loop_Mutex());
        WindowSystem::Message_Pump().RemoveInterceptor(id);
}

} // namespace WWLib
