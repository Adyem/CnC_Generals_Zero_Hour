/*
**      Command & Conquer Generals Zero Hour(tm)
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
//
//  (c) 2001-2003 Electronic Arts Inc.                                                                                         /
//
//                                                                                                                             /
//
////////////////////////////////////////////////////////////////////////////////

// Registry.cpp
// Simple interface for storing/retreiving registry values
// Author: Matthew D. Campbell, December 2001

#include "PreRTS.h"     // This must go first in EVERY cpp file int the GameEngine

#include "Common/Registry.h"
#include "WWLib/registry.h"

#include <string>

namespace
{
        constexpr char kZeroHourRegistryConfigPath[] = "game_zh/registry";
        constexpr char kZeroHourLegacyRegistryPath[] = "SOFTWARE\\Electronic Arts\\EA Games\\Command and Conquer Generals Zero Hour";
        constexpr char kGeneralsRegistryConfigPath[] = "game/registry";
        constexpr char kGeneralsLegacyRegistryPath[] = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
        constexpr char kDefaultValueName[] = "__default__";

        std::string Normalize_Path( const AsciiString &path )
        {
                std::string normalized;
                normalized.reserve( path.getLength() );

                const int length = path.getLength();
                for (int index = 0; index < length; ++index) {
                        char ch = path.getCharAt( index );
                        if (ch == '\\' || ch == '/') {
                                if (!normalized.empty() && normalized.back() != '/') {
                                        normalized.push_back( '/' );
                                }
                        } else {
                                normalized.push_back( ch );
                        }
                }

                if (!normalized.empty() && normalized.back() == '/') {
                        normalized.pop_back();
                }

                return normalized;
        }

        std::string Compose_Entry_Key( const AsciiString &path, const AsciiString &key )
        {
                std::string entry = Normalize_Path( path );

                if (key.getLength() == 0) {
                        if (entry.empty()) {
                                entry = kDefaultValueName;
                        } else {
                                entry.append( "/" );
                                entry.append( kDefaultValueName );
                        }
                } else {
                        if (!entry.empty()) {
                                entry.push_back( '/' );
                        }
                        entry.append( key.str() );
                }

                return entry;
        }

        bool Try_Load_String( ConfigStore &store, const std::string &entry, AsciiString &value )
        {
                if (!store.Is_Valid() || !store.Has_Value( entry.c_str() )) {
                        return false;
                }

                StringClass text;
                store.Get_String( entry.c_str(), text, "" );
                value.set( text.Peek_Buffer() );
                return true;
        }

        bool Try_Load_Unsigned( ConfigStore &store, const std::string &entry, UnsignedInt &value )
        {
                if (!store.Is_Valid() || !store.Has_Value( entry.c_str() )) {
                        return false;
                }

                value = static_cast<UnsignedInt>( store.Get_Int( entry.c_str(), 0 ) );
                return true;
        }

        Bool Load_String_With_Fallback( const char *primary_path, const char *legacy_path, const std::string &entry, AsciiString &value )
        {
                ConfigStore primary_store( primary_path );
                if (Try_Load_String( primary_store, entry, value )) {
                        return TRUE;
                }

                ConfigStore legacy_store( legacy_path );
                if (Try_Load_String( legacy_store, entry, value )) {
                        if (primary_store.Is_Valid()) {
                                primary_store.Set_String( entry.c_str(), value.str() );
                        }
                        return TRUE;
                }

                return FALSE;
        }

        Bool Load_Unsigned_With_Fallback( const char *primary_path, const char *legacy_path, const std::string &entry, UnsignedInt &value )
        {
                ConfigStore primary_store( primary_path );
                if (Try_Load_Unsigned( primary_store, entry, value )) {
                        return TRUE;
                }

                ConfigStore legacy_store( legacy_path );
                if (Try_Load_Unsigned( legacy_store, entry, value )) {
                        if (primary_store.Is_Valid()) {
                                primary_store.Set_Int( entry.c_str(), static_cast<int>( value ) );
                        }
                        return TRUE;
                }

                return FALSE;
        }
}

Bool GetStringFromGeneralsRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
        const std::string entry = Compose_Entry_Key( path, key );
        DEBUG_LOG(("GetStringFromGeneralsRegistry - looking in %s\n", entry.c_str()));

        AsciiString result;
        if (Load_String_With_Fallback( kGeneralsRegistryConfigPath, kGeneralsLegacyRegistryPath, entry, result )) {
                val = result;
                return TRUE;
        }

        return FALSE;
}

Bool GetStringFromRegistry(AsciiString path, AsciiString key, AsciiString& val)
{
        const std::string entry = Compose_Entry_Key( path, key );
        DEBUG_LOG(("GetStringFromRegistry - looking in %s\n", entry.c_str()));

        AsciiString result;
        if (Load_String_With_Fallback( kZeroHourRegistryConfigPath, kZeroHourLegacyRegistryPath, entry, result )) {
                val = result;
                return TRUE;
        }

        return FALSE;
}

Bool GetUnsignedIntFromRegistry(AsciiString path, AsciiString key, UnsignedInt& val)
{
        const std::string entry = Compose_Entry_Key( path, key );
        DEBUG_LOG(("GetUnsignedIntFromRegistry - looking in %s\n", entry.c_str()));

        UnsignedInt result = 0;
        if (Load_Unsigned_With_Fallback( kZeroHourRegistryConfigPath, kZeroHourLegacyRegistryPath, entry, result )) {
                val = result;
                return TRUE;
        }

        return FALSE;
}

AsciiString GetRegistryLanguage(void)
{
        static Bool cached = FALSE;
        static AsciiString val = "english";
        if (cached) {
                return val;
        } else {
                cached = TRUE;
        }

        GetStringFromRegistry("", "Language", val);
        return val;
}

AsciiString GetRegistryGameName(void)
{
        AsciiString val = "GeneralsMPTest";
        GetStringFromRegistry("", "SKU", val);
        return val;
}

UnsignedInt GetRegistryVersion(void)
{
        UnsignedInt val = 65536;
        GetUnsignedIntFromRegistry("", "Version", val);
        return val;
}

UnsignedInt GetRegistryMapPackVersion(void)
{
        UnsignedInt val = 65536;
        GetUnsignedIntFromRegistry("", "MapPackVersion", val);
        return val;
}
