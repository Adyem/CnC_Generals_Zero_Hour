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
//                                                                            //
//  (c) 2001-2003 Electronic Arts Inc.                                        //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// FILE: SfmlLocalFileSystem.h /////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Platform-neutral local file system implementation used by the SFML
//   bootstrap.
////////////////////////////////////////////////////////////////////////////////


#ifndef __SFMLLOCALFILESYSTEM_H
#define __SFMLLOCALFILESYSTEM_H

#include "Common/LocalFileSystem.h"

class SfmlLocalFileSystem : public LocalFileSystem
{
public:
        SfmlLocalFileSystem();
        virtual ~SfmlLocalFileSystem();

        virtual void init();
        virtual void reset();
        virtual void update();

        virtual File* openFile(const Char* filename, Int access = 0);
        virtual Bool doesFileExist(const Char* filename) const;
        virtual void getFileListInDirectory(const AsciiString& currentDirectory,
                                            const AsciiString& originalDirectory,
                                            const AsciiString& searchName,
                                            FilenameList& filenameList,
                                            Bool searchSubdirectories) const;
        virtual Bool getFileInfo(const AsciiString& filename, FileInfo* fileInfo) const;
        virtual Bool createDirectory(AsciiString directory);

private:
        void enumerateDirectory(const AsciiString& currentDirectory,
                                const AsciiString& originalDirectory,
                                const AsciiString& searchName,
                                FilenameList& filenameList,
                                Bool searchSubdirectories) const;
};

#endif // __SFMLLOCALFILESYSTEM_H
