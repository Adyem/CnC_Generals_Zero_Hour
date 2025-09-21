#pragma once

// Case-preserving shim so code that includes "WWMath/Vector3.h" resolves to
// the lowercase header on disk when building on case-sensitive platforms.

#include "vector3.h"
