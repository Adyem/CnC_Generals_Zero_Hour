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
//
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
//
//                                                                                                                             /
//
////////////////////////////////////////////////////////////////////////////////

// FILE: SfmlKeyboardBridge.h //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//  SFML keyboard bridge that adapts sf::Event key input to the legacy
//  GameClient::Keyboard API.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef SFML_KEYBOARD_BRIDGE_H
#define SFML_KEYBOARD_BRIDGE_H

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <deque>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Keyboard.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////
namespace sf
{
        class Event;
}

namespace sfml_platform
{

class SfmlKeyboardBridge : public Keyboard
{
public:

        SfmlKeyboardBridge();
        virtual ~SfmlKeyboardBridge();

        virtual Bool getCapsState( void );

        void handleEvent( const sf::Event &event );

protected:

        virtual void getKey( KeyboardIO *key );

private:

        void enqueueKey( UnsignedByte key, UnsignedShort state );

        std::deque< KeyboardIO > m_pendingEvents;
        UnsignedInt m_nextSequence;
        Bool m_capsLockActive;
        Bool m_capsLockKeyDown;
};

SfmlKeyboardBridge *GetActiveKeyboardBridge();
Keyboard *CreateSfmlKeyboard();

} // namespace sfml_platform

#endif // SFML_KEYBOARD_BRIDGE_H

