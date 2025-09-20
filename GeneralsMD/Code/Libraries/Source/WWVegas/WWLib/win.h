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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/win.h                                  $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 10/16/01 2:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef WIN_H
#define WIN_H

#include <compat/win_compat.h>

#include "sfml_message_pump.h"

#include <memory>

namespace sf {
class Window;
} // namespace sf

namespace WWLib {

class WindowSystem
{
public:
        static void Initialize(std::unique_ptr<SfmlMessagePump> pump);
        static void Shutdown();

        static bool Is_Initialized();

        static SfmlMessagePump &Message_Pump();
        static sf::Window &Window();

        static void Pump_Events();

        static void Set_Game_Focus(bool has_focus);
        static bool Has_Game_Focus();
};

} // namespace WWLib

extern HINSTANCE ProgramInstance;
extern HWND MainWindow;
extern bool GameInFocus;

#endif // WIN_H
