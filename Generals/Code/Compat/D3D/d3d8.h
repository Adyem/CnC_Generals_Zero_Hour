#pragma once

// Minimal Direct3D 8 compatibility declarations for non-Windows builds.
// These definitions are sufficient to satisfy the subset of the legacy
// Command & Conquer Generals headers that refer to Direct3D interfaces.
// They intentionally do not provide any functionality beyond the type
// declarations required for compilation.

#ifndef _WIN32

#include <cstdint>

#include "d3d8types.h"
#include "../Windows/windows_compat.h"

using HRESULT = long;

#ifndef DWORD
using DWORD = std::uint32_t;
#endif

#ifndef D3DCOLOR
using D3DCOLOR = std::uint32_t;
#endif

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

struct IDirect3D8;
struct IDirect3DDevice8;

struct IDirect3DResource8
{
    virtual ~IDirect3DResource8() = default;
    virtual ULONG AddRef() { return 1; }
    virtual ULONG Release() { return 1; }
};

struct IDirect3DBaseTexture8 : public IDirect3DResource8
{
};

struct IDirect3DTexture8 : public IDirect3DBaseTexture8
{
};

struct IDirect3DVolumeTexture8 : public IDirect3DBaseTexture8
{
};

struct IDirect3DCubeTexture8 : public IDirect3DBaseTexture8
{
};

struct IDirect3DSurface8 : public IDirect3DResource8
{
};

struct IDirect3DVertexBuffer8 : public IDirect3DResource8
{
};

struct IDirect3DIndexBuffer8 : public IDirect3DResource8
{
};

struct IDirect3DStateBlock8 : public IDirect3DResource8
{
};

struct IDirect3DSwapChain8 : public IDirect3DResource8
{
};

struct IDirect3DDevice8 : public IDirect3DResource8
{
    virtual HRESULT TestCooperativeLevel() { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT Reset(D3DPRESENT_PARAMETERS*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetDeviceCaps(D3DCAPS8* caps)
    {
        if (caps)
        {
            *caps = {};
        }
        return D3DERR_NOTAVAILABLE;
    }
    virtual HRESULT GetDisplayMode(D3DDISPLAYMODE* mode)
    {
        if (mode)
        {
            *mode = {};
        }
        return D3DERR_NOTAVAILABLE;
    }
    virtual HRESULT GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetFrontBuffer(IDirect3DSurface8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetDepthStencilSurface(IDirect3DSurface8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetRenderTarget(IDirect3DSurface8*, IDirect3DSurface8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetRenderTarget(IDirect3DSurface8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetTransform(D3DTRANSFORMSTATETYPE, D3DMATRIX*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetViewport(const D3DVIEWPORT8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetMaterial(const D3DMATERIAL8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetLight(DWORD, const D3DLIGHT8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT LightEnable(DWORD, BOOL) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetRenderState(D3DRENDERSTATETYPE, DWORD) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetClipPlane(DWORD, const float*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetStreamSource(UINT, IDirect3DVertexBuffer8*, UINT) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetIndices(IDirect3DIndexBuffer8*, UINT) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetTexture(DWORD, IDirect3DBaseTexture8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT SetVertexShader(DWORD) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CreateImageSurface(UINT, UINT, D3DFORMAT, IDirect3DSurface8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT UpdateTexture(IDirect3DBaseTexture8*, IDirect3DBaseTexture8*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT Clear(DWORD, const D3DRECT*, DWORD, D3DCOLOR, float, DWORD) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT BeginScene() { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT EndScene() { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CopyRects(IDirect3DSurface8*, const RECT*, UINT, IDirect3DSurface8*, const POINT*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CreateVertexBuffer(UINT, DWORD, DWORD, DWORD, IDirect3DVertexBuffer8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CreateIndexBuffer(UINT, DWORD, D3DFORMAT, DWORD, IDirect3DIndexBuffer8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS*, IDirect3DSwapChain8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT ValidateDevice(DWORD*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT ResourceManagerDiscardBytes(DWORD) { return D3DERR_NOTAVAILABLE; }
};

struct IDirect3D8 : public IDirect3DResource8
{
    virtual UINT GetAdapterCount() { return 0; }
    virtual HRESULT GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER8*) { return D3DERR_NOTAVAILABLE; }
    virtual UINT GetAdapterModeCount(UINT) { return 0; }
    virtual HRESULT EnumAdapterModes(UINT, UINT, D3DDISPLAYMODE*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE*) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**) { return D3DERR_NOTAVAILABLE; }
    virtual HRESULT GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS8*) { return D3DERR_NOTAVAILABLE; }
};

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

inline IDirect3D8* Direct3DCreate8(unsigned int)
{
    return nullptr;
}



#endif  // !_WIN32

