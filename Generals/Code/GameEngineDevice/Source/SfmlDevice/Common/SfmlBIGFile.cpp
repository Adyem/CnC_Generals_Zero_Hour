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

// FILE: SfmlBIGFile.cpp ///////////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Implementation of BIG archive file access helpers for the SFML device layer.
////////////////////////////////////////////////////////////////////////////////

#include "SfmlDevice/Common/SfmlBIGFile.h"

#include "Common/ArchiveFile.h"
#include "Common/File.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"

SfmlBIGFile::SfmlBIGFile() = default;
SfmlBIGFile::~SfmlBIGFile() = default;

void SfmlBIGFile::initializeMetadata(const AsciiString& name, const AsciiString& path)
{
        m_name = name;
        m_path = path;
}

File* SfmlBIGFile::openFile(const Char* filename, Int access)
{
        const ArchivedFileInfo* fileInfo = getArchivedFileInfo(AsciiString(filename));
        if (fileInfo == NULL)
        {
                return NULL;
        }

        RAMFile* ramFile = NULL;
        if (BitTest(access, File::STREAMING))
        {
                ramFile = newInstance(StreamingArchiveFile);
        }
        else
        {
                ramFile = newInstance(RAMFile);
        }

        if (ramFile == NULL)
        {
                return NULL;
        }

        ramFile->deleteOnClose();
        if (ramFile->openFromArchive(m_file, fileInfo->m_filename, fileInfo->m_offset, fileInfo->m_size) == FALSE)
        {
                ramFile->close();
                ramFile = NULL;
                return NULL;
        }

        if ((access & File::WRITE) == 0)
        {
                return ramFile;
        }

        File* localFile = TheLocalFileSystem->openFile(filename, access);
        if (localFile != NULL)
        {
                ramFile->copyDataToFile(localFile);
        }

        ramFile->close();
        ramFile = NULL;

        return localFile;
}

void SfmlBIGFile::closeAllFiles(void)
{
}

AsciiString SfmlBIGFile::getName(void)
{
        return m_name;
}

AsciiString SfmlBIGFile::getPath(void)
{
        return m_path;
}

void SfmlBIGFile::setSearchPriority(Int new_priority)
{
        (void)new_priority;
}

void SfmlBIGFile::close(void)
{
}

Bool SfmlBIGFile::getFileInfo(const AsciiString& filename, FileInfo* fileInfo) const
{
        const ArchivedFileInfo* tempFileInfo = getArchivedFileInfo(filename);
        if (tempFileInfo == NULL)
        {
                return FALSE;
        }

        TheLocalFileSystem->getFileInfo(AsciiString(m_file->getName()), fileInfo);
        fileInfo->sizeHigh = 0;
        fileInfo->sizeLow = tempFileInfo->m_size;

        return TRUE;
}
