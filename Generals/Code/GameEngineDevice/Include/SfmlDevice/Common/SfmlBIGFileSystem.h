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

// FILE: SfmlBIGFileSystem.h ///////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Platform-neutral BIG archive file system implementation used by the SFML
//   device layer.
////////////////////////////////////////////////////////////////////////////////


#ifndef __SFMLBIGFILESYSTEM_H
#define __SFMLBIGFILESYSTEM_H

#include "Common/ArchiveFileSystem.h"

class SfmlBIGFileSystem : public ArchiveFileSystem
{
public:
        SfmlBIGFileSystem();
        virtual ~SfmlBIGFileSystem();

        virtual void init(void);
        virtual void update(void);
        virtual void reset(void);
        virtual void postProcessLoad(void);

        virtual ArchiveFile* openArchiveFile(const Char* filename);
        virtual void closeArchiveFile(const Char* filename);
        virtual void closeAllArchiveFiles(void);
        virtual void closeAllFiles(void);

        virtual Bool loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite = FALSE);
};

#endif // __SFMLBIGFILESYSTEM_H
