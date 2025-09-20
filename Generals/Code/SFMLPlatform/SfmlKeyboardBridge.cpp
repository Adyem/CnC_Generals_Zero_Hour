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

// FILE: SfmlKeyboardBridge.cpp ///////////////////////////////////////////////

#include "SfmlKeyboardBridge.h"

#include "GameClient/KeyDefs.h"

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

namespace sfml_platform
{

namespace
{

SfmlKeyboardBridge *g_activeKeyboardBridge = NULL;

UnsignedByte translateKeyCode( sf::Keyboard::Key key )
{
        using sf::Keyboard;

        switch( key )
        {
        case Keyboard::A: return KEY_A;
        case Keyboard::B: return KEY_B;
        case Keyboard::C: return KEY_C;
        case Keyboard::D: return KEY_D;
        case Keyboard::E: return KEY_E;
        case Keyboard::F: return KEY_F;
        case Keyboard::G: return KEY_G;
        case Keyboard::H: return KEY_H;
        case Keyboard::I: return KEY_I;
        case Keyboard::J: return KEY_J;
        case Keyboard::K: return KEY_K;
        case Keyboard::L: return KEY_L;
        case Keyboard::M: return KEY_M;
        case Keyboard::N: return KEY_N;
        case Keyboard::O: return KEY_O;
        case Keyboard::P: return KEY_P;
        case Keyboard::Q: return KEY_Q;
        case Keyboard::R: return KEY_R;
        case Keyboard::S: return KEY_S;
        case Keyboard::T: return KEY_T;
        case Keyboard::U: return KEY_U;
        case Keyboard::V: return KEY_V;
        case Keyboard::W: return KEY_W;
        case Keyboard::X: return KEY_X;
        case Keyboard::Y: return KEY_Y;
        case Keyboard::Z: return KEY_Z;

        case Keyboard::Num0: return KEY_0;
        case Keyboard::Num1: return KEY_1;
        case Keyboard::Num2: return KEY_2;
        case Keyboard::Num3: return KEY_3;
        case Keyboard::Num4: return KEY_4;
        case Keyboard::Num5: return KEY_5;
        case Keyboard::Num6: return KEY_6;
        case Keyboard::Num7: return KEY_7;
        case Keyboard::Num8: return KEY_8;
        case Keyboard::Num9: return KEY_9;

        case Keyboard::Escape: return KEY_ESC;
        case Keyboard::LControl: return KEY_LCTRL;
        case Keyboard::LShift: return KEY_LSHIFT;
        case Keyboard::LAlt: return KEY_LALT;
        case Keyboard::RControl: return KEY_RCTRL;
        case Keyboard::RShift: return KEY_RSHIFT;
        case Keyboard::RAlt: return KEY_RALT;
        case Keyboard::Menu: return KEY_SYSREQ;

        case Keyboard::LBracket: return KEY_LBRACKET;
        case Keyboard::RBracket: return KEY_RBRACKET;
        case Keyboard::Semicolon: return KEY_SEMICOLON;
        case Keyboard::Comma: return KEY_COMMA;
        case Keyboard::Period: return KEY_PERIOD;
        case Keyboard::Quote: return KEY_APOSTROPHE;
        case Keyboard::Slash: return KEY_SLASH;
        case Keyboard::Backslash: return KEY_BACKSLASH;
        case Keyboard::Tilde: return KEY_TICK;
        case Keyboard::Equal: return KEY_EQUAL;
        case Keyboard::Hyphen: return KEY_MINUS;

        case Keyboard::Space: return KEY_SPACE;
        case Keyboard::Enter: return KEY_ENTER;
        case Keyboard::Backspace: return KEY_BACKSPACE;
        case Keyboard::Tab: return KEY_TAB;
        case Keyboard::PageUp: return KEY_PGUP;
        case Keyboard::PageDown: return KEY_PGDN;
        case Keyboard::End: return KEY_END;
        case Keyboard::Home: return KEY_HOME;
        case Keyboard::Insert: return KEY_INS;
        case Keyboard::Delete: return KEY_DEL;

        case Keyboard::Add: return KEY_KPPLUS;
        case Keyboard::Subtract: return KEY_KPMINUS;
        case Keyboard::Multiply: return KEY_KPSTAR;
        case Keyboard::Divide: return KEY_KPSLASH;

        case Keyboard::Left: return KEY_LEFT;
        case Keyboard::Right: return KEY_RIGHT;
        case Keyboard::Up: return KEY_UP;
        case Keyboard::Down: return KEY_DOWN;

        case Keyboard::Numpad0: return KEY_KP0;
        case Keyboard::Numpad1: return KEY_KP1;
        case Keyboard::Numpad2: return KEY_KP2;
        case Keyboard::Numpad3: return KEY_KP3;
        case Keyboard::Numpad4: return KEY_KP4;
        case Keyboard::Numpad5: return KEY_KP5;
        case Keyboard::Numpad6: return KEY_KP6;
        case Keyboard::Numpad7: return KEY_KP7;
        case Keyboard::Numpad8: return KEY_KP8;
        case Keyboard::Numpad9: return KEY_KP9;

        case Keyboard::F1: return KEY_F1;
        case Keyboard::F2: return KEY_F2;
        case Keyboard::F3: return KEY_F3;
        case Keyboard::F4: return KEY_F4;
        case Keyboard::F5: return KEY_F5;
        case Keyboard::F6: return KEY_F6;
        case Keyboard::F7: return KEY_F7;
        case Keyboard::F8: return KEY_F8;
        case Keyboard::F9: return KEY_F9;
        case Keyboard::F10: return KEY_F10;
        case Keyboard::F11: return KEY_F11;
        case Keyboard::F12: return KEY_F12;

        case Keyboard::Pause: return KEY_SCROLL;

        default:
                break;
        }

        return KEY_NONE;
}

} // namespace

SfmlKeyboardBridge::SfmlKeyboardBridge()
        : m_nextSequence( 1 )
        , m_capsLockActive( FALSE )
        , m_capsLockKeyDown( FALSE )
{
        g_activeKeyboardBridge = this;
}

SfmlKeyboardBridge::~SfmlKeyboardBridge()
{
        if( g_activeKeyboardBridge == this )
        {
                g_activeKeyboardBridge = NULL;
        }
}

Bool SfmlKeyboardBridge::getCapsState( void )
{
        return m_capsLockActive;
}

void SfmlKeyboardBridge::handleEvent( const sf::Event &event )
{
        if( event.type != sf::Event::KeyPressed && event.type != sf::Event::KeyReleased )
        {
                return;
        }

        UnsignedShort state = (event.type == sf::Event::KeyPressed) ? KEY_STATE_DOWN : KEY_STATE_UP;
        UnsignedByte key = translateKeyCode( event.key.code );

        if( key == KEY_NONE )
        {
                return;
        }

        if( key == KEY_CAPS )
        {
                if( state == KEY_STATE_DOWN )
                {
                        if( !m_capsLockKeyDown )
                        {
                                m_capsLockActive = !m_capsLockActive;
                                m_capsLockKeyDown = TRUE;
                        }
                }
                else
                {
                        m_capsLockKeyDown = FALSE;
                }
        }

        enqueueKey( key, state );
}

void SfmlKeyboardBridge::getKey( KeyboardIO *key )
{
        if( key == NULL )
        {
                return;
        }

        if( m_pendingEvents.empty() )
        {
                key->key = KEY_NONE;
                key->state = KEY_STATE_NONE;
                key->status = KeyboardIO::STATUS_UNUSED;
                key->sequence = 0;
                return;
        }

        *key = m_pendingEvents.front();
        m_pendingEvents.pop_front();
}

void SfmlKeyboardBridge::enqueueKey( UnsignedByte key, UnsignedShort state )
{
        KeyboardIO entry;
        entry.key = key;
        entry.status = KeyboardIO::STATUS_UNUSED;
        entry.state = state;
        entry.sequence = m_nextSequence++;

        m_pendingEvents.push_back( entry );
}

SfmlKeyboardBridge *GetActiveKeyboardBridge()
{
        return g_activeKeyboardBridge;
}

Keyboard *CreateSfmlKeyboard()
{
        return new SfmlKeyboardBridge;
}

} // namespace sfml_platform

