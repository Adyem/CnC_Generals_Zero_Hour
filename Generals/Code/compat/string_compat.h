
#ifndef _WIN32
#include <cctype>
#include <cstddef>
#include <strings.h>
#include <wchar.h>

inline int _stricmp(const char* lhs, const char* rhs) { return strcasecmp(lhs, rhs); }
inline int _strnicmp(const char* lhs, const char* rhs, size_t count) { return strncasecmp(lhs, rhs, count); }
inline int _wcsicmp(const wchar_t* lhs, const wchar_t* rhs) { return wcscasecmp(lhs, rhs); }
inline int _wcsnicmp(const wchar_t* lhs, const wchar_t* rhs, size_t count) { return wcsncasecmp(lhs, rhs, count); }
inline int stricmp(const char* lhs, const char* rhs) { return _stricmp(lhs, rhs); }
inline int strcmpi(const char* lhs, const char* rhs) { return _stricmp(lhs, rhs); }
inline int strnicmp(const char* lhs, const char* rhs, size_t count) { return _strnicmp(lhs, rhs, count); }
inline char* strupr(char* value)
{
    if (value == nullptr)
    {
        return nullptr;
    }

    for (char* cursor = value; *cursor != '\0'; ++cursor)
    {
        *cursor = static_cast<char>(std::toupper(static_cast<unsigned char>(*cursor)));
    }
    return value;
}

inline char* _strupr(char* value)
{
    return strupr(value);
}
#endif  // _WIN32

