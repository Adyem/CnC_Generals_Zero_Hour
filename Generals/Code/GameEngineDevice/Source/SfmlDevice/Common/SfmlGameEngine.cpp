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
//                                                                                                                             /
/
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
/
//                                                                                                                             /
/
////////////////////////////////////////////////////////////////////////////////

// FILE: SfmlGameEngine.cpp ///////////////////////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Implementation of the SFML-aware game engine device layer.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "SfmlDevice/Common/SfmlGameEngine.h"

#include "Common/GameEngine.h"
#include "GameLogic/GameLogic.h"
#include "GameNetwork/LANAPICallbacks.h"

#include <chrono>
#include <thread>

SfmlGameEngine::SfmlGameEngine() = default;

SfmlGameEngine::~SfmlGameEngine() = default;

void SfmlGameEngine::update( void )
{
        GameEngine::update();

        if( !isActive() )
        {
                while( !isActive() )
                {
                        std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
                        serviceWindowsOS();

                        if( TheLAN != NULL )
                        {
                                TheLAN->setIsActive( isActive() );
                                TheLAN->update();
                        }

                        if( getQuitting() || TheGameLogic->isInInternetGame() || TheGameLogic->isInLanGame() )
                        {
                                break;
                        }
                }
        }

        serviceWindowsOS();
}

void SfmlGameEngine::serviceWindowsOS( void )
{
        WindowsMessagePumpOverride pump = GetWindowsMessagePumpOverride();
        if( pump != NULL )
        {
                pump();
                return;
        }

        Win32GameEngine::serviceWindowsOS();
}

GameEngine* CreateSfmlGameEngine()
{
        return NEW SfmlGameEngine;
}
