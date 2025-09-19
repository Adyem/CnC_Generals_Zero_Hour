#pragma once

// Minimal DirectInput compatibility header for non-Windows builds.  Only the
// types referenced by the Command & Conquer Generals headers are declared
// here, enough to satisfy the compiler when the real DirectInput SDK is not
// available.

#ifndef _WIN32

#include <cstdint>

using LPDIRECTINPUT8 = void*;
using LPDIRECTINPUTDEVICE8 = void*;

struct DIDEVICEOBJECTDATA
{
        std::uint32_t dwOfs;
        std::uint32_t dwData;
        std::uint32_t dwTimeStamp;
        std::uint32_t dwSequence;
        std::uintptr_t uAppData;
};

#endif  // !_WIN32

