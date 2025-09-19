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

// FILE: SfmlBIGFile.h /////////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   ArchiveFile implementation for BIG archives used by the SFML device layer.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SFMLBIGFILE_H
#define __SFMLBIGFILE_H

#include "Common/ArchiveFile.h"

class SfmlBIGFile : public ArchiveFile
{
        MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(SfmlBIGFile, "SfmlBIGFile")
public:
        SfmlBIGFile();
        virtual ~SfmlBIGFile();

        virtual File* openFile(const Char* filename, Int access = 0);
        virtual void closeAllFiles(void);
        virtual AsciiString getName(void);
        virtual AsciiString getPath(void);
        virtual void setSearchPriority(Int new_priority);
        virtual void close(void);
        virtual Bool getFileInfo(const AsciiString& filename, FileInfo* fileInfo) const;
};

#endif // __SFMLBIGFILE_H
