#pragma once

#include "d3d8.h"
#include "WWMath/D3DXCompat.h"

#include <cstddef>
#include <cstdio>

#ifndef D3DX_DEFAULT
#define D3DX_DEFAULT 0xFFFFFFFFu
#endif

#ifndef D3D_OK
#define D3D_OK S_OK
#endif

#ifndef E_INVALIDARG
#define E_INVALIDARG static_cast<HRESULT>(0x80070057L)
#endif

#ifndef D3DERR_INVALIDCALL
#define D3DERR_INVALIDCALL E_INVALIDARG
#endif

struct D3DXIMAGE_INFO
{
        UINT Width = 0;
        UINT Height = 0;
        UINT Depth = 0;
        UINT MipLevels = 0;
        D3DFORMAT Format = D3DFMT_UNKNOWN;
        D3DRESOURCETYPE ResourceType = D3DRTYPE_TEXTURE;
        D3DFORMAT ImageFileFormat = D3DFMT_UNKNOWN;
};

inline HRESULT D3DXGetErrorStringA(HRESULT hr, char *buffer, UINT buffer_length)
{
        if (buffer == nullptr || buffer_length == 0)
        {
                return D3DERR_INVALIDCALL;
        }

        const int written = std::snprintf(buffer, buffer_length, "D3DX error 0x%08lX", static_cast<unsigned long>(hr));
        if (written < 0)
        {
                buffer[0] = '\0';
                return D3DERR_INVALIDCALL;
        }

        return D3D_OK;
}

inline HRESULT D3DXCreateTexture(
        LPDIRECT3DDEVICE8 /*device*/,
        UINT /*width*/,
        UINT /*height*/,
        UINT /*mip_levels*/,
        DWORD /*usage*/,
        D3DFORMAT /*format*/,
        D3DPOOL /*pool*/,
        LPDIRECT3DTEXTURE8 *texture)
{
        if (texture != nullptr)
        {
                *texture = nullptr;
        }

        return D3DERR_INVALIDCALL;
}

inline HRESULT D3DXCreateTextureFromFileExA(
        LPDIRECT3DDEVICE8 /*device*/,
        const char * /*filename*/,
        UINT /*width*/,
        UINT /*height*/,
        UINT /*mip_levels*/,
        DWORD /*usage*/,
        D3DFORMAT /*format*/,
        D3DPOOL /*pool*/,
        DWORD /*filter*/,
        DWORD /*mip_filter*/,
        D3DCOLOR /*color_key*/,
        D3DXIMAGE_INFO *src_info,
        PALETTEENTRY * /*palette*/,
        LPDIRECT3DTEXTURE8 *texture)
{
        if (src_info != nullptr)
        {
                *src_info = D3DXIMAGE_INFO{};
        }

        if (texture != nullptr)
        {
                *texture = nullptr;
        }

        return D3DERR_INVALIDCALL;
}
