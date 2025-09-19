#pragma once

#ifndef _WIN32
#include <cstddef>
#include <strings.h>
#include <wchar.h>

inline int _stricmp(const char* lhs, const char* rhs) { return strcasecmp(lhs, rhs); }
inline int _strnicmp(const char* lhs, const char* rhs, size_t count) { return strncasecmp(lhs, rhs, count); }
inline int _wcsicmp(const wchar_t* lhs, const wchar_t* rhs) { return wcscasecmp(lhs, rhs); }
inline int _wcsnicmp(const wchar_t* lhs, const wchar_t* rhs, size_t count) { return wcsncasecmp(lhs, rhs, count); }
inline int stricmp(const char* lhs, const char* rhs) { return _stricmp(lhs, rhs); }
inline int strnicmp(const char* lhs, const char* rhs, size_t count) { return _strnicmp(lhs, rhs, count); }
#endif  // _WIN32

