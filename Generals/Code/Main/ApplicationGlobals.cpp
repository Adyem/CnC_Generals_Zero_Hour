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

#include "WinMain.h"
#include "Lib/BaseType.h"

HINSTANCE ApplicationHInstance = NULL;
HWND ApplicationHWnd = NULL;
HDC ApplicationHDC = NULL;
HGLRC ApplicationHGLRC = NULL;
Bool ApplicationIsWindowed = false;
GraphicsBackend ApplicationGraphicsBackend = GRAPHICS_BACKEND_DIRECT3D8;
Win32Mouse* TheWin32Mouse = NULL;
DWORD TheMessageTime = 0;

const Char* g_strFile = "data\\Generals.str";
const Char* g_csfFile = "data\\%s\\Generals.csf";
char* gAppPrefix = "";
