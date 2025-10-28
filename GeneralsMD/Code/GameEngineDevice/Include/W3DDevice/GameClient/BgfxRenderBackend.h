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


#ifndef GENERALSMD_CODE_GAMEENGINEDEVICE_INCLUDE_W3DDEVICE_GAMECLIENT_BGFXRENDERBACKEND_H
#define GENERALSMD_CODE_GAMEENGINEDEVICE_INCLUDE_W3DDEVICE_GAMECLIENT_BGFXRENDERBACKEND_H

#include "W3DDevice/GameClient/RenderBackend.h"

class BgfxRenderBackend : public IRenderBackend
{
public:
    virtual void HandleFocusChange(bool isActive) override;
    virtual void BindTexture(unsigned stage, TextureClass* texture) override;
};

extern BgfxRenderBackend g_bgfxRenderBackend;


#endif // GENERALSMD_CODE_GAMEENGINEDEVICE_INCLUDE_W3DDEVICE_GAMECLIENT_BGFXRENDERBACKEND_H
