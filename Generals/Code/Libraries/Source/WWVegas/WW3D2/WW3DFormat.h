#ifndef GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_WW3DFORMAT_H
#define GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_WW3DFORMAT_H

// Windows file systems are case-insensitive, but most POSIX platforms are not.
// The original code base frequently includes this header using the mixed case
// path "WW3D2/WW3DFormat.h" even though the actual file on disk is named
// "ww3dformat.h".  Provide a thin wrapper that simply forwards to the real
// header so builds succeed on case-sensitive file systems.

#include "ww3dformat.h"
#endif // GENERALS_CODE_LIBRARIES_SOURCE_WWVEGAS_WW3D2_WW3DFORMAT_H
