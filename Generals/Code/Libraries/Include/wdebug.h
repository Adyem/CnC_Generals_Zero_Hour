// SPDX-License-Identifier: GPL-3.0-or-later
//
// Provide a common include path for the legacy wdebug helper. The original
// project searched for "wdebug.h" from a shared SDK include directory, but in
// this repository the implementation lives inside the MatchBot tool sources.
// Expose it here so that both the tool and any library code that expects the
// historic header name can include it without needing Windows-specific search
// paths.

#pragma once

#include "../Tools/matchbot/wlib/wdebug.h"
