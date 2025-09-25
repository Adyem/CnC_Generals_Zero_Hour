/*
**     Command & Conquer Generals(tm)
**     Copyright 2025 Electronic Arts Inc.
**
**     This program is free software: you can redistribute it and/or modify
**     it under the terms of the GNU General Public License as published by
**     the Free Software Foundation, either version 3 of the License, or
**     (at your option) any later version.
**
**     This program is distributed in the hope that it will be useful,
**     but WITHOUT ANY WARRANTY; without even the implied warranty of
**     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**     GNU General Public License for more details.
**
**     You should have received a copy of the GNU General Public License
**     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Compression.h"
#include "EAC/codex.h"
#include "EAC/refcodex.h"

#include <algorithm>
#include <cstring>

namespace
{
constexpr char kRefPackTag[4] = {'E', 'A', 'R', '\0'};

inline bool HasRefPackHeader(const void* data, Int length)
{
    if (data == nullptr || length < 4)
    {
        return false;
    }

    const auto* bytes = static_cast<const UnsignedByte*>(data);
    return bytes[0] == kRefPackTag[0] && bytes[1] == kRefPackTag[1] &&
           bytes[2] == kRefPackTag[2] && bytes[3] == kRefPackTag[3];
}

inline Int ReadStoredLength(const void* data)
{
    const auto* bytes = static_cast<const UnsignedByte*>(data);
    Int value = 0;
    std::memcpy(&value, bytes, sizeof(value));
    return value;
}

inline void WriteStoredLength(void* data, Int value)
{
    auto* bytes = static_cast<UnsignedByte*>(data);
    std::memcpy(bytes, &value, sizeof(value));
}
}

const char* CompressionManager::getCompressionNameByType(CompressionType compType)
{
    switch (compType)
    {
    case COMPRESSION_NONE:
        return "No compression";
    case COMPRESSION_REFPACK:
        return "RefPack";
    default:
        return "Unsupported";
    }
}

const char* CompressionManager::getDecompressionNameByType(CompressionType compType)
{
    switch (compType)
    {
    case COMPRESSION_NONE:
        return "d_None";
    case COMPRESSION_REFPACK:
        return "d_RefPack";
    default:
        return "d_Unsupported";
    }
}

Bool CompressionManager::isDataCompressed(const void* mem, Int len)
{
    return getCompressionType(mem, len) != COMPRESSION_NONE;
}

CompressionType CompressionManager::getPreferredCompression(void)
{
    return COMPRESSION_REFPACK;
}

CompressionType CompressionManager::getCompressionType(const void* mem, Int len)
{
    if (HasRefPackHeader(mem, len))
    {
        return COMPRESSION_REFPACK;
    }

    return COMPRESSION_NONE;
}

Int CompressionManager::getMaxCompressedSize(Int uncompressedLen, CompressionType compType)
{
    if (uncompressedLen <= 0)
    {
        return 0;
    }

    switch (compType)
    {
    case COMPRESSION_NONE:
        return uncompressedLen;
    case COMPRESSION_REFPACK:
        // Original implementation stored an 8 byte header before the payload.
        return uncompressedLen + 8;
    default:
        return 0;
    }
}

Int CompressionManager::getUncompressedSize(const void* mem, Int len)
{
    if (len <= 0)
    {
        return 0;
    }

    if (HasRefPackHeader(mem, len) && len >= 8)
    {
        return ReadStoredLength(static_cast<const UnsignedByte*>(mem) + 4);
    }

    return len;
}

Int CompressionManager::compressData(CompressionType compType,
                                     void* srcVoid,
                                     Int srcLen,
                                     void* destVoid,
                                     Int destLen)
{
    if (srcVoid == nullptr || destVoid == nullptr || srcLen <= 0 || destLen <= 0)
    {
        return 0;
    }

    auto* src = static_cast<const UnsignedByte*>(srcVoid);
    auto* dest = static_cast<UnsignedByte*>(destVoid);

    switch (compType)
    {
    case COMPRESSION_NONE:
    {
        const Int copyLen = std::min(srcLen, destLen);
        if (copyLen > 0)
        {
            std::memcpy(dest, src, static_cast<std::size_t>(copyLen));
        }
        return copyLen;
    }

    case COMPRESSION_REFPACK:
    {
        if (destLen < srcLen + 8)
        {
            return 0;
        }

        std::memcpy(dest, kRefPackTag, sizeof(kRefPackTag));
        WriteStoredLength(dest + 4, srcLen);

        int compressedSize = REF_encode(dest + 8, src, srcLen, nullptr);
        if (compressedSize <= 0)
        {
            return 0;
        }

        return compressedSize + 8;
    }

    default:
        return 0;
    }
}

Int CompressionManager::decompressData(void* srcVoid,
                                       Int srcLen,
                                       void* destVoid,
                                       Int destLen)
{
    if (srcVoid == nullptr || destVoid == nullptr || srcLen <= 0 || destLen <= 0)
    {
        return 0;
    }

    auto* src = static_cast<const UnsignedByte*>(srcVoid);
    auto* dest = static_cast<UnsignedByte*>(destVoid);

    CompressionType type = getCompressionType(src, srcLen);

    switch (type)
    {
    case COMPRESSION_NONE:
    {
        const Int copyLen = std::min(srcLen, destLen);
        if (copyLen > 0)
        {
            std::memcpy(dest, src, static_cast<std::size_t>(copyLen));
        }
        return copyLen;
    }

    case COMPRESSION_REFPACK:
    {
        if (srcLen < 8)
        {
            return 0;
        }

        int compressedSize = srcLen - 8;
        int decoded = REF_decode(dest, src + 8, &compressedSize);
        if (decoded <= 0 || decoded > destLen)
        {
            return 0;
        }

        return decoded;
    }

    default:
        return 0;
    }
}
