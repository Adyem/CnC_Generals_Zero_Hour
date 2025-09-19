#pragma once

#ifndef _WIN32

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <initializer_list>
#include <thread>
#include <type_traits>
#include <unistd.h>

namespace cnc::windows
{
using BOOL = int;
using BYTE = unsigned char;
using UCHAR = unsigned char;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
using QWORD = std::uint64_t;
using SHORT = short;
using USHORT = unsigned short;
using INT = int;
using UINT = unsigned int;
using LONG = long;
using ULONG = unsigned long;
using LONGLONG = std::int64_t;
using ULONGLONG = std::uint64_t;
using DWORDLONG = std::uint64_t;
using FLOAT = float;
using DOUBLE = double;
using HRESULT = long;
using HANDLE = void*;
using HINSTANCE = void*;
using HWND = void*;
using HDC = void*;
using HGLRC = void*;
using HMODULE = void*;
using HBITMAP = void*;
using HBRUSH = void*;
using HFONT = void*;
using HPALETTE = void*;
using HICON = void*;
using HMENU = void*;
using HCURSOR = void*;
using HGDIOBJ = void*;
using HPEN = void*;
using HKEY = void*;
using LPVOID = void*;
using LPCVOID = const void*;
using LPBYTE = BYTE*;
using LPSTR = char*;
using LPCSTR = const char*;
using LPWSTR = wchar_t*;
using LPCWSTR = const wchar_t*;
using LPTSTR = char*;
using LPCTSTR = const char*;
using LPBOOL = BOOL*;
using LPWORD = WORD*;
using LPDWORD = DWORD*;
using WPARAM = std::uintptr_t;
using LPARAM = std::intptr_t;
using LRESULT = std::intptr_t;
using SIZE_T = std::size_t;
using ULONG_PTR = std::uintptr_t;
using LONG_PTR = std::intptr_t;
using DWORD_PTR = std::uintptr_t;
using UINT_PTR = std::uintptr_t;
using INT_PTR = std::intptr_t;
using LCID = unsigned long;
using DISPID = long;
using OLECHAR = wchar_t;
using LPOLESTR = OLECHAR*;
using LPCOLESTR = const OLECHAR*;

struct RECT
{
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
};

struct POINT
{
    LONG x;
    LONG y;
};

struct SIZE
{
    LONG cx;
    LONG cy;
};

#define cnc_WINAPI
#define cnc_CALLBACK
#define cnc_APIENTRY

inline void Sleep(DWORD milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

inline UINT timeBeginPeriod(UINT)
{
    return 0;
}

inline UINT timeEndPeriod(UINT)
{
    return 0;
}

inline DWORD timeGetTime()
{
    using namespace std::chrono;
    return static_cast<DWORD>(duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

inline DWORD GetTickCount()
{
    return timeGetTime();
}

inline void ZeroMemory(void* destination, std::size_t size)
{
    std::memset(destination, 0, size);
}

inline void CopyMemory(void* destination, const void* source, std::size_t size)
{
    std::memcpy(destination, source, size);
}

inline void MoveMemory(void* destination, const void* source, std::size_t size)
{
    std::memmove(destination, source, size);
}

constexpr BOOL TRUE_VALUE = 1;
constexpr BOOL FALSE_VALUE = 0;
constexpr DWORD INFINITE = 0xffffffffu;
constexpr DWORD MAX_PATH = 260;

constexpr HRESULT S_OK = 0;
constexpr HRESULT S_FALSE = 1;
constexpr HRESULT E_FAIL = static_cast<HRESULT>(0x80004005L);
constexpr HRESULT E_POINTER = static_cast<HRESULT>(0x80004003L);
constexpr HRESULT E_OUTOFMEMORY = static_cast<HRESULT>(0x8007000EL);
constexpr HRESULT E_NOTIMPL = static_cast<HRESULT>(0x80004001L);
constexpr HRESULT E_NOINTERFACE = static_cast<HRESULT>(0x80004002L);

inline bool SUCCEEDED(HRESULT value)
{
    return value >= 0;
}

inline bool FAILED(HRESULT value)
{
    return value < 0;
}

struct GUID
{
    std::uint32_t Data1;
    std::uint16_t Data2;
    std::uint16_t Data3;
    std::uint8_t Data4[8];
};

using IID = GUID;
using CLSID = GUID;

#define cnc_DECLSPEC_UUID(uuid_string)

#define cnc_REFCLSID const ::cnc::windows::CLSID&
#define cnc_REFIID const ::cnc::windows::IID&
#define cnc_REFGUID const ::cnc::windows::GUID&

inline bool IsEqualGUID(cnc_REFGUID lhs, cnc_REFGUID rhs)
{
    return lhs.Data1 == rhs.Data1 && lhs.Data2 == rhs.Data2 && lhs.Data3 == rhs.Data3 &&
           std::memcmp(lhs.Data4, rhs.Data4, sizeof(lhs.Data4)) == 0;
}

struct IUnknown
{
    virtual ~IUnknown() = default;
    virtual HRESULT QueryInterface(cnc_REFIID, void**)
    {
        return E_NOINTERFACE;
    }
    virtual ULONG AddRef()
    {
        return 1;
    }
    virtual ULONG Release()
    {
        return 1;
    }
};

struct IDispatch : public IUnknown
{
    virtual HRESULT GetTypeInfoCount(UINT*)
    {
        return E_NOTIMPL;
    }
    virtual HRESULT GetTypeInfo(UINT, LCID, void**)
    {
        return E_NOTIMPL;
    }
    virtual HRESULT GetIDsOfNames(cnc_REFIID, LPOLESTR*, UINT, LCID, DISPID*)
    {
        return E_NOTIMPL;
    }
    virtual HRESULT Invoke(DISPID, cnc_REFIID, LCID, unsigned short, void*, void*, void*, UINT*)
    {
        return E_NOTIMPL;
    }
};

using LPDISPATCH = IDispatch*;
using LPUNKNOWN = IUnknown*;

using BSTR = wchar_t*;
using VARIANT_BOOL = short;
constexpr VARIANT_BOOL VARIANT_TRUE = -1;
constexpr VARIANT_BOOL VARIANT_FALSE = 0;

enum VARTYPE : std::uint16_t
{
    VT_EMPTY = 0,
    VT_NULL = 1,
    VT_I2 = 2,
    VT_I4 = 3,
    VT_R4 = 4,
    VT_R8 = 5,
    VT_CY = 6,
    VT_DATE = 7,
    VT_BSTR = 8,
    VT_DISPATCH = 9,
    VT_ERROR = 10,
    VT_BOOL = 11,
    VT_VARIANT = 12,
    VT_UNKNOWN = 13,
    VT_I1 = 16,
    VT_UI1 = 17,
    VT_UI2 = 18,
    VT_UI4 = 19,
    VT_I8 = 20,
    VT_UI8 = 21,
    VT_INT = 22,
    VT_UINT = 23,
};

struct VARIANT
{
    VARTYPE vt;
    std::uint16_t wReserved1;
    std::uint16_t wReserved2;
    std::uint16_t wReserved3;
    union
    {
        LONGLONG llVal;
        LONG lVal;
        BYTE bVal;
        SHORT iVal;
        FLOAT fltVal;
        DOUBLE dblVal;
        VARIANT_BOOL boolVal;
        BSTR bstrVal;
        IUnknown* punkVal;
        IDispatch* pdispVal;
        void* byref;
    } n1;

    VARIANT() : vt(VT_EMPTY), wReserved1(0), wReserved2(0), wReserved3(0)
    {
        n1.byref = nullptr;
    }
};

inline void VariantInit(VARIANT* value)
{
    if (value)
    {
        *value = VARIANT{};
    }
}

inline void VariantClear(VARIANT* value)
{
    if (!value)
    {
        return;
    }

    if (value->vt == VT_BSTR && value->n1.bstrVal)
    {
        delete[] value->n1.bstrVal;
    }

    value->vt = VT_EMPTY;
    value->n1.byref = nullptr;
}

inline int lstrcmpiA(LPCSTR left, LPCSTR right)
{
    return ::strcasecmp(left, right);
}

inline int lstrlenA(LPCSTR value)
{
    return static_cast<int>(std::strlen(value));
}

inline void OutputDebugStringA(LPCSTR message)
{
    if (message)
    {
        std::fprintf(stderr, "%s", message);
    }
}

inline DWORD GetLastError()
{
    return 0;
}

inline void SetLastError(DWORD)
{
}

} // namespace cnc::windows

using cnc::windows::BOOL;
using cnc::windows::BYTE;
using cnc::windows::UCHAR;
using cnc::windows::WORD;
using cnc::windows::DWORD;
using cnc::windows::QWORD;
using cnc::windows::SHORT;
using cnc::windows::USHORT;
using cnc::windows::INT;
using cnc::windows::UINT;
using cnc::windows::LONG;
using cnc::windows::ULONG;
using cnc::windows::LONGLONG;
using cnc::windows::ULONGLONG;
using cnc::windows::FLOAT;
using cnc::windows::DOUBLE;
using cnc::windows::HRESULT;
using cnc::windows::HANDLE;
using cnc::windows::HINSTANCE;
using cnc::windows::HWND;
using cnc::windows::HDC;
using cnc::windows::HGLRC;
using cnc::windows::HMODULE;
using cnc::windows::HBITMAP;
using cnc::windows::HBRUSH;
using cnc::windows::HFONT;
using cnc::windows::HPALETTE;
using cnc::windows::HICON;
using cnc::windows::HMENU;
using cnc::windows::HCURSOR;
using cnc::windows::HGDIOBJ;
using cnc::windows::HPEN;
using cnc::windows::HKEY;
using cnc::windows::LPVOID;
using cnc::windows::LPCVOID;
using cnc::windows::LPBYTE;
using cnc::windows::LPSTR;
using cnc::windows::LPCSTR;
using cnc::windows::LPWSTR;
using cnc::windows::LPCWSTR;
using cnc::windows::LPTSTR;
using cnc::windows::LPCTSTR;
using cnc::windows::LPBOOL;
using cnc::windows::LPWORD;
using cnc::windows::LPDWORD;
using cnc::windows::WPARAM;
using cnc::windows::LPARAM;
using cnc::windows::LRESULT;
using cnc::windows::SIZE_T;
using cnc::windows::ULONG_PTR;
using cnc::windows::LONG_PTR;
using cnc::windows::DWORD_PTR;
using cnc::windows::UINT_PTR;
using cnc::windows::INT_PTR;
using cnc::windows::LCID;
using cnc::windows::DISPID;
using cnc::windows::OLECHAR;
using cnc::windows::LPOLESTR;
using cnc::windows::LPCOLESTR;
using cnc::windows::RECT;
using cnc::windows::POINT;
using cnc::windows::SIZE;
using cnc::windows::GUID;
using cnc::windows::IID;
using cnc::windows::CLSID;
using cnc::windows::IUnknown;
using cnc::windows::IDispatch;
using cnc::windows::LPDISPATCH;
using cnc::windows::LPUNKNOWN;
using cnc::windows::BSTR;
using cnc::windows::VARIANT_BOOL;
using cnc::windows::VARIANT;
using cnc::windows::VARTYPE;

using cnc::windows::TRUE_VALUE;
using cnc::windows::FALSE_VALUE;
using cnc::windows::INFINITE;
using cnc::windows::MAX_PATH;
using cnc::windows::S_OK;
using cnc::windows::S_FALSE;
using cnc::windows::E_FAIL;
using cnc::windows::E_POINTER;
using cnc::windows::E_OUTOFMEMORY;
using cnc::windows::E_NOTIMPL;
using cnc::windows::E_NOINTERFACE;
using cnc::windows::VARIANT_TRUE;
using cnc::windows::VARIANT_FALSE;

using cnc::windows::Sleep;
using cnc::windows::timeBeginPeriod;
using cnc::windows::timeEndPeriod;
using cnc::windows::timeGetTime;
using cnc::windows::GetTickCount;
using cnc::windows::ZeroMemory;
using cnc::windows::CopyMemory;
using cnc::windows::MoveMemory;
using cnc::windows::SUCCEEDED;
using cnc::windows::FAILED;
using cnc::windows::IsEqualGUID;
using cnc::windows::VariantInit;
using cnc::windows::VariantClear;
using cnc::windows::lstrcmpiA;
using cnc::windows::lstrlenA;
using cnc::windows::OutputDebugStringA;
using cnc::windows::GetLastError;
using cnc::windows::SetLastError;

#define WINAPI cnc_WINAPI
#define CALLBACK cnc_CALLBACK
#define APIENTRY cnc_APIENTRY
#define STDMETHODCALLTYPE
#define STDAPICALLTYPE
#define STDMETHODIMP HRESULT
#define STDMETHODIMP_(type) type
#define STDMETHOD(method) virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type, method) virtual type STDMETHODCALLTYPE method
#define STDMETHODCALLTYPE
#define DECLSPEC_UUID(x) cnc_DECLSPEC_UUID(x)
#define REFGUID cnc_REFGUID
#define REFIID cnc_REFIID
#define REFCLSID cnc_REFCLSID

#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<std::intptr_t>(-1))

#define LOWORD(l) static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffff)
#define HIWORD(l) static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffff)
#define LOBYTE(w) static_cast<BYTE>(static_cast<DWORD_PTR>(w) & 0xff)
#define HIBYTE(w) static_cast<BYTE>((static_cast<DWORD_PTR>(w) >> 8) & 0xff)

#define MAKELONG(a, b) static_cast<LONG>((static_cast<DWORD>(static_cast<WORD>(a)) & 0xffff) | (static_cast<DWORD>(static_cast<WORD>(b)) << 16))
#define MAKEWORD(a, b) static_cast<WORD>((static_cast<BYTE>(a) & 0xff) | ((static_cast<WORD>(static_cast<BYTE>(b)) & 0xff) << 8))

#endif // !_WIN32

