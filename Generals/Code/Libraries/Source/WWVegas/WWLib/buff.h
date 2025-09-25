// SPDX-License-Identifier: GPL-3.0-or-later
//
// Case-insensitive wrapper so that legacy includes written as "buff.h" resolve
// on case-sensitive filesystems. The original sources shipped the header as
// BUFF.H; mirror that layout here without forcing invasive include edits.

#pragma once

#include "BUFF.H"
