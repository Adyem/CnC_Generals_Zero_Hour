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
 *                     $Archive:: /Commando/Code/wwlib/registry.h                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/21/01 3:42p                                              $*
 *                                                                                             *
 *                    $Revision:: 12                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef REGISTRY_H
#define REGISTRY_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include "vector.h"
#include "wwstring.h"
#include "widestring.h"

#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

class ConfigStore {
public:
        static bool Exists( const char * sub_key );

        ConfigStore( const char * sub_key );
        ~ConfigStore();

        bool    Is_Valid( void ) const          { return IsValid; }

        int     Get_Int( const char * name, int def_value = 0 );
        void    Set_Int( const char * name, int value );

        bool    Get_Bool( const char * name, bool def_value = false );
        void    Set_Bool( const char * name, bool value );

        float   Get_Float( const char * name, float def_value = 0.0f );
        void    Set_Float( const char * name, float value );

        char *  Get_String( const char * name, char *value, int value_size,
                                        const char * default_string = NULL );
        void    Get_String( const char * name, StringClass &string, const char *default_string = NULL );
        void    Set_String( const char * name, const char *value );

        void    Get_String( const WCHAR * name, WideStringClass &string, const WCHAR *default_string = NULL );
        void    Set_String( const WCHAR * name, const WCHAR *value );

        bool    Has_Value( const char * name ) const;

        void    Get_Bin( const char * name, void *buffer, int buffer_size );
        int     Get_Bin_Size( const char * name );
        void    Set_Bin( const char * name, const void *buffer, int buffer_size );

        void    Get_Value_List( DynamicVectorClass<StringClass> &list );

        void    Delete_Value( const char * name );
        void    Deleta_All_Values( void );

        static void Set_Read_Only( bool set );

private:
        enum EntryType
        {
                ENTRY_INT,
                ENTRY_BOOL,
                ENTRY_FLOAT,
                ENTRY_STRING,
                ENTRY_WIDE_STRING,
                ENTRY_BINARY
        };

        struct Entry
        {
                EntryType      Type;
                std::string    Data;
        };

        void    Load_Unlocked();
        void    Save_Unlocked();
        void    Set_Value_Unlocked( const std::string &name, EntryType type, std::string value );

        static std::filesystem::path Resolve_Path( const char * sub_key );
        static std::filesystem::path Config_Root();
        static EntryType            Type_From_String( const std::string &text );
        static std::string          Type_To_String( EntryType type );
        static std::string          Encode_Binary( const void *buffer, int buffer_size );
        static std::vector<unsigned char> Decode_Binary( const std::string &text );
        static std::string          Narrow_From_Wide( const WCHAR *text );
        static void                 Assign_Wide_From_Narrow( const std::string &text, WideStringClass &string );

        mutable std::mutex                                  Mutex;
        std::unordered_map<std::string, Entry>              Entries;
        std::filesystem::path                               FilePath;
        bool                                                IsValid;

        static bool                                         IsLocked;
};

#endif // REGISTRY_H
