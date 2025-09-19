#pragma once

// Minimal Direct3D 8 compatibility declarations for non-Windows builds.
// These definitions are sufficient to satisfy the subset of the legacy
// Command & Conquer Generals headers that refer to Direct3D interfaces.
// They intentionally do not provide any functionality beyond the type
// declarations required for compilation.

#ifndef _WIN32

#include <cstdint>

using HRESULT = long;
using DWORD = std::uint32_t;
using D3DCOLOR = std::uint32_t;

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

#endif  // !_WIN32

