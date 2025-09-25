#pragma once

// Platform abstraction helpers used throughout the original codebase.
// The legacy projects relied on a Windows-only "osdep" layer to collect
// commonly used Win32 headers and keywords.  For the Unix oriented build we
// provide lightweight shims that map the expected Windows types and calling
// conventions onto the portable compatibility headers that already exist in
// the modernised codebase.

#if defined(_WIN32)

// The Windows toolchain still uses the original header layout.
#include <windows.h>

#else

#include "compat/win_compat.h"

// The compatibility header already re-exports most Win32 types and helpers,
// but a few MSVC specific keywords occasionally leak through in the older
// source files.  Define them away so that clang/gcc can accept the code.
#ifndef __forceinline
#define __forceinline inline
#endif

#ifndef __declspec
#define __declspec(_x)
#endif

#ifndef __fastcall
#define __fastcall
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef __declspec_novtable
#define __declspec_novtable
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif

#endif // platform check

