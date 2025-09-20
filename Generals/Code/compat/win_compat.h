#pragma once

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(FORCE_WIN32_COMPAT)

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef _cdecl
#define _cdecl
#endif

using BOOL = int;
using BYTE = std::uint8_t;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using UINT = unsigned int;
using ULONG = std::uint32_t;
using LONG = std::int32_t;
using LPARAM = std::intptr_t;
using WPARAM = std::uintptr_t;
using LRESULT = std::intptr_t;

using HANDLE = void*;
using HINSTANCE = void*;
using HMODULE = void*;
using HWND = void*;
using HRSRC = void*;
using HGLOBAL = void*;
using HCURSOR = void*;
using HICON = void*;
using FARPROC = void (*)();

using LPVOID = void*;
using LPCVOID = const void*;
using LPSTR = char*;
using LPCSTR = const char*;
using LPTSTR = char*;
using LPCTSTR = const char*;
using LPWSTR = wchar_t*;
using LPCWSTR = const wchar_t*;
using WCHAR = wchar_t;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct _EXCEPTION_POINTERS;
using EXCEPTION_POINTERS = _EXCEPTION_POINTERS;

#ifndef WINAPI
#define WINAPI
#endif

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

constexpr unsigned short LANG_NEUTRAL = 0x00;
constexpr unsigned short SUBLANG_DEFAULT = 0x01;
constexpr unsigned short SUBLANG_SYS_DEFAULT = 0x02;
constexpr unsigned short SUBLANG_ENGLISH_US = 0x01;

#ifndef MAKELANGID
#define MAKELANGID(p, s) ((unsigned short)(((unsigned short)(s) << 10) | (unsigned short)(p)))
#endif

constexpr DWORD FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100;
constexpr DWORD FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200;
constexpr DWORD FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000;

constexpr DWORD ERROR_SUCCESS = 0;
constexpr DWORD ERROR_CALL_NOT_IMPLEMENTED = 120;

struct POINT {
    LONG x;
    LONG y;
};

struct RECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

unsigned long GetLastError();
void SetLastError(unsigned long error);

HMODULE LoadLibraryA(const char* fileName);
FARPROC GetProcAddressA(HMODULE handle, const char* procName);
BOOL FreeLibrary(HMODULE handle);

#define LoadLibrary LoadLibraryA
#define GetProcAddress GetProcAddressA

int LoadStringA(HINSTANCE instance, unsigned int id, char* buffer, int bufferMax);
#define LoadString LoadStringA

HRSRC FindResourceA(HINSTANCE instance, const char* name, const char* type);
HGLOBAL LoadResource(HINSTANCE instance, HRSRC resource);
void* LockResource(HGLOBAL resource);
DWORD SizeofResource(HINSTANCE instance, HRSRC resource);

unsigned long GetTickCount();

int FindExecutableA(const char* file, const char* directory, char* result);
#define FindExecutable FindExecutableA

unsigned long FormatMessageA(unsigned long flags,
                             const void* source,
                             unsigned long messageId,
                             unsigned long languageId,
                             char* buffer,
                             unsigned long size,
                             va_list* arguments);
unsigned long FormatMessageW(unsigned long flags,
                             const void* source,
                             unsigned long messageId,
                             unsigned long languageId,
                             wchar_t* buffer,
                             unsigned long size,
                             va_list* arguments);

#define FormatMessage FormatMessageA

void LocalFree(void* memory);

void OutputDebugStringA(const char* message);
#define OutputDebugString OutputDebugStringA

HMODULE GetModuleHandleA(const char* name);
#define GetModuleHandle GetModuleHandleA

unsigned long GetModuleFileNameA(HMODULE module, char* buffer, unsigned long size);
#define GetModuleFileName GetModuleFileNameA

#include "string_compat.h"

#endif // platform compat block

