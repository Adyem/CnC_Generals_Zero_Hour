// SPDX-License-Identifier: GPL-3.0-or-later
//
// Cross-platform millisecond sleep helper used by legacy tooling. The Windows
// sources relied on the Win32 `Sleep` routine; provide an inline wrapper that
// maps to `std::this_thread::sleep_for` so that the same call sites compile on
// modern POSIX toolchains.

#ifndef GENERALS_CODE_LIBRARIES_INCLUDE_MSLEEP_H
#define GENERALS_CODE_LIBRARIES_INCLUDE_MSLEEP_H

#include <chrono>
#include <thread>

inline void msleep(unsigned int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

#endif // GENERALS_CODE_LIBRARIES_INCLUDE_MSLEEP_H
