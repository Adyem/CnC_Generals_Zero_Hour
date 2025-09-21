#pragma once

// Windows file systems are case-insensitive, but most POSIX platforms are not.
// The original code base frequently includes this header using the mixed case
// path "WW3D2/WW3DFormat.h" even though the actual file on disk is named
// "ww3dformat.h".  Provide a thin wrapper that simply forwards to the real
// header so builds succeed on case-sensitive file systems.

#include "ww3dformat.h"
