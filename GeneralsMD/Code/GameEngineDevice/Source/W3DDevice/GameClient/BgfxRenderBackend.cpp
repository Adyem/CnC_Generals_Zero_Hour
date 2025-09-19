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

#include "W3DDevice/GameClient/BgfxRenderBackend.h"

#include "WW3D2/dx8wrapper.h"

void BgfxRenderBackend::HandleFocusChange(bool)
{
    // The bgfx backend currently does not require focus recovery logic.
}

void BgfxRenderBackend::BindTexture(unsigned stage, TextureClass* texture)
{
    DX8Wrapper::Set_Texture(stage, texture);
}

BgfxRenderBackend g_bgfxRenderBackend;

