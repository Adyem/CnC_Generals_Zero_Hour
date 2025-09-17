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

// FILE: RenderBackend.h ///////////////////////////////////////////////////////
//
// Interface describing the active renderer backend used by the game.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef W3DDEVICE_GAMECLIENT_RENDERBACKEND_H
#define W3DDEVICE_GAMECLIENT_RENDERBACKEND_H

class IRenderBackend
{
public:
    virtual ~IRenderBackend() {}

    virtual void HandleFocusChange(bool isActive) = 0;
};

void SetRenderBackend(IRenderBackend* backend);
IRenderBackend& GetRenderBackend();

#endif  // W3DDEVICE_GAMECLIENT_RENDERBACKEND_H

