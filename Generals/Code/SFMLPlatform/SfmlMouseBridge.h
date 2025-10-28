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

// FILE: SfmlMouseBridge.h /////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//  SFML mouse bridge that adapts sf::Event mouse input to the legacy
//  GameClient::Mouse API.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////


#ifndef SFML_MOUSE_BRIDGE_H
#define SFML_MOUSE_BRIDGE_H

#include <deque>

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Cursor.hpp>

#include "W3DDevice/GameClient/W3DMouse.h"

namespace sf
{
        class Event;
}

namespace sfml_platform
{

class SfmlMouseBridge : public W3DMouse
{
public:
        SfmlMouseBridge();
        virtual ~SfmlMouseBridge();

        void handleEvent( const sf::Event &event );
        void refreshCursor();

protected:
        virtual UnsignedByte getMouseEvent( MouseIO *result, Bool flush );
        virtual void platformSetCursor(MouseCursor cursor) override;
        virtual void platformClearCursor() override;
        virtual void platformRestoreDefaultCursor() override;

private:
        enum ButtonIndex
        {
                BUTTON_LEFT = 0,
                BUTTON_RIGHT,
                BUTTON_MIDDLE,
                BUTTON_COUNT
        };

        struct ClickState
        {
                UnsignedInt lastClickTime;
                ICoord2D lastClickPos;
        };

        void enqueueMouseEvent( const MouseIO &event );
        void handleButtonEvent( sf::Mouse::Button button, Bool pressed, int x, int y, UnsignedInt timestamp, UnsignedInt frame );
        void handleMoveEvent( int x, int y, UnsignedInt timestamp, UnsignedInt frame );
        void handleWheelEvent( float delta, int x, int y, UnsignedInt timestamp, UnsignedInt frame );
        Bool isDoubleClick( ButtonIndex index, const ICoord2D &pos, UnsignedInt timestamp );

        UnsignedInt currentFrame() const;
        UnsignedInt currentTimestamp() const;
        static ButtonIndex toButtonIndex( sf::Mouse::Button button );

        std::deque< MouseIO > m_pendingEvents;
        ClickState m_clickState[ BUTTON_COUNT ];
        UnsignedInt m_doubleClickTime;
        sf::Cursor m_arrowCursor;
        bool m_arrowCursorValid;
};

SfmlMouseBridge *GetActiveMouseBridge();
Mouse *CreateSfmlMouse();

} // namespace sfml_platform

#endif // SFML_MOUSE_BRIDGE_H
