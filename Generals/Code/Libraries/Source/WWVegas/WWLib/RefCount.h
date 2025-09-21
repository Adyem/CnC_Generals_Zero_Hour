#pragma once

// Case-preserving shim for builds on case-sensitive file systems.  The legacy
// code includes "WWLib/RefCount.h" but the repository stores the file in all
// lowercase.

#include "refcount.h"
