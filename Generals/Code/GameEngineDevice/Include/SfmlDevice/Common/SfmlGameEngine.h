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
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// FILE: SfmlGameEngine.h /////////////////////////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   SFML-aware variant of the legacy Win32 game engine.  The implementation
//   relies on the message pump override installed by the SFML bootstrap rather
//   than querying Win32 window state directly.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SFMLGAMEENGINE_H_
#define __SFMLGAMEENGINE_H_

#include "Win32Device/Common/Win32GameEngine.h"

class SfmlGameEngine : public Win32GameEngine
{
public:
        SfmlGameEngine();
        virtual ~SfmlGameEngine();

        virtual void update( void );

protected:
        virtual void serviceWindowsOS( void );
};

GameEngine* CreateSfmlGameEngine();

#endif  // __SFMLGAMEENGINE_H_
