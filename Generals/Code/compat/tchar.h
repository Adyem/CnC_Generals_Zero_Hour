/*
**      Command & Conquer Generals(tm)
**      Portable TCHAR compatibility layer for non-Windows builds.
*/

#pragma once

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef char TCHAR;
typedef char* PTCHAR;
typedef char* LPTSTR;
typedef const char* LPCTSTR;
typedef const char* PCTSTR;

#define _T(x) x
#define _TEXT(x) x
#define TEXT(x) x

#define _tcslen strlen
#define _tcsnlen strnlen
#define _tcscmp strcmp
#define _tcsicmp strcasecmp
#define _tcsncmp strncmp
#define _tcsnicmp strncasecmp
#define _tcscpy strcpy
#define _tcscat strcat
#define _tcsncpy strncpy
#define _tcsncat strncat
#define _tcsstr strstr
#define _tcschr strchr
#define _tcsrchr strrchr
#define _tcsdup strdup

static inline char* _tcsupr(char* s) {
    if (!s) {
        return NULL;
    }
    for (char* p = s; *p; ++p) {
        *p = (char)toupper((unsigned char)*p);
    }
    return s;
}

static inline char* _tcslwr(char* s) {
    if (!s) {
        return NULL;
    }
    for (char* p = s; *p; ++p) {
        *p = (char)tolower((unsigned char)*p);
    }
    return s;
}

#define _stprintf sprintf
#define _sntprintf snprintf
#define _vsntprintf vsnprintf
#define _tfopen fopen
#define _tprintf printf

#define _tstoi atoi
#define _tstof atof

using LPTCSTR = const char*;
using LPTCH = char*;
using PTSTR = char*;

