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
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
//                                                                                                                             /
////////////////////////////////////////////////////////////////////////////////

// FILE: ThreadUtils.cpp //////////////////////////////////////////////////////
// GameSpy thread utils
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"     // This must go first in EVERY cpp file int the GameEngine

#include <codecvt>
#include <locale>

//-------------------------------------------------------------------------

std::wstring MultiByteToWideCharSingleLine(const char *orig)
{
	if (orig == nullptr) {
		return std::wstring();
	}
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring result = converter.from_bytes(orig);
	for (WideChar &ch : result) {
		if ((ch == L'\n') || (ch == L'\r')) {
			ch = L' ';
		}
	}
	return result;
}

std::string WideCharStringToMultiByte(const WideChar *orig)
{
	if (orig == nullptr) {
		return std::string();
	}
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.to_bytes(orig);
}
