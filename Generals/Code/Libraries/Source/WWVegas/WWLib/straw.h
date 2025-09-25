// SPDX-License-Identifier: GPL-3.0-or-later
//
// Case-normalization shim for the legacy STRAW.H header. Several call sites
// include "straw.h" directly; provide the expected name on case-sensitive
// platforms.

#pragma once

#include "STRAW.H"
