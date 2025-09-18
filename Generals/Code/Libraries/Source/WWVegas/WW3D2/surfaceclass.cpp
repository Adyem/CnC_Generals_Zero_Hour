/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/surfaceclass.cpp                            $*
 *                                                                                             *
 *              Original Author:: Nathaniel Hoffman                                            *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:29p                                               $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SurfaceClass::Clear -- Clears a surface to 0                                              *
 *   SurfaceClass::Copy -- Copies a region from one surface to another of the same format      *
 *   SurfaceClass::FindBBAlpha -- Finds the bounding box of non zero pixels in the region (x0, *
 *   PixelSize -- Helper Function to find the size in bytes of a pixel                         *
 *   SurfaceClass::Is_Transparent_Column -- Tests to see if the column is transparent or not   *
 *   SurfaceClass::Copy -- Copies from a byte array to the surface                             *
 *   SurfaceClass::CreateCopy -- Creates a byte array copy of the surface                      *
 *   SurfaceClass::DrawHLine -- draws a horizontal line                                        *
 *   SurfaceClass::DrawPixel -- draws a pixel                                                  *
 *   SurfaceClass::Copy -- Copies a block of system ram to the surface                         *
 *   SurfaceClass::Hue_Shift -- changes the hue of the surface                                 *
 *   SurfaceClass::Is_Monochrome -- Checks if surface is monochrome or not                     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "surfaceclass.h"
#include "formconv.h"
#include "dx8wrapper.h"
#include "vector2i.h"
#include "colorspace.h"
#include "bound.h"
#include "WWMath/D3DXCompat.h"

#if WW3D_BGFX_AVAILABLE
#include "texture.h"
#include <vector>
#include <cstring>
#endif

#if WW3D_BGFX_AVAILABLE
namespace
{

bgfx::TextureFormat::Enum Convert_WW3D_To_Bgfx_Format(WW3DFormat format)
{
        switch (format)
        {
        case WW3D_FORMAT_A8R8G8B8:
        case WW3D_FORMAT_X8R8G8B8:
                return bgfx::TextureFormat::BGRA8;
        case WW3D_FORMAT_R8G8B8:
                return bgfx::TextureFormat::RGB8;
        case WW3D_FORMAT_R5G6B5:
                return bgfx::TextureFormat::R5G6B5;
        case WW3D_FORMAT_X1R5G5B5:
        case WW3D_FORMAT_A1R5G5B5:
                return bgfx::TextureFormat::BGR5A1;
        case WW3D_FORMAT_A4R4G4B4:
                return bgfx::TextureFormat::BGRA4;
        case WW3D_FORMAT_A8:
                return bgfx::TextureFormat::A8;
        case WW3D_FORMAT_L8:
                return bgfx::TextureFormat::R8;
        case WW3D_FORMAT_A8L8:
                return bgfx::TextureFormat::RG8;
        case WW3D_FORMAT_DXT1:
                return bgfx::TextureFormat::BC1;
        case WW3D_FORMAT_DXT2:
        case WW3D_FORMAT_DXT3:
                return bgfx::TextureFormat::BC2;
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
                return bgfx::TextureFormat::BC3;
        default:
                break;
        }

        return bgfx::TextureFormat::Count;
}

bool Is_Compressed_WW3D_Format(WW3DFormat format)
{
        switch (format)
        {
        case WW3D_FORMAT_DXT1:
        case WW3D_FORMAT_DXT2:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
                return true;
        default:
                break;
        }

        return false;
}

uint32_t Get_Compressed_Block_Size(WW3DFormat format)
{
        switch (format)
        {
        case WW3D_FORMAT_DXT1:
                return 8;
        case WW3D_FORMAT_DXT2:
        case WW3D_FORMAT_DXT3:
        case WW3D_FORMAT_DXT4:
        case WW3D_FORMAT_DXT5:
                return 16;
        default:
                break;
        }

        return 0;
}

void Compute_Bgfx_Row_Info(WW3DFormat format, uint32_t width, uint32_t height, uint32_t& row_bytes, uint32_t& row_count)
{
        row_bytes = 0;
        row_count = 0;

        if (Is_Compressed_WW3D_Format(format))
        {
                const uint32_t block_size = Get_Compressed_Block_Size(format);
                if (block_size == 0)
                {
                        return;
                }

                const uint32_t block_width = (width + 3) / 4;
                const uint32_t block_height = (height + 3) / 4;
                row_bytes = block_width * block_size;
                row_count = block_height;
        }
        else
        {
                switch (format)
                {
                case WW3D_FORMAT_A8R8G8B8:
                case WW3D_FORMAT_X8R8G8B8:
                        row_bytes = width * 4;
                        break;
                case WW3D_FORMAT_R8G8B8:
                        row_bytes = width * 3;
                        break;
                case WW3D_FORMAT_R5G6B5:
                case WW3D_FORMAT_X1R5G5B5:
                case WW3D_FORMAT_A1R5G5B5:
                case WW3D_FORMAT_A4R4G4B4:
                case WW3D_FORMAT_A8L8:
                        row_bytes = width * 2;
                        break;
                case WW3D_FORMAT_A8:
                case WW3D_FORMAT_L8:
                        row_bytes = width;
                        break;
                default:
                        break;
                }

                row_count = height;
        }
}

uint32_t Calculate_Bgfx_Buffer_Size(WW3DFormat format, uint32_t width, uint32_t height)
{
        uint32_t row_bytes = 0;
        uint32_t row_count = 0;
        Compute_Bgfx_Row_Info(format, width, height, row_bytes, row_count);
        return row_bytes * row_count;
}

}
#endif
#if WW3D_BGFX_AVAILABLE
SurfaceClass::BgfxSurfaceInfo::BgfxSurfaceInfo()
        : ownerTexture(NULL)
{
        Reset();
}

void SurfaceClass::BgfxSurfaceInfo::Reset()
{
        if (ownerTexture)
        {
                ownerTexture->Release_Ref();
                ownerTexture = NULL;
        }
        texture.idx = bgfx::kInvalidHandle;
        framebuffer.idx = bgfx::kInvalidHandle;
        ownerMipLevel = 0;
        width = 0;
        height = 0;
        format = WW3D_FORMAT_UNKNOWN;
        renderTarget = false;
        ownsHandles = true;
}

bool SurfaceClass::Has_Bgfx_Surface() const
{
        if (m_bgfxData.ownerTexture)
        {
                return m_bgfxData.ownerTexture->Has_Bgfx_Texture();
        }

        return bgfx::isValid(m_bgfxData.texture);
}

bgfx::TextureHandle SurfaceClass::Get_Bgfx_Texture_Handle() const
{
        if (m_bgfxData.ownerTexture)
        {
                return m_bgfxData.ownerTexture->Get_Bgfx_Texture_Handle();
        }

        return m_bgfxData.texture;
}

bgfx::FrameBufferHandle SurfaceClass::Get_Bgfx_Frame_Buffer_Handle() const
{
        if (m_bgfxData.ownerTexture)
        {
                return m_bgfxData.ownerTexture->Get_Bgfx_Frame_Buffer_Handle();
        }

        return m_bgfxData.framebuffer;
}

void SurfaceClass::Destroy_Bgfx_Surface()
{
        if (!m_bgfxData.ownsHandles)
        {
                m_bgfxData.Reset();
                return;
        }

        if (bgfx::isValid(m_bgfxData.framebuffer))
        {
                bgfx::destroy(m_bgfxData.framebuffer);
        }
        if (bgfx::isValid(m_bgfxData.texture))
        {
                bgfx::destroy(m_bgfxData.texture);
        }
        m_bgfxData.Reset();
}

void SurfaceClass::Create_Bgfx_Surface(uint16_t width, uint16_t height, WW3DFormat format, bool render_target)
{
        if (!DX8Wrapper::Is_Bgfx_Active())
        {
                Destroy_Bgfx_Surface();
                return;
        }

        const bgfx::TextureFormat::Enum bgfx_format = Convert_WW3D_To_Bgfx_Format(format);
        if (bgfx_format == bgfx::TextureFormat::Count)
        {
                Destroy_Bgfx_Surface();
                return;
        }

        Destroy_Bgfx_Surface();

        const uint16_t actual_width = width ? width : 1;
        const uint16_t actual_height = height ? height : 1;
        const uint64_t flags = render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;

        m_bgfxData.texture = bgfx::createTexture2D(actual_width, actual_height, false, 1, bgfx_format, flags, NULL);
        if (!bgfx::isValid(m_bgfxData.texture))
        {
                m_bgfxData.Reset();
                return;
        }

        if (render_target)
        {
                m_bgfxData.framebuffer = bgfx::createFrameBuffer(1, &m_bgfxData.texture, false);
                if (!bgfx::isValid(m_bgfxData.framebuffer))
                {
                        bgfx::destroy(m_bgfxData.texture);
                        m_bgfxData.Reset();
                        return;
                }
        }

        m_bgfxData.width = actual_width;
        m_bgfxData.height = actual_height;
        m_bgfxData.format = format;
        m_bgfxData.renderTarget = render_target;
        m_bgfxData.ownerTexture = NULL;
        m_bgfxData.ownerMipLevel = 0;
        m_bgfxData.ownsHandles = true;
}

void SurfaceClass::Create_Bgfx_Surface_From_D3D(IDirect3DSurface8* surface)
{
        if (!DX8Wrapper::Is_Bgfx_Active())
        {
                Destroy_Bgfx_Surface();
                return;
        }

        if (!surface)
        {
                Destroy_Bgfx_Surface();
                return;
        }

        D3DSURFACE_DESC desc;
        ::ZeroMemory(&desc, sizeof(desc));
        if (FAILED(surface->GetDesc(&desc)))
        {
                Destroy_Bgfx_Surface();
                return;
        }

        const WW3DFormat format = D3DFormat_To_WW3DFormat(desc.Format);
        const bgfx::TextureFormat::Enum bgfx_format = Convert_WW3D_To_Bgfx_Format(format);
        if (bgfx_format == bgfx::TextureFormat::Count)
        {
                Destroy_Bgfx_Surface();
                return;
        }

        D3DLOCKED_RECT locked_rect;
        if (FAILED(surface->LockRect(&locked_rect, NULL, D3DLOCK_READONLY)))
        {
                Destroy_Bgfx_Surface();
                return;
        }

        const uint16_t width = static_cast<uint16_t>(desc.Width ? desc.Width : 1);
        const uint16_t height = static_cast<uint16_t>(desc.Height ? desc.Height : 1);
        const uint32_t total_size = Calculate_Bgfx_Buffer_Size(format, width, height);

        const bgfx::Memory* memory = NULL;
        std::vector<uint8_t> staging;
        if (total_size > 0 && locked_rect.pBits != NULL)
        {
                uint32_t row_bytes = 0;
                uint32_t row_count = 0;
                Compute_Bgfx_Row_Info(format, width, height, row_bytes, row_count);
                if (row_bytes > 0 && row_count > 0)
                {
                        staging.resize(total_size);
                        const uint8_t* src = static_cast<const uint8_t*>(locked_rect.pBits);
                        const uint32_t pitch = static_cast<uint32_t>(locked_rect.Pitch);
                        for (uint32_t row = 0; row < row_count; ++row)
                        {
                                std::memcpy(&staging[row * row_bytes], src + row * pitch, row_bytes);
                        }
                        memory = bgfx::copy(staging.data(), static_cast<uint32_t>(staging.size()));
                }
        }

        surface->UnlockRect();

        Destroy_Bgfx_Surface();

        m_bgfxData.texture = bgfx::createTexture2D(width, height, false, 1, bgfx_format, BGFX_TEXTURE_NONE, memory);
        if (!bgfx::isValid(m_bgfxData.texture))
        {
                m_bgfxData.Reset();
                return;
        }

        m_bgfxData.width = width;
        m_bgfxData.height = height;
        m_bgfxData.format = format;
        m_bgfxData.renderTarget = false;
        m_bgfxData.ownerTexture = NULL;
        m_bgfxData.ownerMipLevel = 0;
        m_bgfxData.ownsHandles = true;
}

void SurfaceClass::Create_Bgfx_Surface_From_Texture(TextureClass* texture, unsigned int level)
{
        if (!DX8Wrapper::Is_Bgfx_Active())
        {
                Destroy_Bgfx_Surface();
                return;
        }

        if (!texture || !texture->Has_Bgfx_Texture())
        {
                Destroy_Bgfx_Surface();
                return;
        }

        SurfaceDescription desc;
        ::ZeroMemory(&desc, sizeof(desc));
        texture->Get_Level_Description(desc, level);

        Destroy_Bgfx_Surface();

        bgfx::TextureHandle borrowed_texture = texture->Get_Bgfx_Texture_Handle();
        bgfx::FrameBufferHandle borrowed_framebuffer = texture->Get_Bgfx_Frame_Buffer_Handle();
        texture->Add_Ref();

        m_bgfxData.texture = borrowed_texture;
        m_bgfxData.framebuffer = borrowed_framebuffer;
        m_bgfxData.ownerTexture = texture;
        m_bgfxData.ownerMipLevel = level;
        m_bgfxData.width = static_cast<uint16_t>(desc.Width ? desc.Width : 1);
        m_bgfxData.height = static_cast<uint16_t>(desc.Height ? desc.Height : 1);
        m_bgfxData.format = desc.Format;
        m_bgfxData.renderTarget = bgfx::isValid(borrowed_framebuffer);
        m_bgfxData.ownsHandles = false;
}
#endif
/***********************************************************************************************
 * PixelSize -- Helper Function to find the size in bytes of a pixel                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/

unsigned int PixelSize(const SurfaceClass::SurfaceDescription &sd)
{
	unsigned int size=0;

	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
		size=4;
		break;
	case WW3D_FORMAT_R8G8B8:
		size=3;
		break;
	case WW3D_FORMAT_R5G6B5:
	case WW3D_FORMAT_X1R5G5B5:
	case WW3D_FORMAT_A1R5G5B5:
	case WW3D_FORMAT_A4R4G4B4:
	case WW3D_FORMAT_A8R3G3B2:
	case WW3D_FORMAT_X4R4G4B4:
	case WW3D_FORMAT_A8P8:	
	case WW3D_FORMAT_A8L8:
		size=2;
		break;
	case WW3D_FORMAT_R3G3B2:
	case WW3D_FORMAT_A8:
	case WW3D_FORMAT_P8:
	case WW3D_FORMAT_L8:
	case WW3D_FORMAT_A4L4:
		size=1;
		break;
	}

	return size;
}

void Convert_Pixel(Vector3 &rgb, const SurfaceClass::SurfaceDescription &sd, const unsigned char * pixel)
{
	const float scale=1/255.0f;
	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
	case WW3D_FORMAT_R8G8B8:
		{
			rgb.X=pixel[2]; // R
			rgb.Y=pixel[1]; // G
			rgb.Z=pixel[0]; // B
		}
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			rgb.X=((tmp&0x0f00)>>4);   // R
			rgb.Y=((tmp&0x00f0));		// G
			rgb.Z=((tmp&0x000f)<<4);	// B			
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];			
			rgb.X=(tmp>>7)&0xf8; // R
			rgb.Y=(tmp>>2)&0xf8; // G
			rgb.Z=(tmp<<3)&0xf8; // B			
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];			
			rgb.X=(tmp>>8)&0xf8;
			rgb.Y=(tmp>>3)&0xfc;
			rgb.Z=(tmp<<3)&0xf8;
		}
		break;

	default:
		// TODO: Implement other pixel formats
		WWASSERT(0);
	}
	rgb*=scale;
}

// Note: This function must never overwrite the original alpha
void Convert_Pixel(unsigned char * pixel,const SurfaceClass::SurfaceDescription &sd, const Vector3 &rgb)
{
	unsigned char r,g,b;
	r=(unsigned char) (rgb.X*255.0f);
	g=(unsigned char) (rgb.Y*255.0f);
	b=(unsigned char) (rgb.Z*255.0f);
	switch (sd.Format)
	{	
	case WW3D_FORMAT_A8R8G8B8:
	case WW3D_FORMAT_X8R8G8B8:
	case WW3D_FORMAT_R8G8B8:
		pixel[0]=b;
		pixel[1]=g;
		pixel[2]=r;
		break;
	case WW3D_FORMAT_A4R4G4B4:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			tmp&=0xF000;
			tmp|=(r&0xF0) << 4;
			tmp|=(g&0xF0);
			tmp|=(b&0xF0) >> 4;			
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	case WW3D_FORMAT_A1R5G5B5:
		{
			unsigned short tmp;
			tmp=*(unsigned short*)&pixel[0];
			tmp&=0x8000;
			tmp|=(r&0xF8) << 7;
			tmp|=(g&0xF8) << 2;
			tmp|=(b&0xF8) >> 3;			
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	case WW3D_FORMAT_R5G6B5:
		{
			unsigned short tmp;			
			tmp=(r&0xf8) << 8;
			tmp|=(g&0xfc) << 3;
			tmp|=(b&0xf8) >> 3;
			*(unsigned short*)&pixel[0]=tmp;
		}
		break;
	default:
		// TODO: Implement other pixel formats
		WWASSERT(0);
	}
}

/*************************************************************************
**                             SurfaceClass
*************************************************************************/
SurfaceClass::SurfaceClass(unsigned width, unsigned height, WW3DFormat format):
        D3DSurface(NULL),
        SurfaceFormat(format)
{
        WWASSERT(width);
        WWASSERT(height);
        D3DSurface = DX8Wrapper::_Create_DX8_Surface(width, height, format, this);
}

SurfaceClass::SurfaceClass(const char *filename):
        D3DSurface(NULL)
{
        D3DSurface = DX8Wrapper::_Create_DX8_Surface(filename, this);
        SurfaceDescription desc;
        Get_Description(desc);
        SurfaceFormat=desc.Format;
}

SurfaceClass::SurfaceClass(IDirect3DSurface8 *d3d_surface)	:
	D3DSurface (NULL)
{
	Attach (d3d_surface);
	SurfaceDescription desc;
	Get_Description(desc);
	SurfaceFormat=desc.Format;
}

SurfaceClass::~SurfaceClass(void)
{
#if WW3D_BGFX_AVAILABLE
        Destroy_Bgfx_Surface();
#endif
        if (D3DSurface) {
                D3DSurface->Release();
                D3DSurface = NULL;
        }
}

void SurfaceClass::Get_Description(SurfaceDescription &surface_desc)
{
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(D3DSurface->GetDesc(&d3d_desc));
	surface_desc.Format = D3DFormat_To_WW3DFormat(d3d_desc.Format);
	surface_desc.Height = d3d_desc.Height;
	surface_desc.Width = d3d_desc.Width;
}

void * SurfaceClass::Lock(int * pitch)
{
	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect, 0, 0));
	*pitch = lock_rect.Pitch;
	return (void *)lock_rect.pBits;
}

void SurfaceClass::Unlock(void)
{
	DX8_ErrorCode(D3DSurface->UnlockRect());
}

/***********************************************************************************************
 * SurfaceClass::Clear -- Clears a surface to 0                                                *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Clear()
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,0));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		memset(mem,0,size*sd.Width);
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies from a byte array to the surface                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/15/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(const unsigned char *other)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,0));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		memcpy(mem,&other[i*sd.Width*size],size*sd.Width);		
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a block of system ram to the surface                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/2/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(Vector2i &min,Vector2i &max, const unsigned char *other)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	rect.left=min.I;
	rect.right=max.I;
	rect.top=min.J;
	rect.bottom=max.J;
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;	
	int dx=max.I-min.I;

	for (i=min.J; i<max.J; i++)
	{
		memcpy(mem,&other[(i*sd.Width+min.I)*size],size*dx);		
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::CreateCopy -- Creates a byte array copy of the surface                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/16/2001  hy : Created.                                                                  *
 *=============================================================================================*/
unsigned char *SurfaceClass::CreateCopy(int *width,int *height,int*size,bool flip)
{
	SurfaceDescription sd;
	Get_Description(sd);

	// size of each pixel in bytes
	unsigned int mysize=PixelSize(sd);

	*width=sd.Width;
	*height=sd.Height;
	*size=mysize;

	unsigned char *other=W3DNEWARRAY unsigned char [sd.Height*sd.Width*mysize];

	D3DLOCKED_RECT lock_rect;	
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,0,D3DLOCK_READONLY));
	unsigned int i;
	unsigned char *mem=(unsigned char *) lock_rect.pBits;

	for (i=0; i<sd.Height; i++)
	{
		if (flip)
		{
			memcpy(&other[(sd.Height-i-1)*sd.Width*mysize],mem,mysize*sd.Width);		
		} else
		{
			memcpy(&other[i*sd.Width*mysize],mem,mysize*sd.Width);		
		}
		mem+=lock_rect.Pitch;
	}
	
	DX8_ErrorCode(D3DSurface->UnlockRect());

	return other;
}


/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a region from one surface to another                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Copy(
	unsigned int dstx, unsigned int dsty,
	unsigned int srcx, unsigned int srcy, 
	unsigned int width, unsigned int height,
	const SurfaceClass *other)
{
	WWASSERT(other);
	WWASSERT(width);
	WWASSERT(height);

	SurfaceDescription sd,osd;
	Get_Description(sd);
	const_cast <SurfaceClass*>(other)->Get_Description(osd);

	RECT src;
	src.left=srcx;
	src.right=srcx+width;
	src.top=srcy;
	src.bottom=srcy+height;

	if (src.right>int(osd.Width)) src.right=int(osd.Width);
	if (src.bottom>int(osd.Height)) src.bottom=int(osd.Height);	

	if (sd.Format==osd.Format && sd.Width==osd.Width && sd.Height==osd.Height)
	{
		POINT dst;
		dst.x=dstx;
		dst.y=dsty;	
		DX8Wrapper::_Copy_DX8_Rects(other->D3DSurface,&src,1,D3DSurface,&dst);
	}
	else
	{
		RECT dest;
		dest.left=dstx;
		dest.right=dstx+width;
		dest.top=dsty;
		dest.bottom=dsty+height;

		if (dest.right>int(sd.Width)) dest.right=int(sd.Width);
		if (dest.bottom>int(sd.Height)) dest.bottom=int(sd.Height);

		DX8_ErrorCode(D3DXLoadSurfaceFromSurface(D3DSurface,NULL,&dest,other->D3DSurface,NULL,&src,D3DX_FILTER_NONE,0));
	}
}

/***********************************************************************************************
 * SurfaceClass::Copy -- Copies a region from one surface to another                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Stretch_Copy(
	unsigned int dstx, unsigned int dsty, unsigned int dstwidth, unsigned int dstheight,
	unsigned int srcx, unsigned int srcy, unsigned int srcwidth, unsigned int srcheight,
	const SurfaceClass *other)
{
	WWASSERT(other);

	SurfaceDescription sd,osd;
	Get_Description(sd);
	const_cast <SurfaceClass*>(other)->Get_Description(osd);

	RECT src;
	src.left=srcx;
	src.right=srcx+srcwidth;
	src.top=srcy;	
	src.bottom=srcy+srcheight;

	RECT dest;
	dest.left=dstx;
	dest.right=dstx+dstwidth;
	dest.top=dsty;
	dest.bottom=dsty+dstheight;

	DX8_ErrorCode(D3DXLoadSurfaceFromSurface(D3DSurface,NULL,&dest,other->D3DSurface,NULL,&src,D3DX_FILTER_TRIANGLE ,0));
}

/***********************************************************************************************
 * SurfaceClass::FindBB -- Finds the bounding box of non zero pixels in the region             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::FindBB(Vector2i *min,Vector2i*max)
{
	SurfaceDescription sd;
	Get_Description(sd);

	WWASSERT(Has_Alpha(sd.Format));

	int alphabits=Alpha_Bits(sd.Format);
	int mask=0;
	switch (alphabits)
	{
	case 1: mask=1;
		break;
	case 4: mask=0xf;
		break;
	case 8: mask=0xff;
		break;
	}

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=max->J;
	rect.top=min->J;
	rect.left=min->I;
	rect.right=max->I;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));

	int x,y;
	unsigned int size=PixelSize(sd);
	Vector2i realmin=*max;
	Vector2i realmax=*min;	
	
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (y = min->J; y < max->J; y++) {
		for (x = min->I; x < max->I; x++) {
			unsigned char *alpha=(unsigned char*) ((unsigned int)lock_rect.pBits+(y-min->J)*lock_rect.Pitch+(x-min->I)*size);
			#pragma MESSAGE("HY - this is not endian safe")
			unsigned char myalpha=alpha[size-1];
			myalpha=(myalpha>>(8-alphabits)) & mask;
			if (myalpha) {
				realmin.I = MIN(realmin.I, x);
				realmax.I = MAX(realmax.I, x);
				realmin.J = MIN(realmin.J, y);
				realmax.J = MAX(realmax.J, y);
			}
		}
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());

	*max=realmax;
	*min=realmin;
}


/***********************************************************************************************
 * SurfaceClass::Is_Transparent_Column -- Tests to see if the column is transparent or not     *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
bool SurfaceClass::Is_Transparent_Column(unsigned int column)
{
	SurfaceDescription sd;
	Get_Description(sd);

	WWASSERT(column<sd.Width);
	WWASSERT(Has_Alpha(sd.Format));

	int alphabits=Alpha_Bits(sd.Format);
	int mask=0;
	switch (alphabits)
	{
	case 1: mask=1;
		break;
	case 4: mask=0xf;
		break;
	case 8: mask=0xff;
		break;
	}

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=sd.Height;
	rect.top=0;
	rect.left=column;
	rect.right=column+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));

	int y;	
	
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (y = 0; y < (int) sd.Height; y++)
	{
		unsigned char *alpha=(unsigned char*) ((unsigned int)lock_rect.pBits+y*lock_rect.Pitch);		
		#pragma MESSAGE("HY - this is not endian safe")
		unsigned char myalpha=alpha[size-1];		
		myalpha=(myalpha>>(8-alphabits)) & mask;		
		if (myalpha) {
			DX8_ErrorCode(D3DSurface->UnlockRect());
			return false;			
		}		
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
	return true;
}

/***********************************************************************************************
 * SurfaceClass::Get_Pixel -- Returns the pixel's RGB valus to the caller							  *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/13/2001  hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Get_Pixel(Vector3 &rgb, int x,int y)
{
	SurfaceDescription sd;
	Get_Description(sd);

	x = min(x,(int)sd.Width - 1);
	y = min(y,(int)sd.Height - 1);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x;
	rect.right=x+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,D3DLOCK_READONLY));	
	Convert_Pixel(rgb,sd,(unsigned char *) lock_rect.pBits);
	DX8_ErrorCode(D3DSurface->UnlockRect());	
}

/***********************************************************************************************
 * SurfaceClass::Attach -- Attaches a surface pointer to the object, releasing the current ptr.*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/2001  pds : Created.                                                                 *
 *=============================================================================================*/
void SurfaceClass::Attach (IDirect3DSurface8 *surface)
{
	Detach ();
	D3DSurface = surface;
	
	//
	//	Lock a reference onto the object
	//
	if (D3DSurface != NULL) {
		D3DSurface->AddRef ();
#if WW3D_BGFX_AVAILABLE
		if (DX8Wrapper::Is_Bgfx_Active())
		{
			Create_Bgfx_Surface_From_D3D(D3DSurface);
		}
#endif
	}

	return ;
}


/***********************************************************************************************
 * SurfaceClass::Detach -- Releases the reference on the internal surface ptr, and NULLs it.	 .*
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/27/2001  pds : Created.                                                                 *
 *=============================================================================================*/
void SurfaceClass::Detach (void)
{
	//
	//	Release the hold we have on the D3D object
	//
#if WW3D_BGFX_AVAILABLE
	Destroy_Bgfx_Surface();
#endif
	if (D3DSurface != NULL) {
		D3DSurface->Release ();
	}

	D3DSurface = NULL;
	return ;
}


/***********************************************************************************************
 * SurfaceClass::DrawPixel -- draws a pixel                                                    *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void SurfaceClass::DrawPixel(const unsigned int x,const unsigned int y, unsigned int color)
{
	SurfaceDescription sd;
	Get_Description(sd);

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x;
	rect.right=x+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	unsigned char *cptr=(unsigned char*)lock_rect.pBits;
	unsigned short *sptr=(unsigned short*)lock_rect.pBits;
	unsigned int *lptr=(unsigned int*)lock_rect.pBits;

	switch (size)
	{
	case 1:
		*cptr=(unsigned char) (color & 0xFF);
		break;
	case 2:
		*sptr=(unsigned short) (color & 0xFFFF);
		break;
	case 4:
		*lptr=color;
		break;
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
}

/***********************************************************************************************
 * SurfaceClass::DrawHLine -- draws a horizontal line                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/9/2001   hy : Created.                                                                  *
 *   4/9/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::DrawHLine(const unsigned int y,const unsigned int x1, const unsigned int x2, unsigned int color)
{ 
	SurfaceDescription sd;
	Get_Description(sd);

	unsigned int size=PixelSize(sd);

	D3DLOCKED_RECT lock_rect;
	::ZeroMemory(&lock_rect, sizeof(D3DLOCKED_RECT));
	RECT rect;
	::ZeroMemory(&rect, sizeof(RECT));

	rect.bottom=y+1;
	rect.top=y;
	rect.left=x1;
	rect.right=x2+1;

	DX8_ErrorCode(D3DSurface->LockRect(&lock_rect,&rect,0));
	unsigned char *cptr=(unsigned char*)lock_rect.pBits;
	unsigned short *sptr=(unsigned short*)lock_rect.pBits;
	unsigned int *lptr=(unsigned int*)lock_rect.pBits;

	unsigned int x;
	// the assumption here is that whenever a pixel has alpha it's in the MSB
	for (x=x1; x<=x2; x++)
	{		
		switch (size)
		{
		case 1:
			*cptr++=(unsigned char) (color & 0xFF);
			break;
		case 2:
			*sptr++=(unsigned short) (color & 0xFFFF);
			break;
		case 4:
			*lptr++=color;
			break;
		}
	}

	DX8_ErrorCode(D3DSurface->UnlockRect());
}


/***********************************************************************************************
 * SurfaceClass::Is_Monochrome -- Checks if surface is monochrome or not                       *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/5/2001   hy : Created.                                                                  *
 *=============================================================================================*/
bool SurfaceClass::Is_Monochrome(void)
{
	unsigned int x,y;
	SurfaceDescription sd;
	Get_Description(sd);

	switch (sd.Format)
	{
		case WW3D_FORMAT_A8L8:	
		case WW3D_FORMAT_A8:		
		case WW3D_FORMAT_L8:
		case WW3D_FORMAT_A4L4:
			return true;
		break;
	}

	int pitch,size;

	size=PixelSize(sd);
	unsigned char *bits=(unsigned char*) Lock(&pitch);

	Vector3 rgb;
	bool mono=true;

	for (y=0; y<sd.Height; y++)
	{
		for (x=0; x<sd.Width; x++)
		{
			Convert_Pixel(rgb,sd,&bits[x*size]);
			mono&=(rgb.X==rgb.Y);
			mono&=(rgb.X==rgb.Z);
			mono&=(rgb.Z==rgb.Y);
			if (!mono)
			{
				Unlock();
				return false;
			}
		}
		bits+=pitch;
	}

	Unlock();

	return true;
}

/***********************************************************************************************
 * SurfaceClass::Hue_Shift -- changes the hue of the surface                                   *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/3/2001   hy : Created.                                                                  *
 *=============================================================================================*/
void SurfaceClass::Hue_Shift(const Vector3 &hsv_shift)
{
	unsigned int x,y;
	SurfaceDescription sd;
	Get_Description(sd);
	int pitch,size;

	size=PixelSize(sd);
	unsigned char *bits=(unsigned char*) Lock(&pitch);

	Vector3 rgb;

	for (y=0; y<sd.Height; y++)
	{
		for (x=0; x<sd.Width; x++)
		{
			Convert_Pixel(rgb,sd,&bits[x*size]);
			Recolor(rgb,hsv_shift);
			rgb.X=Bound(rgb.X,0.0f,1.0f);
			rgb.Y=Bound(rgb.Y,0.0f,1.0f);
			rgb.Z=Bound(rgb.Z,0.0f,1.0f);
			Convert_Pixel(&bits[x*size],sd,rgb);
		}
		bits+=pitch;
	}

	Unlock();
}