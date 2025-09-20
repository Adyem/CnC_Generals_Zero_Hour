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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G Library                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/registry.cpp                           $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 3:46p                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "registry.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <vector>

namespace
{
        int Hex_Digit( char ch )
        {
                if (ch >= '0' && ch <= '9') {
                        return ch - '0';
                }
                if (ch >= 'a' && ch <= 'f') {
                        return 10 + (ch - 'a');
                }
                if (ch >= 'A' && ch <= 'F') {
                        return 10 + (ch - 'A');
                }
                return -1;
        }
}

bool ConfigStore::IsLocked = false;

bool ConfigStore::Exists( const char * sub_key )
{
        return std::filesystem::exists( Resolve_Path( sub_key ) );
}

ConfigStore::ConfigStore( const char * sub_key ) :
        FilePath(),
        IsValid( false )
{
        try {
                FilePath = Resolve_Path( sub_key );
                std::filesystem::create_directories( FilePath.parent_path() );
                IsValid = true;
                std::lock_guard<std::mutex> lock( Mutex );
                Load_Unlocked();
        } catch ( ... ) {
                IsValid = false;
        }
}

ConfigStore::~ConfigStore() = default;

int ConfigStore::Get_Int( const char * name, int def_value )
{
        if (!IsValid) {
                return def_value;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( name != NULL ? name : "" );
        if (it == Entries.end()) {
                return def_value;
        }

        if (it->second.Type == ENTRY_INT || it->second.Type == ENTRY_BOOL) {
                try {
                        return std::stoi( it->second.Data );
                } catch ( ... ) {
                }
        }

        return def_value;
}

void ConfigStore::Set_Int( const char * name, int value )
{
        if (!IsValid || IsLocked) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        Set_Value_Unlocked( name != NULL ? name : "", ENTRY_INT, std::to_string( value ) );
}

bool ConfigStore::Get_Bool( const char * name, bool def_value )
{
        return (Get_Int( name, def_value ? 1 : 0 ) != 0);
}

void ConfigStore::Set_Bool( const char * name, bool value )
{
        Set_Int( name, value ? 1 : 0 );
}

float ConfigStore::Get_Float( const char * name, float def_value )
{
        if (!IsValid) {
                return def_value;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( name != NULL ? name : "" );
        if (it == Entries.end()) {
                return def_value;
        }

        if (it->second.Type == ENTRY_FLOAT || it->second.Type == ENTRY_INT || it->second.Type == ENTRY_BOOL) {
                try {
                        return std::stof( it->second.Data );
                } catch ( ... ) {
                }
        }

        return def_value;
}

void ConfigStore::Set_Float( const char * name, float value )
{
        if (!IsValid || IsLocked) {
                return;
        }

        std::ostringstream stream;
        stream << std::setprecision( std::numeric_limits<float>::max_digits10 ) << value;

        std::lock_guard<std::mutex> lock( Mutex );
        Set_Value_Unlocked( name != NULL ? name : "", ENTRY_FLOAT, stream.str() );
}

int ConfigStore::Get_Bin_Size( const char * name )
{
        if (!IsValid) {
                return 0;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( name != NULL ? name : "" );
        if (it == Entries.end() || it->second.Type != ENTRY_BINARY) {
                return 0;
        }

        return static_cast<int>( it->second.Data.length() / 2 );
}

void ConfigStore::Get_Bin( const char * name, void *buffer, int buffer_size )
{
        if (buffer == NULL || buffer_size <= 0) {
                return;
        }

        std::memset( buffer, 0, buffer_size );

        if (!IsValid) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( name != NULL ? name : "" );
        if (it == Entries.end() || it->second.Type != ENTRY_BINARY) {
                return;
        }

        std::vector<unsigned char> data = Decode_Binary( it->second.Data );
        int copy_size = static_cast<int>( std::min<std::size_t>( data.size(), static_cast<std::size_t>( buffer_size ) ) );
        if (copy_size > 0) {
                std::memcpy( buffer, data.data(), copy_size );
        }
}

void ConfigStore::Set_Bin( const char * name, const void *buffer, int buffer_size )
{
        if (!IsValid || IsLocked || buffer == NULL || buffer_size <= 0) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        Set_Value_Unlocked( name != NULL ? name : "", ENTRY_BINARY, Encode_Binary( buffer, buffer_size ) );
}

void ConfigStore::Get_String( const char * name, StringClass &string, const char *default_string )
{
        const char * fallback = (default_string != NULL) ? default_string : "";
        string = fallback;

        if (!IsValid) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( name != NULL ? name : "" );
        if (it == Entries.end()) {
                return;
        }

        if (it->second.Type == ENTRY_STRING || it->second.Type == ENTRY_WIDE_STRING) {
                string = it->second.Data.c_str();
        }
}

char * ConfigStore::Get_String( const char * name, char *value, int value_size, const char * default_string )
{
        if (value == NULL || value_size <= 0) {
                return value;
        }

        std::string result = (default_string != NULL) ? default_string : "";

        if (IsValid) {
                std::lock_guard<std::mutex> lock( Mutex );
                auto it = Entries.find( name != NULL ? name : "" );
                if (it != Entries.end() && (it->second.Type == ENTRY_STRING || it->second.Type == ENTRY_WIDE_STRING)) {
                        result = it->second.Data;
                }
        }

        std::size_t copy_size = std::min<std::size_t>( result.size(), static_cast<std::size_t>( value_size - 1 ) );
        std::memcpy( value, result.data(), copy_size );
        value[copy_size] = '\0';
        return value;
}

void ConfigStore::Set_String( const char * name, const char *value )
{
        if (!IsValid || IsLocked) {
                return;
        }

        const char * text = (value != NULL) ? value : "";

        std::lock_guard<std::mutex> lock( Mutex );
        Set_Value_Unlocked( name != NULL ? name : "", ENTRY_STRING, text );
}

bool ConfigStore::Has_Value( const char * name ) const
{
        if (!IsValid) {
                return false;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        return Entries.find( name != NULL ? name : "" ) != Entries.end();
}

void ConfigStore::Get_String( const WCHAR * name, WideStringClass &string, const WCHAR *default_string )
{
        string = (default_string != NULL) ? default_string : L"";

        if (!IsValid) {
                return;
        }

        std::string key = Narrow_From_Wide( name );

        std::lock_guard<std::mutex> lock( Mutex );
        auto it = Entries.find( key );
        if (it == Entries.end()) {
                return;
        }

        if (it->second.Type == ENTRY_STRING || it->second.Type == ENTRY_WIDE_STRING) {
                Assign_Wide_From_Narrow( it->second.Data, string );
        }
}

void ConfigStore::Set_String( const WCHAR * name, const WCHAR *value )
{
        if (!IsValid || IsLocked) {
                return;
        }

        std::string key = Narrow_From_Wide( name );
        WideStringClass wide_value( value != NULL ? value : L"" );
        StringClass narrow_value;
        wide_value.Convert_To( narrow_value );

        std::lock_guard<std::mutex> lock( Mutex );
        Set_Value_Unlocked( key, ENTRY_WIDE_STRING, static_cast<const char *>( narrow_value ) );
}

void ConfigStore::Get_Value_List( DynamicVectorClass<StringClass> &list )
{
        if (!IsValid) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        for (const auto & entry : Entries) {
                list.Add( entry.first.c_str() );
        }
}

void ConfigStore::Delete_Value( const char * name )
{
        if (!IsValid || IsLocked) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        if (Entries.erase( name != NULL ? name : "" ) > 0) {
                Save_Unlocked();
        }
}

void ConfigStore::Deleta_All_Values( void )
{
        if (!IsValid || IsLocked) {
                return;
        }

        std::lock_guard<std::mutex> lock( Mutex );
        if (!Entries.empty()) {
                Entries.clear();
                Save_Unlocked();
        }
}

void ConfigStore::Set_Read_Only( bool set )
{
        IsLocked = set;
}

void ConfigStore::Load_Unlocked()
{
        Entries.clear();

        if (!std::filesystem::exists( FilePath )) {
                return;
        }

        std::ifstream input( FilePath, std::ios::binary );
        if (!input) {
                return;
        }

        std::string line;
        while (std::getline( input, line )) {
                if (line.empty()) {
                        continue;
                }

                std::istringstream stream( line );
                std::string name;
                std::string type;
                std::string data;
                if (!(stream >> std::quoted( name ) >> type >> std::quoted( data ))) {
                        continue;
                }

                Entry entry;
                entry.Type = Type_From_String( type );
                entry.Data = data;
                Entries[name] = entry;
        }
}

void ConfigStore::Save_Unlocked()
{
        try {
                std::filesystem::create_directories( FilePath.parent_path() );
                std::ofstream output( FilePath, std::ios::binary | std::ios::trunc );
                if (!output) {
                        return;
                }

                std::vector<std::string> keys;
                keys.reserve( Entries.size() );
                for (const auto & entry : Entries) {
                        keys.push_back( entry.first );
                }
                std::sort( keys.begin(), keys.end() );

                for (const std::string & key : keys) {
                        const Entry &entry = Entries[key];
                        output << std::quoted( key ) << ' ' << Type_To_String( entry.Type ) << ' ' << std::quoted( entry.Data ) << '\n';
                }
        } catch ( ... ) {
        }
}

void ConfigStore::Set_Value_Unlocked( const std::string &name, EntryType type, std::string value )
{
        if (IsLocked) {
                return;
        }

        if (!IsValid) {
                return;
        }

        Entries[name] = Entry{ type, std::move( value ) };
        Save_Unlocked();
}

std::filesystem::path ConfigStore::Config_Root()
{
        if (const char * environment = std::getenv( "WW_CONFIG_DIR" )) {
                if (*environment != '\0') {
                        return std::filesystem::path( environment );
                }
        }

        try {
                return std::filesystem::current_path() / "config";
        } catch ( ... ) {
                return std::filesystem::path( "config" );
        }
}

std::filesystem::path ConfigStore::Resolve_Path( const char * sub_key )
{
        std::filesystem::path path = Config_Root();
        std::string key = (sub_key != NULL) ? sub_key : "";

        std::vector<std::string> components;
        std::string component;
        for (char ch : key) {
                if (ch == '\\' || ch == '/') {
                        if (!component.empty()) {
                                components.push_back( component );
                                component.clear();
                        }
                } else if (ch != '\0') {
                        component.push_back( ch );
                }
        }

        if (!component.empty()) {
                components.push_back( component );
        }

        if (components.empty()) {
                path /= "default.cfg";
        } else {
                for (std::size_t index = 0; index + 1 < components.size(); ++index) {
                        path /= components[index];
                }
                path /= components.back();
                path += ".cfg";
        }

        return path;
}

ConfigStore::EntryType ConfigStore::Type_From_String( const std::string &text )
{
        if (text == "int") {
                return ENTRY_INT;
        }
        if (text == "bool") {
                return ENTRY_BOOL;
        }
        if (text == "float") {
                return ENTRY_FLOAT;
        }
        if (text == "wstring") {
                return ENTRY_WIDE_STRING;
        }
        if (text == "binary") {
                return ENTRY_BINARY;
        }
        return ENTRY_STRING;
}

std::string ConfigStore::Type_To_String( EntryType type )
{
        switch (type) {
                case ENTRY_INT:           return "int";
                case ENTRY_BOOL:          return "bool";
                case ENTRY_FLOAT:         return "float";
                case ENTRY_STRING:        return "string";
                case ENTRY_WIDE_STRING:   return "wstring";
                case ENTRY_BINARY:        return "binary";
        }
        return "string";
}

std::string ConfigStore::Encode_Binary( const void *buffer, int buffer_size )
{
        const unsigned char * bytes = static_cast<const unsigned char *>( buffer );
        std::ostringstream stream;
        stream << std::hex << std::setfill( '0' );
        for (int index = 0; index < buffer_size; ++index) {
                stream << std::setw( 2 ) << static_cast<unsigned int>( bytes[index] );
        }
        return stream.str();
}

std::vector<unsigned char> ConfigStore::Decode_Binary( const std::string &text )
{
        std::vector<unsigned char> data;
        if ((text.length() % 2) != 0) {
                return data;
        }

        data.reserve( text.length() / 2 );
        for (std::size_t index = 0; index < text.length(); index += 2) {
                int high = Hex_Digit( text[index] );
                int low  = Hex_Digit( text[index + 1] );
                if (high < 0 || low < 0) {
                        data.clear();
                        return data;
                }
                data.push_back( static_cast<unsigned char>( (high << 4) | low ) );
        }

        return data;
}

std::string ConfigStore::Narrow_From_Wide( const WCHAR *text )
{
        if (text == NULL) {
                return std::string();
        }

        WideStringClass wide( text );
        StringClass narrow;
        wide.Convert_To( narrow );
        const char * buffer = narrow;
        return (buffer != NULL) ? buffer : std::string();
}

void ConfigStore::Assign_Wide_From_Narrow( const std::string &text, WideStringClass &string )
{
        string.Convert_From( text.c_str() );
}
