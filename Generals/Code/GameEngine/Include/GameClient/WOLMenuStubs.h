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

////////////////////////////////////////////////////////////////////////////////
//
//  (c) 2001-2003 Electronic Arts Inc.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __WOL_MENU_STUBS_H__
#define __WOL_MENU_STUBS_H__

#include "Common/GameEngine.h"
#include "GameClient/GameWindow.h"
#include "GameClient/MessageBox.h"
#include "GameClient/Shell.h"
#include "GameClient/WindowLayout.h"
#include "GameNetwork/Online/OnlineServices.h"

namespace WOLMenuStubs
{
        inline void ShowOfflineDialog()
        {
                UnicodeString title;
                title.translate("Online Services Offline");

                UnicodeString body;
                body.translate("Multiplayer matchmaking is temporarily unavailable in this build.");

                MessageBoxOk(title, body, NULL);
        }

        inline void ResetOnlineState()
        {
                if (OnlineServices::GetServices())
                {
                        OnlineServices::GetServices()->reset();
                }
        }

        inline void PopShellIfPossible()
        {
                if (TheShell)
                {
                        TheShell->popImmediate();
                }
        }

        inline WindowMsgHandledType EatWindowMessage(GameWindow *, UnsignedInt, UnsignedInt, UnsignedInt)
        {
                return MSG_HANDLED;
        }

        inline WindowMsgHandledType EatSystemMessage(GameWindow *, UnsignedInt, WindowMsgData, WindowMsgData)
        {
                return MSG_HANDLED;
        }
}

#define DEFINE_WOL_MENU_STUB(prefix) \
        void prefix##Init(WindowLayout *, void *) \
        { \
                WOLMenuStubs::ResetOnlineState(); \
                WOLMenuStubs::ShowOfflineDialog(); \
                WOLMenuStubs::PopShellIfPossible(); \
        } \
        void prefix##Shutdown(WindowLayout *, void *) \
        { \
        } \
        void prefix##Update(WindowLayout *, void *) \
        { \
        } \
        WindowMsgHandledType prefix##Input(GameWindow *window, UnsignedInt msg, UnsignedInt param1, UnsignedInt param2) \
        { \
                return WOLMenuStubs::EatWindowMessage(window, msg, param1, param2); \
        } \
        WindowMsgHandledType prefix##System(GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2) \
        { \
                return WOLMenuStubs::EatSystemMessage(window, msg, mData1, mData2); \
        }

#define DEFINE_WOL_MENU_WITH_OVERLAY_STUB(prefix) \
        DEFINE_WOL_MENU_STUB(prefix) \
        void prefix##RCMenuInit(WindowLayout *, void *) \
        { \
                WOLMenuStubs::ShowOfflineDialog(); \
        } \
        WindowMsgHandledType prefix##RCMenuSystem(GameWindow *, UnsignedInt, WindowMsgData, WindowMsgData) \
        { \
                return MSG_HANDLED; \
        }

#endif // __WOL_MENU_STUBS_H__
