#pragma once

// Minimal subset of the RAD Game Tools Bink API required to compile the
// cross-platform build.  The real SDK is proprietary and only available on
// Windows.  We provide lightweight stand-ins for the handle and a few helper
// structures so that code can be compiled without the SDK.

#ifndef _WIN32

#include <cstdint>

using HBINK = void*;

struct BINKRECT
{
        std::int32_t Left;
        std::int32_t Top;
        std::int32_t Width;
        std::int32_t Height;
};

#endif  // !_WIN32

