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

// FILE: SfmlMouseBridge.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//  SFML mouse bridge implementation.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#include "SfmlMouseBridge.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include <GameClient/GameClient.h>

#include <cstring>
#include <chrono>

namespace sfml_platform
{
namespace
{

SfmlMouseBridge *g_activeMouseBridge = NULL;

MouseIO createDefaultMouseEvent( UnsignedInt timestamp )
{
        MouseIO event;
        std::memset( &event, 0, sizeof( event ) );
        event.leftState = MBS_Up;
        event.rightState = MBS_Up;
        event.middleState = MBS_Up;
        event.time = timestamp;
        return event;
}

UnsignedInt getPlatformDoubleClickTime()
{
#if defined(_WIN32)
        return static_cast< UnsignedInt >( ::GetDoubleClickTime() );
#else
        return 500;
#endif
}

} // unnamed namespace

SfmlMouseBridge::SfmlMouseBridge()
{
        for( Int i = 0; i < BUTTON_COUNT; ++i )
        {
                m_clickState[ i ].lastClickTime = 0;
                m_clickState[ i ].lastClickPos.x = 0;
                m_clickState[ i ].lastClickPos.y = 0;
        }

        m_doubleClickTime = getPlatformDoubleClickTime();
        g_activeMouseBridge = this;
}

SfmlMouseBridge::~SfmlMouseBridge()
{
        if( g_activeMouseBridge == this )
        {
                g_activeMouseBridge = NULL;
        }
}

void SfmlMouseBridge::handleEvent( const sf::Event &event )
{
        UnsignedInt timestamp = currentTimestamp();
        UnsignedInt frame = currentFrame();

        switch( event.type )
        {
                case sf::Event::MouseMoved:
                {
                        handleMoveEvent( event.mouseMove.x, event.mouseMove.y, timestamp, frame );
                        break;
                }

                case sf::Event::MouseButtonPressed:
                {
                        handleButtonEvent( event.mouseButton.button, TRUE, event.mouseButton.x, event.mouseButton.y, timestamp, frame );
                        break;
                }

                case sf::Event::MouseButtonReleased:
                {
                        handleButtonEvent( event.mouseButton.button, FALSE, event.mouseButton.x, event.mouseButton.y, timestamp, frame );
                        break;
                }

                case sf::Event::MouseWheelScrolled:
                {
                        handleWheelEvent( event.mouseWheelScroll.delta, event.mouseWheelScroll.x, event.mouseWheelScroll.y, timestamp, frame );
                        break;
                }

                default:
                {
                        break;
                }
        }
}

void SfmlMouseBridge::refreshCursor()
{
        setCursor( getMouseCursor() );
}

UnsignedByte SfmlMouseBridge::getMouseEvent( MouseIO *result, Bool flush )
{
        if( result == NULL )
        {
                return MOUSE_NONE;
        }

        if( m_pendingEvents.empty() )
        {
                return MOUSE_NONE;
        }

        *result = m_pendingEvents.front();
        if( flush )
        {
                m_pendingEvents.pop_front();
        }

        return MOUSE_OK;
}

void SfmlMouseBridge::enqueueMouseEvent( const MouseIO &event )
{
        if( m_pendingEvents.size() >= Mouse::NUM_MOUSE_EVENTS )
        {
                return;
        }

        m_pendingEvents.push_back( event );
}

void SfmlMouseBridge::handleButtonEvent( sf::Mouse::Button button, Bool pressed, int x, int y, UnsignedInt timestamp, UnsignedInt frame )
{
        ButtonIndex index = toButtonIndex( button );
        if( index == BUTTON_COUNT )
        {
                return;
        }

        MouseIO event = createDefaultMouseEvent( timestamp );
        event.pos.x = x;
        event.pos.y = y;

        Bool doubleClick = FALSE;
        if( pressed )
        {
                doubleClick = isDoubleClick( index, event.pos, timestamp );
        }

        switch( index )
        {
                case BUTTON_LEFT:
                {
                        event.leftState = pressed ? MBS_Down : MBS_Up;
                        event.leftFrame = frame;
                        break;
                }

                case BUTTON_RIGHT:
                {
                        event.rightState = pressed ? MBS_Down : MBS_Up;
                        event.rightFrame = frame;
                        break;
                }

                case BUTTON_MIDDLE:
                {
                        event.middleState = pressed ? MBS_Down : MBS_Up;
                        event.middleFrame = frame;
                        break;
                }

                default:
                {
                        break;
                }
        }

        enqueueMouseEvent( event );

        if( pressed && doubleClick )
        {
                MouseIO doubleClickEvent = event;

                switch( index )
                {
                        case BUTTON_LEFT:
                        {
                                doubleClickEvent.leftState = MBS_DoubleClick;
                                break;
                        }

                        case BUTTON_RIGHT:
                        {
                                doubleClickEvent.rightState = MBS_DoubleClick;
                                break;
                        }

                        case BUTTON_MIDDLE:
                        {
                                doubleClickEvent.middleState = MBS_DoubleClick;
                                break;
                        }

                        default:
                        {
                                break;
                        }
                }

                enqueueMouseEvent( doubleClickEvent );
        }
}

void SfmlMouseBridge::handleMoveEvent( int x, int y, UnsignedInt timestamp, UnsignedInt frame )
{
        (void)frame;

        MouseIO event = createDefaultMouseEvent( timestamp );
        event.pos.x = x;
        event.pos.y = y;

        enqueueMouseEvent( event );
}

void SfmlMouseBridge::handleWheelEvent( float delta, int x, int y, UnsignedInt timestamp, UnsignedInt frame )
{
        (void)frame;

        MouseIO event = createDefaultMouseEvent( timestamp );
        event.pos.x = x;
        event.pos.y = y;
        event.wheelPos = static_cast< Int >( delta * MOUSE_WHEEL_DELTA );

        enqueueMouseEvent( event );
}

Bool SfmlMouseBridge::isDoubleClick( ButtonIndex index, const ICoord2D &pos, UnsignedInt timestamp )
{
        ClickState &state = m_clickState[ index ];

        Bool withinTime = (state.lastClickTime != 0) && ((timestamp - state.lastClickTime) <= m_doubleClickTime);
        Int dx = pos.x - state.lastClickPos.x;
        Int dy = pos.y - state.lastClickPos.y;
        Bool withinDistance = (dx * dx + dy * dy) <= CLICK_DISTANCE_DELTA_SQUARED;

        state.lastClickTime = timestamp;
        state.lastClickPos = pos;

        return withinTime && withinDistance;
}

UnsignedInt SfmlMouseBridge::currentFrame() const
{
        if( TheGameClient )
        {
                return TheGameClient->getFrame();
        }

        return 1;
}

UnsignedInt SfmlMouseBridge::currentTimestamp() const
{
#if defined(_WIN32)
        return static_cast< UnsignedInt >( ::GetTickCount() );
#else
        using namespace std::chrono;
        return static_cast< UnsignedInt >( duration_cast< milliseconds >( steady_clock::now().time_since_epoch() ).count() );
#endif
}

SfmlMouseBridge::ButtonIndex SfmlMouseBridge::toButtonIndex( sf::Mouse::Button button )
{
        switch( button )
        {
                case sf::Mouse::Left:
                        return BUTTON_LEFT;
                case sf::Mouse::Right:
                        return BUTTON_RIGHT;
                case sf::Mouse::Middle:
                        return BUTTON_MIDDLE;
                default:
                        return BUTTON_COUNT;
        }
}

SfmlMouseBridge *GetActiveMouseBridge()
{
        return g_activeMouseBridge;
}

Mouse *CreateSfmlMouse()
{
        SfmlMouseBridge *mouse = NEW SfmlMouseBridge;
        return mouse;
}

} // namespace sfml_platform
