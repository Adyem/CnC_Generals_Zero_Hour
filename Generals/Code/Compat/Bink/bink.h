#pragma once

// Minimal subset of the RAD Game Tools Bink API required to compile the
// cross-platform build.  The real SDK is proprietary and only available on
// Windows.  We provide lightweight stand-ins for the handle and a few helper
// structures so that code can be compiled without the SDK.

#ifndef _WIN32

#include <cstddef>
#include <cstdint>

struct BINK
{
        std::int32_t FrameNum = 0;
        std::int32_t Frames = 0;
        std::int32_t Height = 0;
        std::int32_t Width = 0;
};

using HBINK = BINK*;

struct BINKRECT
{
        std::int32_t Left;
        std::int32_t Top;
        std::int32_t Width;
        std::int32_t Height;
};

constexpr unsigned int BINKPRELOADALL = 0;
constexpr unsigned int BINKSURFACE32 = 0;
constexpr unsigned int BINKSURFACE24 = 0;
constexpr unsigned int BINKSURFACE565 = 0;
constexpr unsigned int BINKSURFACE555 = 0;

inline void BinkSetSoundTrack(int, int) {}
inline void* BinkOpenTrack(HBINK, int) { return nullptr; }
inline void BinkCloseTrack(HBINK, void*) {}
inline unsigned int BinkGetTrackFrequency(HBINK, int) { return 0; }
inline unsigned int BinkGetTrackType(HBINK, int) { return 0; }
inline bool BinkGetTrackData(HBINK, void*, void** outData, unsigned int* outSize)
{
        if (outData) {
                *outData = nullptr;
        }
        if (outSize) {
                *outSize = 0;
        }
        return false;
}

inline bool BinkSoundUseDirectSound(void*) { return false; }

inline HBINK BinkOpen(const char*, unsigned int)
{
        return new BINK{};
}

inline void BinkClose(HBINK handle)
{
        delete handle;
}

inline int BinkWait(HBINK)
{
        return 1;
}

inline void BinkDoFrame(HBINK) {}
inline void BinkNextFrame(HBINK) {}
inline void BinkCopyToBuffer(HBINK, void*, int, int, int, int, unsigned int) {}
inline void BinkGoto(HBINK, int, void*) {}
inline void BinkSetVolume(HBINK, int, int) {}

#endif  // !_WIN32

