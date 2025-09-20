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

// Registry.cpp
// Simple interface for storing/retreiving registry values
// Author: Matthew D. Campbell, December 2001

#include <string>

#include "WWLib/registry.h"
#include "Registry.h"

namespace
{
        constexpr char kRegistryConfigPath[] = "game/registry";
        constexpr char kLegacyRegistryPath[] = "SOFTWARE\\Electronic Arts\\EA Games\\Generals";
        constexpr char kDefaultValueName[] = "__default__";

        std::string Normalize_Path( const std::string &path )
        {
                std::string normalized;
                normalized.reserve( path.size() );

                for (char ch : path) {
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

        std::string Compose_Entry_Key( const std::string &path, const std::string &key )
        {
                std::string entry = Normalize_Path( path );

                if (key.empty()) {
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
                        entry.append( key );
                }

                return entry;
        }

        bool Try_Load_String( ConfigStore &store, const std::string &entry, std::string &value )
        {
                if (!store.Is_Valid() || !store.Has_Value( entry.c_str() )) {
                        return false;
                }

                StringClass text;
                store.Get_String( entry.c_str(), text, "" );
                value.assign( text.Peek_Buffer() );
                return true;
        }

        bool Try_Load_Unsigned( ConfigStore &store, const std::string &entry, unsigned int &value )
        {
                if (!store.Is_Valid() || !store.Has_Value( entry.c_str() )) {
                        return false;
                }

                value = static_cast<unsigned int>( store.Get_Int( entry.c_str(), 0 ) );
                return true;
        }
}

bool GetStringFromRegistry(std::string path, std::string key, std::string& val)
{
        const std::string entry = Compose_Entry_Key( path, key );

        std::string result;
        ConfigStore config_store( kRegistryConfigPath );
        if (Try_Load_String( config_store, entry, result )) {
                val = result;
                return true;
        }

        ConfigStore legacy_store( kLegacyRegistryPath );
        if (Try_Load_String( legacy_store, entry, result )) {
                if (config_store.Is_Valid()) {
                        config_store.Set_String( entry.c_str(), result.c_str() );
                }
                val = result;
                return true;
        }

        return false;
}

bool GetUnsignedIntFromRegistry(std::string path, std::string key, unsigned int& val)
{
        const std::string entry = Compose_Entry_Key( path, key );

        unsigned int result = 0;
        ConfigStore config_store( kRegistryConfigPath );
        if (Try_Load_Unsigned( config_store, entry, result )) {
                val = result;
                return true;
        }

        ConfigStore legacy_store( kLegacyRegistryPath );
        if (Try_Load_Unsigned( legacy_store, entry, result )) {
                if (config_store.Is_Valid()) {
                        config_store.Set_Int( entry.c_str(), static_cast<int>( result ) );
                }
                val = result;
                return true;
        }

        return false;
}

bool SetStringInRegistry( std::string path, std::string key, std::string val)
{
        const std::string entry = Compose_Entry_Key( path, key );

        bool wrote = false;
        ConfigStore config_store( kRegistryConfigPath );
        if (config_store.Is_Valid()) {
                config_store.Set_String( entry.c_str(), val.c_str() );
                wrote = true;
        }

        ConfigStore legacy_store( kLegacyRegistryPath );
        if (legacy_store.Is_Valid()) {
                legacy_store.Set_String( entry.c_str(), val.c_str() );
                wrote = true;
        }

        return wrote;
}

bool SetUnsignedIntInRegistry( std::string path, std::string key, unsigned int val)
{
        const std::string entry = Compose_Entry_Key( path, key );

        bool wrote = false;
        ConfigStore config_store( kRegistryConfigPath );
        if (config_store.Is_Valid()) {
                config_store.Set_Int( entry.c_str(), static_cast<int>( val ) );
                wrote = true;
        }

        ConfigStore legacy_store( kLegacyRegistryPath );
        if (legacy_store.Is_Valid()) {
                legacy_store.Set_Int( entry.c_str(), static_cast<int>( val ) );
                wrote = true;
        }

        return wrote;
}
