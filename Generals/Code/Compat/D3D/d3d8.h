
// Minimal Direct3D 8 compatibility declarations for non-Windows builds.
// These definitions are sufficient to satisfy the subset of the legacy
// Command & Conquer Generals headers that refer to Direct3D interfaces.
// They intentionally do not provide any functionality beyond the type
// declarations required for compilation.

#ifndef _WIN32

#include <cstdint>

#include "../Windows/windows_compat.h"
#include "d3d8types.h"

using HRESULT = cnc::windows::HRESULT;
using DWORD = cnc::windows::DWORD;
using D3DCOLOR = cnc::windows::DWORD;
using UINT = cnc::windows::UINT;
using ULONG = cnc::windows::ULONG;
using RECT = cnc::windows::RECT;
using POINT = cnc::windows::POINT;
using PALETTEENTRY = cnc::windows::PALETTEENTRY;

struct IDirect3D8;
struct IDirect3DDevice8;
struct IDirect3DTexture8;
struct IDirect3DSurface8;
struct IDirect3DBaseTexture8;
struct IDirect3DVertexBuffer8;
struct IDirect3DIndexBuffer8;
struct IDirect3DStateBlock8;
struct IDirect3DVolumeTexture8;
struct IDirect3DCubeTexture8;
struct IDirect3DSwapChain8;

using LPDIRECT3D8 = IDirect3D8*;
using LPDIRECT3DDEVICE8 = IDirect3DDevice8*;
using LPDIRECT3DTEXTURE8 = IDirect3DTexture8*;
using LPDIRECT3DSURFACE8 = IDirect3DSurface8*;
using LPDIRECT3DVERTEXBUFFER8 = IDirect3DVertexBuffer8*;
using LPDIRECT3DINDEXBUFFER8 = IDirect3DIndexBuffer8*;
using LPDIRECT3DSTATEBLOCK8 = IDirect3DStateBlock8*;
using LPDIRECT3DVOLUMETEXTURE8 = IDirect3DVolumeTexture8*;
using LPDIRECT3DCUBETEXTURE8 = IDirect3DCubeTexture8*;
using LPDIRECT3DSWAPCHAIN8 = IDirect3DSwapChain8*;

constexpr HRESULT D3D_OK = 0;
constexpr HRESULT D3DERR_INVALIDCALL = -2005530516;      // 0x8876086C
constexpr HRESULT D3DERR_DEVICELOST = -2005530520;       // 0x88760868
constexpr HRESULT D3DERR_DEVICENOTRESET = -2005530519;   // 0x88760869
constexpr HRESULT D3DERR_OUTOFVIDEOMEMORY = -2005532292; // 0x8876017C
constexpr HRESULT D3DERR_NOTAVAILABLE = -2005530518;     // 0x8876086A

constexpr DWORD D3DFVF_RESERVED0 = 0x0001;
constexpr DWORD D3DFVF_POSITION_MASK = 0x400E;
constexpr DWORD D3DFVF_XYZ = 0x0002;
constexpr DWORD D3DFVF_XYZRHW = 0x0004;
constexpr DWORD D3DFVF_XYZB1 = 0x0006;
constexpr DWORD D3DFVF_XYZB2 = 0x0008;
constexpr DWORD D3DFVF_XYZB3 = 0x000A;
constexpr DWORD D3DFVF_XYZB4 = 0x000C;
constexpr DWORD D3DFVF_XYZB5 = 0x000E;
constexpr DWORD D3DFVF_XYZW = 0x4002;
constexpr DWORD D3DFVF_NORMAL = 0x0010;
constexpr DWORD D3DFVF_PSIZE = 0x0020;
constexpr DWORD D3DFVF_DIFFUSE = 0x0040;
constexpr DWORD D3DFVF_SPECULAR = 0x0080;
constexpr DWORD D3DFVF_TEXCOUNT_MASK = 0x0F00;
constexpr DWORD D3DFVF_TEXCOUNT_SHIFT = 8;
constexpr DWORD D3DFVF_TEX0 = 0x0000;
constexpr DWORD D3DFVF_TEX1 = 0x0100;
constexpr DWORD D3DFVF_TEX2 = 0x0200;
constexpr DWORD D3DFVF_TEX3 = 0x0300;
constexpr DWORD D3DFVF_TEX4 = 0x0400;
constexpr DWORD D3DFVF_TEX5 = 0x0500;
constexpr DWORD D3DFVF_TEX6 = 0x0600;
constexpr DWORD D3DFVF_TEX7 = 0x0700;
constexpr DWORD D3DFVF_TEX8 = 0x0800;
constexpr DWORD D3DFVF_LASTBETA_UBYTE4 = 0x1000;
constexpr DWORD D3DFVF_LASTBETA_D3DCOLOR = 0x8000;
constexpr DWORD D3DFVF_RESERVED2 = 0x6000;

constexpr DWORD D3DDP_MAXTEXCOORD = 8;

#define D3DFVF_TEXCOORDSIZE1(i) (static_cast<DWORD>(3) << ((i) * 2 + 16))
#define D3DFVF_TEXCOORDSIZE2(i) (static_cast<DWORD>(0) << ((i) * 2 + 16))
#define D3DFVF_TEXCOORDSIZE3(i) (static_cast<DWORD>(1) << ((i) * 2 + 16))
#define D3DFVF_TEXCOORDSIZE4(i) (static_cast<DWORD>(2) << ((i) * 2 + 16))

enum D3DMULTISAMPLE_TYPE : UINT
{
    D3DMULTISAMPLE_NONE = 0,
    D3DMULTISAMPLE_2_SAMPLES = 2,
    D3DMULTISAMPLE_3_SAMPLES = 3,
    D3DMULTISAMPLE_4_SAMPLES = 4,
    D3DMULTISAMPLE_5_SAMPLES = 5,
    D3DMULTISAMPLE_6_SAMPLES = 6,
    D3DMULTISAMPLE_7_SAMPLES = 7,
    D3DMULTISAMPLE_8_SAMPLES = 8,
    D3DMULTISAMPLE_9_SAMPLES = 9,
    D3DMULTISAMPLE_10_SAMPLES = 10,
    D3DMULTISAMPLE_11_SAMPLES = 11,
    D3DMULTISAMPLE_12_SAMPLES = 12,
    D3DMULTISAMPLE_13_SAMPLES = 13,
    D3DMULTISAMPLE_14_SAMPLES = 14,
    D3DMULTISAMPLE_15_SAMPLES = 15,
    D3DMULTISAMPLE_16_SAMPLES = 16,
    D3DMULTISAMPLE_FORCE_DWORD = 0xffffffffu
};

struct D3DSURFACE_DESC
{
    D3DFORMAT Format;
    D3DRESOURCETYPE Type;
    DWORD Usage;
    D3DPOOL Pool;
    D3DMULTISAMPLE_TYPE MultiSampleType;
    DWORD MultiSampleQuality;
    UINT Width;
    UINT Height;
};

struct IDirect3DBaseTexture8
{
    virtual ~IDirect3DBaseTexture8() = default;
    virtual void GenerateMipSubLevels() = 0;
};

struct IDirect3DDevice8;

struct IDirect3DSurface8
{
    virtual ~IDirect3DSurface8() = default;
    virtual HRESULT GetDesc(D3DSURFACE_DESC* desc) = 0;
    virtual HRESULT GetDevice(IDirect3DDevice8** device) = 0;
};

struct IDirect3DDevice8
{
    virtual ~IDirect3DDevice8() = default;
    virtual ULONG AddRef() = 0;
    virtual ULONG Release() = 0;
    virtual HRESULT CopyRects(IDirect3DSurface8* src,
                              const RECT* srcRects,
                              UINT rectCount,
                              IDirect3DSurface8* dest,
                              const POINT* destPoints) = 0;
};

struct IDirect3D8
{
    virtual ~IDirect3D8() = default;
};

struct IDirect3DTexture8
{
    virtual ~IDirect3DTexture8() = default;
};

struct IDirect3DVolumeTexture8
{
    virtual ~IDirect3DVolumeTexture8() = default;
};

struct IDirect3DCubeTexture8
{
    virtual ~IDirect3DCubeTexture8() = default;
};

struct IDirect3DVertexBuffer8
{
    virtual ~IDirect3DVertexBuffer8() = default;
};

struct IDirect3DIndexBuffer8
{
    virtual ~IDirect3DIndexBuffer8() = default;
};

struct IDirect3DStateBlock8
{
    virtual ~IDirect3DStateBlock8() = default;
};

struct IDirect3DSwapChain8
{
    virtual ~IDirect3DSwapChain8() = default;
};

#endif  // !_WIN32

