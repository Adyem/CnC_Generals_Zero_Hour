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

// FILE: SfmlLocalFileSystem.cpp ///////////////////////////////////////////////
// Author: OpenAI Assistant, 2025
// Description:
//   Implementation of a platform-neutral local file system used by the SFML
//   bootstrap.
////////////////////////////////////////////////////////////////////////////////

#include "SfmlDevice/Common/SfmlLocalFileSystem.h"

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/LocalFile.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{

bool matchPattern(std::string_view pattern, std::string_view candidate)
{
        std::string lowerPattern(pattern);
        std::string lowerCandidate(candidate);
        std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        std::transform(lowerCandidate.begin(), lowerCandidate.end(), lowerCandidate.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });

        const char* p = lowerPattern.c_str();
        const char* s = lowerCandidate.c_str();
        const char* star = nullptr;
        const char* match = nullptr;

        while (*s != '\0')
        {
                if (*p == '?' || *p == *s)
                {
                        ++p;
                        ++s;
                }
                else if (*p == '*')
                {
                        star = p++;
                        match = s;
                }
                else if (star != nullptr)
                {
                        p = star + 1;
                        s = ++match;
                }
                else
                {
                        return false;
                }
        }

        while (*p == '*')
        {
                ++p;
        }

        return *p == '\0';
}

std::filesystem::path toPath(const AsciiString& path)
{
        std::string temp = path.str();
        std::replace(temp.begin(), temp.end(), '\\', std::filesystem::path::preferred_separator);
        return std::filesystem::path(temp);
}

} // anonymous namespace

SfmlLocalFileSystem::SfmlLocalFileSystem() = default;
SfmlLocalFileSystem::~SfmlLocalFileSystem() = default;

void SfmlLocalFileSystem::init()
{
}

void SfmlLocalFileSystem::reset()
{
}

void SfmlLocalFileSystem::update()
{
}

File* SfmlLocalFileSystem::openFile(const Char* filename, Int access)
{
        LocalFile* file = newInstance(LocalFile);

        if (filename == NULL || *filename == '\0')
        {
                return NULL;
        }

        if ((access & File::WRITE) != 0)
        {
                std::filesystem::path targetPath = toPath(AsciiString(filename));
                if (targetPath.has_parent_path())
                {
                        std::error_code ec;
                        std::filesystem::create_directories(targetPath.parent_path(), ec);
                }
        }

        if (file->open(filename, access) == FALSE)
        {
                file->close();
                file->deleteInstance();
                return NULL;
        }

        file->deleteOnClose();
        return file;
}

Bool SfmlLocalFileSystem::doesFileExist(const Char* filename) const
{
        if (filename == NULL)
        {
                return FALSE;
        }

#if defined(_WIN32)
        return (_access(filename, 0) == 0) ? TRUE : FALSE;
#else
        return (access(filename, F_OK) == 0) ? TRUE : FALSE;
#endif
}

void SfmlLocalFileSystem::getFileListInDirectory(const AsciiString& currentDirectory,
                                                 const AsciiString& originalDirectory,
                                                 const AsciiString& searchName,
                                                 FilenameList& filenameList,
                                                 Bool searchSubdirectories) const
{
        enumerateDirectory(currentDirectory, originalDirectory, searchName, filenameList, searchSubdirectories);
}

void SfmlLocalFileSystem::enumerateDirectory(const AsciiString& currentDirectory,
                                             const AsciiString& originalDirectory,
                                             const AsciiString& searchName,
                                             FilenameList& filenameList,
                                             Bool searchSubdirectories) const
{
        namespace fs = std::filesystem;

        fs::path basePath = toPath(originalDirectory);
        fs::path relativePath = toPath(currentDirectory);
        fs::path searchPath = basePath / relativePath;

        std::error_code ec;
        if (!fs::exists(searchPath, ec) || !fs::is_directory(searchPath, ec))
        {
                return;
        }

        const std::string pattern = searchName.str();

        for (fs::directory_iterator iter(searchPath, ec); !ec && iter != fs::directory_iterator(); ++iter)
        {
                if (!iter->is_regular_file(ec))
                {
                        continue;
                }

                const std::string filename = iter->path().filename().string();
                if (!matchPattern(pattern, filename))
                {
                        continue;
                }

                AsciiString fullPath = originalDirectory;
                fullPath.concat(currentDirectory);
                fullPath.concat(filename.c_str());

                if (filenameList.find(fullPath) == filenameList.end())
                {
                        filenameList.insert(fullPath);
                }
        }

        if (!searchSubdirectories)
        {
                return;
        }

        ec.clear();
        for (fs::directory_iterator iter(searchPath, ec); !ec && iter != fs::directory_iterator(); ++iter)
        {
                if (!iter->is_directory(ec))
                {
                        continue;
                }

                const std::string folder = iter->path().filename().string();
                if (folder == "." || folder == "..")
                {
                        continue;
                }

                AsciiString nextDirectory = currentDirectory;
                nextDirectory.concat(folder.c_str());
                nextDirectory.concat("\\");

                enumerateDirectory(nextDirectory, originalDirectory, searchName, filenameList, searchSubdirectories);
        }
}

Bool SfmlLocalFileSystem::getFileInfo(const AsciiString& filename, FileInfo* fileInfo) const
{
        if (fileInfo == NULL)
        {
                return FALSE;
        }

        namespace fs = std::filesystem;
        fs::path path = toPath(filename);

        std::error_code ec;
        if (!fs::exists(path, ec))
        {
                return FALSE;
        }

        const uintmax_t size = fs::file_size(path, ec);
        if (ec)
        {
                return FALSE;
        }

        auto writeTime = fs::last_write_time(path, ec);
        if (ec)
        {
                return FALSE;
        }

        const auto sysTime = std::chrono::time_point_cast<std::chrono::nanoseconds>(writeTime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        const auto timestamp = static_cast<unsigned long long>(sysTime.time_since_epoch().count());

        fileInfo->sizeHigh = static_cast<Int>((static_cast<unsigned long long>(size) >> 32) & 0xFFFFFFFFULL);
        fileInfo->sizeLow = static_cast<Int>(static_cast<unsigned long long>(size) & 0xFFFFFFFFULL);
        fileInfo->timestampHigh = static_cast<Int>((timestamp >> 32) & 0xFFFFFFFFULL);
        fileInfo->timestampLow = static_cast<Int>(timestamp & 0xFFFFFFFFULL);

        return TRUE;
}

Bool SfmlLocalFileSystem::createDirectory(AsciiString directory)
{
        namespace fs = std::filesystem;
        fs::path path = toPath(directory);

        std::error_code ec;
        if (fs::exists(path, ec))
        {
                                return TRUE;
        }

        return fs::create_directories(path, ec) ? TRUE : FALSE;
}
