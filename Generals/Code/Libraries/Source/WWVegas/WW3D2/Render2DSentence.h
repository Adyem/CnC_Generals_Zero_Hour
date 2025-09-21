#pragma once

// Case-insensitive include compatibility shim.  The original project relied
// on Windows paths and included this header as "WW3D2/Render2DSentence.h" even
// though the file on disk is named in lowercase.  Forward the include so the
// code builds on case-sensitive platforms.

#include "render2dsentence.h"
