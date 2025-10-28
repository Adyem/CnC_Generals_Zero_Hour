
#ifdef _WIN32
#include <ddraw.h>
#else

// Minimal DirectDraw stub for non-Windows builds.
// Provides just enough type definitions for legacy interfaces that are
// guarded behind WW3D_ENABLE_LEGACY_DX8.

#include "../Windows/windows_compat.h"

#include <cstdint>

using cnc::windows::BOOL;
using cnc::windows::BYTE;
using cnc::windows::DWORD;
using cnc::windows::HRESULT;
using cnc::windows::LONG;

struct IDirectDraw;
struct IDirectDraw2;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

using LPDIRECTDRAW = IDirectDraw*;
using LPDIRECTDRAW2 = IDirectDraw2*;
using LPDIRECTDRAWSURFACE = IDirectDrawSurface*;
using LPDIRECTDRAWPALETTE = IDirectDrawPalette*;
using LPDIRECTDRAWCLIPPER = IDirectDrawClipper*;

struct DDPIXELFORMAT
{
    DWORD dwSize = 0;
    DWORD dwFlags = 0;
    DWORD dwFourCC = 0;
    DWORD dwRGBBitCount = 0;
    DWORD dwRBitMask = 0;
    DWORD dwGBitMask = 0;
    DWORD dwBBitMask = 0;
    DWORD dwRGBAlphaBitMask = 0;
};

struct DDSCAPS
{
    DWORD dwCaps = 0;
};

struct DDSURFACEDESC
{
    DWORD dwSize = 0;
    DWORD dwFlags = 0;
    DWORD dwHeight = 0;
    DWORD dwWidth = 0;
    LONG lPitch = 0;
    void* lpSurface = nullptr;
    DDPIXELFORMAT ddpfPixelFormat{};
    DDSCAPS ddsCaps{};
};

struct PALETTEENTRY
{
    BYTE peRed = 0;
    BYTE peGreen = 0;
    BYTE peBlue = 0;
    BYTE peFlags = 0;
};

constexpr DWORD DDSCAPS_OFFSCREENPLAIN = 0;
constexpr DWORD DDSCAPS_SYSTEMMEMORY = 0;
constexpr DWORD DDSCAPS_VIDEOMEMORY = 0;
constexpr DWORD DDSD_CAPS = 0;
constexpr DWORD DDSD_HEIGHT = 0;
constexpr DWORD DDSD_WIDTH = 0;
constexpr DWORD DDSD_PIXELFORMAT = 0;

constexpr HRESULT DD_OK = 0;
constexpr HRESULT DDERR_GENERIC = static_cast<HRESULT>(-1);

constexpr DWORD DDWAITVB_BLOCKBEGIN = 0;
constexpr DWORD DDBLT_WAIT = 0;
constexpr DWORD DDBLTFAST_NOCOLORKEY = 0;
constexpr DWORD DDBLTFAST_SRCCOLORKEY = 0;
constexpr DWORD DDFLIP_WAIT = 0;
constexpr DWORD DDSCL_NORMAL = 0;
constexpr DWORD DDSCL_EXCLUSIVE = 0;
constexpr DWORD DDSCL_FULLSCREEN = 0;
constexpr DWORD DDPCAPS_8BIT = 0;
constexpr DWORD DDPCAPS_ALLOW256 = 0;
constexpr DWORD DDPCAPS_INITIALIZE = 0;
constexpr DWORD DDGBS_ISBLTDONE = 0;

using cnc::windows::RECT;

#endif // _WIN32
