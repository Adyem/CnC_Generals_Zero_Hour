
#if defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(FORCE_WIN32_COMPAT)

#include <cstdarg>

#include "Compat/Windows/windows_compat.h"

#ifndef __cdecl
#define __cdecl
#endif

#ifndef __stdcall
#define __stdcall
#endif

#ifndef _cdecl
#define _cdecl
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SEVERITY_SUCCESS
#define SEVERITY_SUCCESS 0
#endif

#ifndef SEVERITY_ERROR
#define SEVERITY_ERROR 1
#endif

#ifndef FACILITY_ITF
#define FACILITY_ITF 4
#endif

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev, fac, code) \
    (static_cast<cnc::windows::HRESULT>(((static_cast<unsigned long>(sev) << 31) | \
                                         (static_cast<unsigned long>(fac) << 16) | \
                                         (static_cast<unsigned long>(code)))))
#endif

constexpr unsigned short LANG_NEUTRAL = 0x00;
constexpr unsigned short SUBLANG_DEFAULT = 0x01;
constexpr unsigned short SUBLANG_SYS_DEFAULT = 0x02;
constexpr unsigned short SUBLANG_ENGLISH_US = 0x01;

#ifndef MAKELANGID
#define MAKELANGID(p, s) ((unsigned short)(((unsigned short)(s) << 10) | (unsigned short)(p)))
#endif

constexpr cnc::windows::DWORD FORMAT_MESSAGE_ALLOCATE_BUFFER = 0x00000100;
constexpr cnc::windows::DWORD FORMAT_MESSAGE_IGNORE_INSERTS = 0x00000200;
constexpr cnc::windows::DWORD FORMAT_MESSAGE_FROM_SYSTEM = 0x00001000;

constexpr cnc::windows::DWORD ERROR_SUCCESS = 0;
constexpr cnc::windows::DWORD ERROR_CALL_NOT_IMPLEMENTED = 120;
constexpr cnc::windows::DWORD ERROR_OUTOFMEMORY = 14;

namespace cnc::windows
{
using FARPROC = void (*)();
using HRSRC = void*;
using HGLOBAL = void*;

HMODULE LoadLibraryA(const char* fileName);
FARPROC GetProcAddressA(HMODULE handle, const char* procName);
BOOL FreeLibrary(HMODULE handle);

int LoadStringA(HINSTANCE instance, unsigned int id, char* buffer, int bufferMax);

HRSRC FindResourceA(HINSTANCE instance, const char* name, const char* type);
HGLOBAL LoadResource(HINSTANCE instance, HRSRC resource);
void* LockResource(HGLOBAL resource);
DWORD SizeofResource(HINSTANCE instance, HRSRC resource);

DWORD GetTickCount();

int FindExecutableA(const char* file, const char* directory, char* result);

DWORD FormatMessageA(DWORD flags,
                     const void* source,
                     DWORD messageId,
                     DWORD languageId,
                     char* buffer,
                     DWORD size,
                     va_list* arguments);
DWORD FormatMessageW(DWORD flags,
                     const void* source,
                     DWORD messageId,
                     DWORD languageId,
                     wchar_t* buffer,
                     DWORD size,
                     va_list* arguments);

void LocalFree(void* memory);

void OutputDebugStringA(LPCSTR message);

HMODULE GetModuleHandleA(LPCSTR name);
DWORD GetModuleFileNameA(HMODULE module, LPSTR buffer, DWORD size);

DWORD GetLastError();
void SetLastError(DWORD error);

inline int MulDiv(int number, int numerator, int denominator)
{
    if (denominator == 0)
    {
        return 0;
    }

    long long result = static_cast<long long>(number) * static_cast<long long>(numerator);
    return static_cast<int>(result / denominator);
}
} // namespace cnc::windows

using cnc::windows::FARPROC;
using cnc::windows::FindExecutableA;
using cnc::windows::FormatMessageA;
using cnc::windows::FormatMessageW;
using cnc::windows::FreeLibrary;
using cnc::windows::GetLastError;
using cnc::windows::GetModuleFileNameA;
using cnc::windows::GetModuleHandleA;
using cnc::windows::GetProcAddressA;
using cnc::windows::GetTickCount;
using cnc::windows::HMODULE;
using cnc::windows::LoadLibraryA;
using cnc::windows::LoadResource;
using cnc::windows::LoadStringA;
using cnc::windows::LockResource;
using cnc::windows::LocalFree;
using cnc::windows::MulDiv;
using cnc::windows::OutputDebugStringA;
using cnc::windows::SizeofResource;
using cnc::windows::SetLastError;

#define LoadLibrary LoadLibraryA
#define GetProcAddress GetProcAddressA
#define LoadString LoadStringA
#define FindExecutable FindExecutableA
#define FormatMessage FormatMessageA
#define OutputDebugString OutputDebugStringA
#define GetModuleHandle GetModuleHandleA
#define GetModuleFileName GetModuleFileNameA

#include "string_compat.h"

#endif // platform compat block

