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

// FILE: RenderBackend.cpp /////////////////////////////////////////////////////
//
// Storage for the active renderer backend instance.
//
////////////////////////////////////////////////////////////////////////////////

#include "W3DDevice/GameClient/RenderBackend.h"

namespace
{
class NullRenderBackend : public IRenderBackend
{
public:
    virtual void HandleFocusChange(bool) {}
    virtual void BindTexture(unsigned, TextureClass*) {}
};

NullRenderBackend g_nullRenderBackend;
IRenderBackend* g_activeRenderBackend = &g_nullRenderBackend;
}

void SetRenderBackend(IRenderBackend* backend)
{
    if (backend)
    {
        g_activeRenderBackend = backend;
    }
    else
    {
        g_activeRenderBackend = &g_nullRenderBackend;
    }
}

IRenderBackend& GetRenderBackend()
{
    return *g_activeRenderBackend;
}

