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

#include "PreRTS.h"

#include "GameNetwork/GameSpyOverlay.h"

#include "GameClient/WOLMenuStubs.h"

namespace
{
        GameWindow *s_messageBoxWindow = NULL;
}

void ClearGSMessageBoxes(void)
{
        if (s_messageBoxWindow)
        {
                TheWindowManager->winDestroy(s_messageBoxWindow);
                s_messageBoxWindow = NULL;
        }
}

void GSMessageBoxOk(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc okFunc)
{
        ClearGSMessageBoxes();
        s_messageBoxWindow = MessageBoxOk(title, message, okFunc);
}

void GSMessageBoxOkCancel(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc okFunc, GameWinMsgBoxFunc cancelFunc)
{
        ClearGSMessageBoxes();
        s_messageBoxWindow = MessageBoxOkCancel(title, message, okFunc, cancelFunc);
}

void GSMessageBoxYesNo(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc yesFunc, GameWinMsgBoxFunc noFunc)
{
        ClearGSMessageBoxes();
        s_messageBoxWindow = MessageBoxYesNo(title, message, yesFunc, noFunc);
}

void RaiseGSMessageBox(void)
{
        if (s_messageBoxWindow)
        {
                s_messageBoxWindow->winBringToTop();
        }
}

void GameSpyOpenOverlay(GSOverlayType)
{
        WOLMenuStubs::ResetOnlineState();
        WOLMenuStubs::ShowOfflineDialog();
}

void GameSpyCloseOverlay(GSOverlayType)
{
}

void GameSpyCloseAllOverlays(void)
{
}

Bool GameSpyIsOverlayOpen(GSOverlayType)
{
        return FALSE;
}

void GameSpyToggleOverlay(GSOverlayType overlay)
{
        GameSpyOpenOverlay(overlay);
}

void GameSpyUpdateOverlays(void)
{
}

void ReOpenPlayerInfo(void)
{
}

void CheckReOpenPlayerInfo(void)
{
}
