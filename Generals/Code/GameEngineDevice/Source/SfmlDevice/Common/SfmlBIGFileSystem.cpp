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

// FILE: SfmlBIGFileSystem.cpp /////////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Implementation of the platform-neutral BIG archive file system for the
//   SFML device layer.
////////////////////////////////////////////////////////////////////////////////

#include "SfmlDevice/Common/SfmlBIGFileSystem.h"

#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/AudioAffect.h"
#include "Common/Debug.h"
#include "Common/File.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"
#include "SfmlDevice/Common/SfmlBIGFile.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <string>

namespace
{

UnsignedInt readBigEndianUInt32(File* file)
{
        std::array<unsigned char, 4> buffer{};
        if (file->read(buffer.data(), static_cast<Int>(buffer.size())) != static_cast<Int>(buffer.size()))
        {
                return 0;
        }

        return (static_cast<UnsignedInt>(buffer[0]) << 24) |
               (static_cast<UnsignedInt>(buffer[1]) << 16) |
               (static_cast<UnsignedInt>(buffer[2]) << 8) |
               static_cast<UnsignedInt>(buffer[3]);
}

} // anonymous namespace

SfmlBIGFileSystem::SfmlBIGFileSystem() = default;
SfmlBIGFileSystem::~SfmlBIGFileSystem() = default;

void SfmlBIGFileSystem::init(void)
{
        DEBUG_ASSERTCRASH(TheLocalFileSystem != NULL, ("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
        if (TheLocalFileSystem == NULL)
        {
                return;
        }

        loadBigFilesFromDirectory("", "*.big");
}

void SfmlBIGFileSystem::reset(void)
{
}

void SfmlBIGFileSystem::update(void)
{
}

void SfmlBIGFileSystem::postProcessLoad(void)
{
}

ArchiveFile* SfmlBIGFileSystem::openArchiveFile(const Char* filename)
{
        File* fp = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
        if (fp == NULL)
        {
                DEBUG_CRASH(("Could not open archive file %s for parsing", filename));
                return NULL;
        }

        AsciiString archiveFileName(filename);
        archiveFileName.toLower();

        const char bigIdentifier[] = "BIGF";
        std::array<char, 1024> buffer{};
        fp->read(buffer.data(), 4);
        buffer[4] = 0;

        if (std::strncmp(buffer.data(), bigIdentifier, 4) != 0)
        {
                DEBUG_CRASH(("Error reading BIG file identifier in file %s", filename));
                fp->close();
                return NULL;
        }

        const Int archiveFileSize = static_cast<Int>(readBigEndianUInt32(fp));
        (void)archiveFileSize;

        const Int numLittleFiles = static_cast<Int>(readBigEndianUInt32(fp));

        fp->seek(0x10, File::START);

        ArchivedFileInfo* fileInfo = NEW ArchivedFileInfo;
        SfmlBIGFile* sfmlBigFile = NEW SfmlBIGFile;
        ArchiveFile* archiveFile = sfmlBigFile;

        std::string fullPathString(filename);
        const std::string::size_type lastSeparator = fullPathString.find_last_of("\\/");
        AsciiString archiveName;
        AsciiString archivePath;
        if (lastSeparator == std::string::npos)
        {
                archiveName = fullPathString.c_str();
                archivePath = fullPathString.c_str();
        }
        else
        {
                archiveName = fullPathString.substr(lastSeparator + 1).c_str();
                archivePath = fullPathString.substr(0, lastSeparator).c_str();
        }

        sfmlBigFile->initializeMetadata(archiveName, archivePath);

        for (Int i = 0; i < numLittleFiles; ++i)
        {
                const Int fileOffset = static_cast<Int>(readBigEndianUInt32(fp));
                const Int fileSize = static_cast<Int>(readBigEndianUInt32(fp));

                fileInfo->m_archiveFilename = archiveFileName;
                fileInfo->m_offset = fileOffset;
                fileInfo->m_size = fileSize;

                Int pathIndex = -1;
                do
                {
                        ++pathIndex;
                        fp->read(buffer.data() + pathIndex, 1);
                } while (buffer[pathIndex] != 0 && pathIndex < static_cast<Int>(buffer.size()) - 1);

                Int filenameIndex = pathIndex;
                while (filenameIndex >= 0 && buffer[filenameIndex] != '\\' && buffer[filenameIndex] != '/')
                {
                        --filenameIndex;
                }

                fileInfo->m_filename = (char*)(buffer.data() + filenameIndex + 1);
                fileInfo->m_filename.toLower();
                buffer[filenameIndex + 1] = 0;

                AsciiString path(buffer.data());
                archiveFile->addFile(path, fileInfo);
        }

        archiveFile->attachFile(fp);
        delete fileInfo;

        return archiveFile;
}

void SfmlBIGFileSystem::closeArchiveFile(const Char* filename)
{
        ArchiveFileMap::iterator it = m_archiveFileMap.find(AsciiString(filename));
        if (it == m_archiveFileMap.end())
        {
                return;
        }

        if (AsciiString(filename).compareNoCase(MUSIC_BIG) == 0)
        {
                TheAudio->stopAudio(AudioAffect_Music);
        }

        delete it->second;
        m_archiveFileMap.erase(it);
}

void SfmlBIGFileSystem::closeAllArchiveFiles(void)
{
}

void SfmlBIGFileSystem::closeAllFiles(void)
{
}

Bool SfmlBIGFileSystem::loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite)
{
        FilenameList filenameList;
        TheLocalFileSystem->getFileListInDirectory(dir, AsciiString(""), fileMask, filenameList, TRUE);

        Bool actuallyAdded = FALSE;
        for (FilenameListIter it = filenameList.begin(); it != filenameList.end(); ++it)
        {
                ArchiveFile* archiveFile = openArchiveFile((*it).str());
                if (archiveFile != NULL)
                {
                        loadIntoDirectoryTree(archiveFile, *it, overwrite);
                        m_archiveFileMap[*it] = archiveFile;
                        actuallyAdded = TRUE;
                }
        }

        return actuallyAdded;
}
