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

// FILE: SfmlOSDisplay.cpp /////////////////////////////////////////////////////
//----------------------------------------------------------------------------- 
//
//  Cross-platform implementation of OSDisplayWarningBox built on top of the
//  SFML bootstrap.  The legacy Win32 path surfaced modal MessageBox dialogs;
//  this version falls back to terminal prompts while continuing to expose the
//  same interface to the rest of the engine.
//
//----------------------------------------------------------------------------- 
///////////////////////////////////////////////////////////////////////////////

#include "Common/OSDisplay.h"

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include "GameClient/GameText.h"

#include <cctype>
#include <cstdio>
#include <iostream>
#include <string>

#if defined(_WIN32)
#include <io.h>
#define SFML_OSDISPLAY_ISATTY _isatty
#define SFML_OSDISPLAY_FILENO _fileno
#else
#include <unistd.h>
#define SFML_OSDISPLAY_ISATTY isatty
#define SFML_OSDISPLAY_FILENO fileno
#endif

namespace
{

Bool IsInteractiveTerminal()
{
        FILE *stream = stdin;
        if (stream == NULL)
        {
                return FALSE;
        }

        return SFML_OSDISPLAY_ISATTY(SFML_OSDISPLAY_FILENO(stream)) ? TRUE : FALSE;
}

} // anonymous namespace

OSDisplayButtonType OSDisplayWarningBox(AsciiString p, AsciiString m, UnsignedInt buttonFlags, UnsignedInt /*otherFlags*/)
{
        if (TheGameText == NULL)
        {
                return OSDBT_ERROR;
        }

        UnicodeString promptStr = TheGameText->fetch(p);
        UnicodeString messageStr = TheGameText->fetch(m);

        AsciiString promptAscii;
        promptAscii.translate(promptStr);
        AsciiString messageAscii;
        messageAscii.translate(messageStr);

        std::cerr << "[OSDisplay] " << promptAscii.str() << ": " << messageAscii.str() << std::endl;

        const Bool allowCancel = (buttonFlags & OSDBT_CANCEL) != 0;
        if (!allowCancel)
        {
                return OSDBT_OK;
        }

        if (!IsInteractiveTerminal())
        {
                std::cerr << "[OSDisplay] Non-interactive terminal detected; defaulting to OK" << std::endl;
                return OSDBT_OK;
        }

        while (true)
        {
                std::cerr << "Confirm action? [y/N]: " << std::flush;
                std::string response;
                if (!std::getline(std::cin, response))
                {
                        return OSDBT_CANCEL;
                }

                if (response.empty())
                {
                        return OSDBT_OK;
                }

                const char ch = static_cast<char>(std::tolower(static_cast<unsigned char>(response[0])));
                if (ch == 'y')
                {
                        return OSDBT_OK;
                }
                if (ch == 'n')
                {
                        return OSDBT_CANCEL;
                }
        }
}

