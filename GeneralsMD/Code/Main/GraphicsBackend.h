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

// FILE: GraphicsBackend.h //////////////////////////////////////////////////////
//
// Enumeration describing the available rendering backends.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef MAIN_GRAPHICSBACKEND_H
#define MAIN_GRAPHICSBACKEND_H

#ifndef WW3D_ENABLE_LEGACY_DX8
#define WW3D_ENABLE_LEGACY_DX8 0
#endif

enum GraphicsBackend
{
        GRAPHICS_BACKEND_DIRECT3D8 = 0,
        GRAPHICS_BACKEND_OPENGL,
        GRAPHICS_BACKEND_BGFX,
};

#endif  // MAIN_GRAPHICSBACKEND_H

