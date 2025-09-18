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
 *                     $Archive:: /Commando/Code/ww3d2/texture.cpp                            $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 7:06p                                               $*
 *                                                                                             *
 *                    $Revision:: 65                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FileListTextureClass::Load_Frame_Surface -- Load source texture                           * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "texture.h"

#include <d3d8.h>
#include <stdio.h>
#include <D3dx8core.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>
#include "dx8wrapper.h"
#include "targa.h"
#include <nstrdup.h>
#include "w3d_file.h"
#include "assetmgr.h"
#include "formconv.h"
#include "textureloader.h"
#include "missingtexture.h"
#include "ffactory.h"
#include "dx8caps.h"
#include "dx8texman.h"
#include "meshmatdesc.h"

/*
** Definitions of static members:
*/

static unsigned unused_texture_id;

unsigned _MinTextureFilters[TextureClass::FILTER_TYPE_COUNT];
unsigned _MagTextureFilters[TextureClass::FILTER_TYPE_COUNT];
unsigned _MipMapFilters[TextureClass::FILTER_TYPE_COUNT];

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
	if (Is_Compressed_WW3D_Format(format))
	{
		const uint32_t block_size = Get_Compressed_Block_Size(format);
		const uint32_t block_width = std::max(1u, width / 4);
		const uint32_t block_height = std::max(1u, height / 4);
		row_bytes = block_width * block_size;
		row_count = block_height;
		return;
	}

	row_bytes = width * Get_Bytes_Per_Pixel(format);
	row_count = height;
}

uint32_t Calculate_Bgfx_Buffer_Size(WW3DFormat format, uint16_t width, uint16_t height, uint8_t mip_count)
{
        uint32_t total_size = 0;
        uint32_t current_width = width ? width : 1;
        uint32_t current_height = height ? height : 1;

	for (uint8_t level = 0; level < mip_count; ++level)
	{
		uint32_t row_bytes = 0;
		uint32_t row_count = 0;
		Compute_Bgfx_Row_Info(format, current_width, current_height, row_bytes, row_count);
		total_size += row_bytes * row_count;

		if (current_width > 1)
		{
			current_width >>= 1;
		}
		if (current_height > 1)
		{
			current_height >>= 1;
		}
	}

	return total_size;
}

} // namespace
#endif

// ----------------------------------------------------------------------------

#if WW3D_BGFX_AVAILABLE
uint8_t TextureClass::Calculate_Max_Bgfx_Mip_Count(uint16_t width, uint16_t height)
{
        uint32_t current_width = (width != 0) ? width : 1;
        uint32_t current_height = (height != 0) ? height : 1;
        uint8_t count = 1;

        while (current_width > 1 || current_height > 1)
        {
                if (current_width > 1)
                {
                        current_width >>= 1;
                }
                if (current_height > 1)
                {
                        current_height >>= 1;
                }

                ++count;
        }

        return count;
}

uint8_t TextureClass::Calculate_Bgfx_Mip_Count(uint16_t width, uint16_t height, MipCountType mip_level_count)
{
        const uint8_t max_mips = Calculate_Max_Bgfx_Mip_Count(width, height);
        uint8_t requested = max_mips;

        switch (mip_level_count)
        {
        case MIP_LEVELS_ALL:
                requested = max_mips;
                break;
        case MIP_LEVELS_1:
                requested = 1;
                break;
        case MIP_LEVELS_2:
                requested = 2;
                break;
        case MIP_LEVELS_3:
                requested = 3;
                break;
        case MIP_LEVELS_4:
                requested = 4;
                break;
        case MIP_LEVELS_5:
                requested = 5;
                break;
        case MIP_LEVELS_6:
                requested = 6;
                break;
        case MIP_LEVELS_7:
                requested = 7;
                break;
        case MIP_LEVELS_8:
                requested = 8;
                break;
        case MIP_LEVELS_10:
                requested = 10;
                break;
        case MIP_LEVELS_11:
                requested = 11;
                break;
        case MIP_LEVELS_12:
                requested = 12;
                break;
        default:
                requested = max_mips;
                break;
        }

        if (requested > max_mips)
        {
                requested = max_mips;
        }

        return (requested != 0) ? requested : 1;
}

TextureClass::BgfxTextureInfo::BgfxTextureInfo()
{
        Reset();
}

void TextureClass::BgfxTextureInfo::Reset()
{
        texture = BGFX_INVALID_HANDLE;
        framebuffer = BGFX_INVALID_HANDLE;
        width = 0;
        height = 0;
        mipCount = 0;
        format = WW3D_FORMAT_UNKNOWN;
        renderTarget = false;
        flags = BGFX_TEXTURE_NONE;
}
#endif

// ----------------------------------------------------------------------------

static int Calculate_Texture_Memory_Usage(const TextureClass* texture,int red_factor=0)
{
        // Set performance statistics

        int size=0;
        IDirect3DTexture8* d3d_texture=const_cast<TextureClass*>(texture)->Peek_DX8_Texture();
#if WW3D_BGFX_AVAILABLE
        if (d3d_texture == NULL && DX8Wrapper::Is_Bgfx_Active() && texture->Has_Bgfx_Texture())
        {
                const TextureClass::BgfxTextureInfo& info = texture->m_bgfxData;
                const WW3DFormat bgfx_format = info.format != WW3D_FORMAT_UNKNOWN ? info.format : texture->Get_Texture_Format();
                if (info.width != 0 && info.height != 0 && info.mipCount != 0 && bgfx_format != WW3D_FORMAT_UNKNOWN)
                {
                        size = static_cast<int>(Calculate_Bgfx_Buffer_Size(bgfx_format, info.width, info.height, info.mipCount));
                }
                return size;
        }
#endif
        if (!d3d_texture) return 0;
        for (unsigned i=red_factor;i<d3d_texture->GetLevelCount();++i) {
                D3DSURFACE_DESC desc;
                DX8_ErrorCode(d3d_texture->GetLevelDesc(i,&desc));
                size+=desc.Size;
        }
	return size;
}

#if WW3D_BGFX_AVAILABLE
bool TextureClass::Has_Bgfx_Texture() const
{
	return bgfx::isValid(m_bgfxData.texture);
}

bgfx::TextureHandle TextureClass::Get_Bgfx_Texture_Handle() const
{
	return m_bgfxData.texture;
}

bgfx::FrameBufferHandle TextureClass::Get_Bgfx_Frame_Buffer_Handle() const
{
	return m_bgfxData.framebuffer;
}

void TextureClass::Destroy_Bgfx_Resources()
{
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

void TextureClass::Upload_Bgfx_Texture(uint16_t width, uint16_t height, WW3DFormat format, uint8_t mip_count, bool render_target, const uint8_t* const* data, const uint32_t* pitches)
{
	if (!DX8Wrapper::Is_Bgfx_Active())
	{
		return;
	}

	const bgfx::TextureFormat::Enum bgfx_format = Convert_WW3D_To_Bgfx_Format(format);
	if (bgfx_format == bgfx::TextureFormat::Count)
	{
		Destroy_Bgfx_Resources();
		return;
	}

	Destroy_Bgfx_Resources();

	const bgfx::Memory* memory = NULL;
	std::vector<uint8_t> staging;

	if (data != NULL && pitches != NULL && mip_count > 0)
	{
		const uint32_t total_size = Calculate_Bgfx_Buffer_Size(format, width ? width : 1, height ? height : 1, mip_count);
		if (total_size > 0)
		{
			staging.resize(total_size);
			uint32_t offset = 0;
			uint32_t current_width = width ? width : 1;
			uint32_t current_height = height ? height : 1;

			for (uint8_t level = 0; level < mip_count; ++level)
			{
				const uint8_t* src = data[level];
				if (!src)
				{
					staging.clear();
					break;
				}

				uint32_t row_bytes = 0;
				uint32_t row_count = 0;
				Compute_Bgfx_Row_Info(format, current_width, current_height, row_bytes, row_count);
				if (row_bytes == 0 || row_count == 0)
				{
					staging.clear();
					break;
				}

				const uint32_t pitch = pitches[level];
				for (uint32_t row = 0; row < row_count; ++row)
				{
					std::memcpy(&staging[offset + row * row_bytes], src + row * pitch, row_bytes);
				}

				offset += row_bytes * row_count;

				if (current_width > 1)
				{
					current_width >>= 1;
				}
				if (current_height > 1)
				{
					current_height >>= 1;
				}
			}

			if (!staging.empty())
			{
				memory = bgfx::copy(staging.data(), static_cast<uint32_t>(staging.size()));
			}
		}
	}

        const uint64_t flags = render_target ? BGFX_TEXTURE_RT : BGFX_TEXTURE_NONE;
        const uint16_t tex_width = width ? width : 1;
        const uint16_t tex_height = height ? height : 1;
        const uint8_t actual_mips = mip_count > 0 ? mip_count : 1;

	m_bgfxData.texture = bgfx::createTexture2D(tex_width, tex_height, actual_mips > 1, 1, bgfx_format, flags, memory);
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

        m_bgfxData.width = tex_width;
        m_bgfxData.height = tex_height;
        m_bgfxData.mipCount = actual_mips;
        m_bgfxData.format = format;
        m_bgfxData.renderTarget = render_target;
        m_bgfxData.flags = flags;
}

void TextureClass::Create_Bgfx_Texture_From_D3D(IDirect3DTexture8* texture, WW3DFormat format, bool render_target)
{
	if (!DX8Wrapper::Is_Bgfx_Active())
	{
		return;
	}

	if (!texture)
	{
		Destroy_Bgfx_Resources();
		return;
	}

	D3DSURFACE_DESC desc;
	::ZeroMemory(&desc, sizeof(desc));
	if (FAILED(texture->GetLevelDesc(0, &desc)))
	{
		return;
	}

	const uint16_t width = static_cast<uint16_t>(desc.Width);
	const uint16_t height = static_cast<uint16_t>(desc.Height);
	const uint8_t mip_count = static_cast<uint8_t>(texture->GetLevelCount());

	std::vector<const uint8_t*> data;
	std::vector<uint32_t> pitches;
	std::vector<D3DLOCKED_RECT> locked_rects;
	bool copy_data = true;

	if (mip_count > 0)
	{
				data.resize(mip_count);
				pitches.resize(mip_count);
				locked_rects.resize(mip_count);

				for (uint8_t level = 0; level < mip_count; ++level)
				{
					HRESULT hr = texture->LockRect(level, &locked_rects[level], NULL, D3DLOCK_READONLY);
					if (FAILED(hr))
					{
						copy_data = false;
						for (uint8_t unlock = 0; unlock < level; ++unlock)
						{
							texture->UnlockRect(unlock);
						}
						data.clear();
						pitches.clear();
						locked_rects.clear();
						break;
					}

					data[level] = static_cast<const uint8_t*>(locked_rects[level].pBits);
					pitches[level] = static_cast<uint32_t>(locked_rects[level].Pitch);
				}
	}
	else
	{
		copy_data = false;
	}

	if (copy_data && !data.empty())
	{
		Upload_Bgfx_Texture(width, height, format, mip_count, render_target, data.data(), pitches.data());
		for (uint8_t level = 0; level < mip_count; ++level)
		{
			texture->UnlockRect(level);
		}
	}
	else
	{
		Upload_Bgfx_Texture(width, height, format, mip_count ? mip_count : 1, render_target, NULL, NULL);
	}
}

void TextureClass::Create_Bgfx_Texture_From_Locked_Task(TextureLoadTaskClass* task)
{
	if (!DX8Wrapper::Is_Bgfx_Active() || !task || task->Has_Failed())
	{
		return;
	}

	const unsigned mip_count = task->Get_Mip_Level_Count();
	if (mip_count == 0)
	{
		return;
	}

	for (unsigned i = 0; i < mip_count; ++i)
	{
		if (task->LockedSurfacePtr[i] == NULL)
		{
			return;
		}
	}

	const uint16_t width = static_cast<uint16_t>(task->Get_Width());
	const uint16_t height = static_cast<uint16_t>(task->Get_Height());
	const WW3DFormat format = task->Get_Format();

	std::vector<const uint8_t*> data(mip_count);
	std::vector<uint32_t> pitches(mip_count);
	for (unsigned i = 0; i < mip_count; ++i)
	{
			data[i] = task->LockedSurfacePtr[i];
			pitches[i] = task->LockedSurfacePitch[i];
	}

	Upload_Bgfx_Texture(width, height, format, static_cast<uint8_t>(mip_count), false, data.data(), pitches.data());
}
#endif

/*************************************************************************
**                             TextureClass
*************************************************************************/

TextureClass::TextureClass(unsigned width, unsigned height, WW3DFormat format, MipCountType mip_level_count, PoolType pool,bool rendertarget)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(pool),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(true),
	Name(""),
	TextureFormat(format),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL)
{
	switch (format) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}
	D3DPOOL d3dpool=(D3DPOOL) 0;
	switch(pool)
	{
	case POOL_DEFAULT:
		d3dpool=D3DPOOL_DEFAULT;		
		break;
	case POOL_MANAGED:
		d3dpool=D3DPOOL_MANAGED;
		break;
	case POOL_SYSTEMMEM:
		d3dpool=D3DPOOL_SYSTEMMEM;
		break;
	default:
		WWASSERT(0);		
	}
        D3DTexture = DX8Wrapper::_Create_DX8_Texture(width, height, format, mip_level_count,d3dpool,rendertarget,this);
	if (pool==POOL_DEFAULT)
	{
		Dirty=true;
		DX8TextureTrackerClass *track=W3DNEW
		DX8TextureTrackerClass(width, height, format, mip_level_count,rendertarget,
		this);
		DX8TextureManagerClass::Add(track);
	}
        LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(
	const char *name, 
	const char *full_path, 
	MipCountType mip_level_count,
	WW3DFormat texture_format,
	bool allow_compression)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(false),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	IsProcedural(false),
	TextureFormat(texture_format),
	IsCompressionAllowed(allow_compression),
	TextureLoadTask(NULL)
{
	switch (TextureFormat) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	case WW3D_FORMAT_U8V8:		// Bumpmap
	case WW3D_FORMAT_L6V5U5:	// Bumpmap
	case WW3D_FORMAT_X8L8V8U8:	// Bumpmap
		// If requesting bumpmap format that isn't available we'll just return the surface in whatever color
		// format the texture file is in. (This is illegal case, the format support should always be queried
		// before creating a bump texture!)
		if (!DX8Caps::Support_Texture_Format(TextureFormat)) {
			TextureFormat=WW3D_FORMAT_UNKNOWN;
		}
		// If bump format is valid, make sure compression is not allowed so that we don't even attempt to load
		// from a compressed file (quality isn't good enough for bump map). Also disable mipmapping.
		else {
			IsCompressionAllowed=false;
			MipLevelCount=MIP_LEVELS_1;
			MipMapFilter=FILTER_TYPE_NONE;
		}
		break;

	default:
		break;
	}

	WWASSERT_PRINT(name && name[0], "TextureClass CTor: NULL or empty texture name\n");
	int len=strlen(name);
	for (int i=0;i<len;++i) {
		if (name[i]=='+') {
			IsLightmap=true;

			// Set bilinear filtering for lightmaps (they are very stretched and
			// low detail so we don't care for anisotropic or trilinear filtering...)
			TextureMinFilter=FILTER_TYPE_FAST;
			TextureMagFilter=FILTER_TYPE_FAST;
			if (mip_level_count!=MIP_LEVELS_1) MipMapFilter=FILTER_TYPE_FAST;
			break;
		}
	}
	Set_Texture_Name(name);
	Set_Full_Path(full_path);
	WWASSERT(name[0]!='\0');
	if (!WW3D::Is_Texturing_Enabled()) {
		Initialized=true;
		D3DTexture=0;
	}
	else if (WW3D::Get_Texture_Thumbnail_Mode()==WW3D::TEXTURE_THUMBNAIL_MODE_OFF || mip_level_count==MIP_LEVELS_1)
	{
		Initialized=true;
		D3DTexture=0;
		TextureLoader::Request_High_Priority_Loading(this,mip_level_count);
	}
	else {
		Load_Locked_Surface();
		TextureFormat=texture_format;	// Locked surface may be in a wrong format
	}
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(SurfaceClass *surface, MipCountType mip_level_count)
	:
	D3DTexture(NULL),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((mip_level_count!=MIP_LEVELS_1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount(mip_level_count),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	TextureFormat(surface->Get_Surface_Format()),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL)
{
	SurfaceClass::SurfaceDescription sd;
	surface->Get_Description(sd);
	switch (sd.Format) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}

        D3DTexture = DX8Wrapper::_Create_DX8_Texture(surface->Peek_D3D_Surface(), mip_level_count, this);
        LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::TextureClass(IDirect3DTexture8* d3d_texture)
	:
	D3DTexture(d3d_texture),
	texture_id(unused_texture_id++),
	Initialized(true),
	TextureMinFilter(FILTER_TYPE_DEFAULT),
	TextureMagFilter(FILTER_TYPE_DEFAULT),
	MipMapFilter((d3d_texture->GetLevelCount()!=1) ? FILTER_TYPE_DEFAULT : FILTER_TYPE_NONE),
	UAddressMode(TEXTURE_ADDRESS_REPEAT),
	VAddressMode(TEXTURE_ADDRESS_REPEAT),
	MipLevelCount((MipCountType)d3d_texture->GetLevelCount()),
	Pool(POOL_MANAGED),
	Dirty(false),
	IsLightmap(false),
	Name(""),
	IsProcedural(true),
	IsCompressionAllowed(false),
	TextureLoadTask(NULL)
{
	D3DTexture->AddRef();
	IDirect3DSurface8* surface;
	DX8_ErrorCode(D3DTexture->GetSurfaceLevel(0,&surface));
	D3DSURFACE_DESC d3d_desc;
	::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
	DX8_ErrorCode(surface->GetDesc(&d3d_desc));
	TextureFormat=D3DFormat_To_WW3DFormat(d3d_desc.Format);
	switch (TextureFormat) {
	case WW3D_FORMAT_DXT1:
	case WW3D_FORMAT_DXT2:
	case WW3D_FORMAT_DXT3:
	case WW3D_FORMAT_DXT4:
	case WW3D_FORMAT_DXT5:
		IsCompressionAllowed=true;
		break;
	default:
		break;
	}

#if WW3D_BGFX_AVAILABLE
	if (DX8Wrapper::Is_Bgfx_Active() && D3DTexture)
	{
		Create_Bgfx_Texture_From_D3D(D3DTexture, TextureFormat, false);
	}
#endif
	LastAccessed=WW3D::Get_Sync_Time();
}

// ----------------------------------------------------------------------------

TextureClass::~TextureClass(void)
{
	TextureLoadTaskClass::Release_Instance(TextureLoadTask);
	TextureLoadTask=NULL;

	if (!Initialized) {
		WWDEBUG_SAY(("Warning: Texture %s was loaded but never used\n",Get_Texture_Name()));
	}

#if WW3D_BGFX_AVAILABLE
	Destroy_Bgfx_Resources();
#endif
	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}
	DX8TextureManagerClass::Remove(this);
}

// ----------------------------------------------------------------------------

void TextureClass::Init()
{
	// If the texture has already been initialised we should exit now
	if (Initialized) return;

	TextureLoader::Add_Load_Task(this);
	LastAccessed=WW3D::Get_Sync_Time();
}

void TextureClass::Invalidate()
{
	// Don't invalidate procedural textures
	if (IsProcedural) return;

	// Don't invalidate missing texture
	if (Is_Missing_Texture()) return;

	if (D3DTexture) {
		D3DTexture->Release();
		D3DTexture = NULL;
	}
#if WW3D_BGFX_AVAILABLE
	Destroy_Bgfx_Resources();
#endif

	if (!WW3D::Is_Texturing_Enabled()) {
		Initialized=true;
		D3DTexture=0;
	}
	else if (WW3D::Get_Texture_Thumbnail_Mode()==WW3D::TEXTURE_THUMBNAIL_MODE_OFF || MipLevelCount==MIP_LEVELS_1)
	{
		Initialized=true;
		D3DTexture=0;
		TextureLoader::Request_High_Priority_Loading(this,MipLevelCount);
	}
	else {
		Initialized=false;
		Load_Locked_Surface();
	}
}

// ----------------------------------------------------------------------------

void TextureClass::Load_Locked_Surface()
{
	if (D3DTexture) D3DTexture->Release();
	D3DTexture=0;
#if WW3D_BGFX_AVAILABLE
	Destroy_Bgfx_Resources();
#endif
	TextureLoader::Request_Thumbnail(this);
	Initialized=false;
}

// ----------------------------------------------------------------------------

bool TextureClass::Is_Missing_Texture()
{
	bool flag = false;
	IDirect3DTexture8 *missing_texture = MissingTexture::_Get_Missing_Texture();
	
	if(D3DTexture == missing_texture)
		flag = true;

	if(missing_texture)
		missing_texture->Release();

	return flag;
}

// ----------------------------------------------------------------------------

void TextureClass::Set_Texture_Name(const char * name)
{
	Name=name;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Mip_Level_Count(void)
{
        if (D3DTexture) {
                return D3DTexture->GetLevelCount();
        }
#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                return (m_bgfxData.mipCount != 0) ? m_bgfxData.mipCount : 1;
        }
#endif

        WWASSERT_PRINT(0, "Get_Mip_Level_Count: No backing texture available!\n");
        return 0;
}

// ----------------------------------------------------------------------------

void TextureClass::Get_Level_Description(SurfaceClass::SurfaceDescription &surface_desc, unsigned int level)
{
        if (D3DTexture) {
                D3DSURFACE_DESC d3d_surf_desc;
                DX8_ErrorCode(D3DTexture->GetLevelDesc(level, &d3d_surf_desc));
                surface_desc.Format = D3DFormat_To_WW3DFormat(d3d_surf_desc.Format);
                surface_desc.Height = d3d_surf_desc.Height;
                surface_desc.Width = d3d_surf_desc.Width;
                return;
        }
#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                const uint32_t mip_count = Get_Mip_Level_Count();
                if (level >= mip_count)
                {
                        WWASSERT_PRINT(0, "Get_Level_Description: Level out of range for bgfx texture!\n");
                        level = mip_count ? (mip_count - 1) : 0;
                }

                const uint32_t level_width = std::max(1u, static_cast<unsigned>(m_bgfxData.width) >> level);
                const uint32_t level_height = std::max(1u, static_cast<unsigned>(m_bgfxData.height) >> level);
                const WW3DFormat format = (m_bgfxData.format != WW3D_FORMAT_UNKNOWN) ? m_bgfxData.format : TextureFormat;

                surface_desc.Format = format;
                surface_desc.Width = level_width;
                surface_desc.Height = level_height;
                return;
        }
#endif

        WWASSERT_PRINT(0, "Get_Level_Description: No backing texture available!\n");
        surface_desc.Format = TextureFormat;
        surface_desc.Width = 0;
        surface_desc.Height = 0;
}

// ----------------------------------------------------------------------------

SurfaceClass *TextureClass::Get_Surface_Level(unsigned int level)
{
#if WW3D_BGFX_AVAILABLE
        if (!D3DTexture && DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                return SurfaceClass::Create_From_Bgfx_Texture(this, level);
        }
#endif

        IDirect3DSurface8 *d3d_surface = NULL;
        DX8_ErrorCode(D3DTexture->GetSurfaceLevel(level, &d3d_surface));
        SurfaceClass *surface = W3DNEW SurfaceClass(d3d_surface);
#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && surface)
        {
                surface->Create_Bgfx_Surface_From_Texture(this, level);
        }
#endif
        d3d_surface->Release();
        return surface;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Get_Priority(void)
{
        if (D3DTexture) {
                return D3DTexture->GetPriority();
        }

#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                return 0;
        }
#endif

        WWASSERT_PRINT(0, "Get_Priority: No backing texture available!\n");
        return 0;
}

// ----------------------------------------------------------------------------

unsigned int TextureClass::Set_Priority(unsigned int priority)
{
        if (D3DTexture) {
                return D3DTexture->SetPriority(priority);
        }

#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                return 0;
        }
#endif

        WWASSERT_PRINT(0, "Set_Priority: No backing texture available!\n");
        return 0;
}

// ----------------------------------------------------------------------------

void TextureClass::Set_Mip_Mapping(FilterType mipmap)
{
	if (mipmap != FILTER_TYPE_NONE && Get_Mip_Level_Count() <= 1) {
		WWASSERT_PRINT(0, "Trying to enable MipMapping on texture w/o Mip levels!\n");
		return;
	}
	MipMapFilter=mipmap;
}

unsigned TextureClass::Get_Reduction() const
{
	if (MipLevelCount==MIP_LEVELS_1) return 0;

	int reduction=WW3D::Get_Texture_Reduction();
	if (MipLevelCount && reduction>MipLevelCount) {
		reduction=MipLevelCount;
	}
	return reduction;
}

// ----------------------------------------------------------------------------

void TextureClass::Apply(unsigned int stage)
{
	if (!Initialized) {
		Init();
	}
	LastAccessed=WW3D::Get_Sync_Time();

	DX8_RECORD_TEXTURE(this);

	// Set texture itself
	if (WW3D::Is_Texturing_Enabled()) {
		DX8Wrapper::Set_DX8_Texture(stage, D3DTexture);
	} else {
		DX8Wrapper::Set_DX8_Texture(stage, NULL);
	}

	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MINFILTER,_MinTextureFilters[TextureMinFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MAGFILTER,_MagTextureFilters[TextureMagFilter]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MIPFILTER,_MipMapFilters[MipMapFilter]);

	switch (Get_U_Addr_Mode()) {

		case TEXTURE_ADDRESS_REPEAT:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
			break;

		case TEXTURE_ADDRESS_CLAMP:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
			break;

	}

	switch (Get_V_Addr_Mode()) {

		case TEXTURE_ADDRESS_REPEAT:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
			break;

		case TEXTURE_ADDRESS_CLAMP:
			DX8Wrapper::Set_DX8_Texture_Stage_State(stage, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
			break;

	}
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_Null(unsigned int stage)
{
	// This function sets the render states for a "NULL" texture
	DX8Wrapper::Set_DX8_Texture(stage, NULL);
}

// ----------------------------------------------------------------------------

void TextureClass::Apply_New_Surface(bool initialized)
{
	if (D3DTexture)
	{
		D3DTexture->Release();
		D3DTexture = NULL;
	}

	IDirect3DTexture8* new_texture = (TextureLoadTask != NULL) ? TextureLoadTask->Peek_D3D_Texture() : NULL;
	if (new_texture)
	{
		D3DTexture = new_texture;
		D3DTexture->AddRef();
		if (initialized)
		{
			Initialized = true;
		}

		IDirect3DSurface8* surface;
		DX8_ErrorCode(D3DTexture->GetSurfaceLevel(0,&surface));
		D3DSURFACE_DESC d3d_desc;
		::ZeroMemory(&d3d_desc, sizeof(D3DSURFACE_DESC));
		DX8_ErrorCode(surface->GetDesc(&d3d_desc));
		TextureFormat = D3DFormat_To_WW3DFormat(d3d_desc.Format);
		surface->Release();

#if WW3D_BGFX_AVAILABLE
		if (DX8Wrapper::Is_Bgfx_Active())
		{
			if (!Has_Bgfx_Texture())
			{
				Create_Bgfx_Texture_From_D3D(D3DTexture, TextureFormat, false);
			}
		}
#endif
		return;
	}

#if WW3D_BGFX_AVAILABLE
        if (DX8Wrapper::Is_Bgfx_Active() && Has_Bgfx_Texture())
        {
                if (initialized)
                {
                        Initialized = true;
                }
                if (TextureFormat == WW3D_FORMAT_UNKNOWN && m_bgfxData.format != WW3D_FORMAT_UNKNOWN)
                {
                        TextureFormat = m_bgfxData.format;
                }
                return;
        }
#endif

        WWASSERT(D3DTexture);
}

// ----------------------------------------------------------------------------

unsigned TextureClass::Get_Texture_Memory_Usage() const
{
	if (/*!ReductionEnabled || */!Initialized) return Calculate_Texture_Memory_Usage(this,0);
//	unsigned reduction=WW3D::Get_Texture_Reduction();
//	if (CurrentReductionFactor>reduction) reduction=CurrentReductionFactor;
	return Calculate_Texture_Memory_Usage(this,0);//reduction);
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Size()
{
	int total_locked_surface_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {

		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
//			total_locked_surface_size+=tex->Get_Non_Reduced_Texture_Memory_Usage();
			total_locked_surface_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_locked_surface_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		total_texture_size+=tex->Get_Texture_Memory_Usage();
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Lightmap()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Size()
{
	int total_texture_size=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (tex->Is_Procedural()) {
			total_texture_size+=tex->Get_Texture_Memory_Usage();
		}
	}
	return total_texture_size;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		texture_count++;
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Lightmap_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Lightmap()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Procedural_Texture_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		if (ite.Peek_Value()->Is_Procedural()) {
			texture_count++;
		}
	}

	return texture_count;
}

// ----------------------------------------------------------------------------

int TextureClass::_Get_Total_Locked_Surface_Count()
{
	int texture_count=0;

	HashTemplateIterator<StringClass,TextureClass*> ite(WW3DAssetManager::Get_Instance()->Texture_Hash());
	// Loop through all the textures in the manager
	for (ite.First ();!ite.Is_Done();ite.Next ()) {
		// Get the current texture
		TextureClass* tex=ite.Peek_Value();
		if (!tex->Initialized) {
			texture_count++;
		}
	}

	return texture_count;
}
/*
bool Validate_Filters(unsigned type)
{
	ShaderClass shader=ShaderClass::_PresetOpaqueShader;
	shader.Apply();
	DX8Wrapper::Set_DX8_Texture(0, MissingTexture::_Get_Missing_Texture());
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MINFILTER,_MinTextureFilters[type]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MAGFILTER,_MagTextureFilters[type]);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,D3DTSS_MIPFILTER,_MipMapFilters[type]);
	unsigned long passes;
	HRESULT hres=DX8Wrapper::_Get_D3D_Device8()->ValidateDevice(&passes);
	return !FAILED(hres);
}
*/
void TextureClass::_Init_Filters()
{
	const D3DCAPS8& dx8caps=DX8Caps::Get_Default_Caps();

	_MinTextureFilters[FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MagTextureFilters[FILTER_TYPE_NONE]=D3DTEXF_POINT;
	_MipMapFilters[FILTER_TYPE_NONE]=D3DTEXF_NONE;

	_MinTextureFilters[FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MagTextureFilters[FILTER_TYPE_FAST]=D3DTEXF_LINEAR;
	_MipMapFilters[FILTER_TYPE_FAST]=D3DTEXF_POINT;

	// Jani: Disabling anisotropic filtering as it doesn't seem to work with the latest nVidia drivers.
	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFAFLATCUBIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_FLATCUBIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFANISOTROPIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFGAUSSIANCUBIC) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_GAUSSIANCUBIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFLINEAR) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MAGFPOINT) _MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No magnification filter found!"));
	}

	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFANISOTROPIC) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_ANISOTROPIC;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFLINEAR) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MINFPOINT) _MinTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No minification filter found!"));
	}

	if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MIPFLINEAR) _MipMapFilters[FILTER_TYPE_BEST]=D3DTEXF_LINEAR;
	else if (dx8caps.TextureFilterCaps&D3DPTFILTERCAPS_MIPFPOINT) _MipMapFilters[FILTER_TYPE_BEST]=D3DTEXF_POINT;
	else {
		WWASSERT_PRINT(0,("No mip filter found!"));
	}

//_MagTextureFilters[FILTER_TYPE_BEST]=D3DTEXF_FLATCUBIC;
//	WWASSERT(Validate_Filters(FILTER_TYPE_BEST));

	_MinTextureFilters[FILTER_TYPE_DEFAULT]=_MinTextureFilters[FILTER_TYPE_BEST];
	_MagTextureFilters[FILTER_TYPE_DEFAULT]=_MagTextureFilters[FILTER_TYPE_BEST];
	_MipMapFilters[FILTER_TYPE_DEFAULT]=_MipMapFilters[FILTER_TYPE_BEST];

	for (int stage=0;stage<MeshMatDescClass::MAX_TEX_STAGES;++stage) {
		DX8Wrapper::Set_DX8_Texture_Stage_State(stage,D3DTSS_MAXANISOTROPY,2);
	}


}

void TextureClass::_Set_Default_Min_Filter(FilterType filter)
{
	_MinTextureFilters[FILTER_TYPE_DEFAULT]=_MinTextureFilters[filter];
}

void TextureClass::_Set_Default_Mag_Filter(FilterType filter)
{
	_MagTextureFilters[FILTER_TYPE_DEFAULT]=_MagTextureFilters[filter];
}

void TextureClass::_Set_Default_Mip_Filter(FilterType filter)
{
	_MipMapFilters[FILTER_TYPE_DEFAULT]=_MipMapFilters[filter];
}

// Utility functions
TextureClass *Load_Texture(ChunkLoadClass & cload)
{
	// Assume failure
	TextureClass *newtex = NULL;

	char name[256];
	if (cload.Open_Chunk () && (cload.Cur_Chunk_ID () == W3D_CHUNK_TEXTURE)) {

		W3dTextureInfoStruct texinfo;
		bool hastexinfo = false;

		/*
		** Read in the texture filename, and a possible texture info structure.
		*/
		while (cload.Open_Chunk()) {
			switch (cload.Cur_Chunk_ID()) {
				case W3D_CHUNK_TEXTURE_NAME:
					cload.Read(&name,cload.Cur_Chunk_Length());
					break;

				case W3D_CHUNK_TEXTURE_INFO:
					cload.Read(&texinfo,sizeof(W3dTextureInfoStruct));
					hastexinfo = true;
					break;
			};
			cload.Close_Chunk();
		}
		cload.Close_Chunk();

		/*
		** Get the texture from the asset manager
		*/
		if (hastexinfo) {
			
			TextureClass::MipCountType mipcount;

			bool no_lod = ((texinfo.Attributes & W3DTEXTURE_NO_LOD) == W3DTEXTURE_NO_LOD);
			
			if (no_lod) {
				mipcount = TextureClass::MIP_LEVELS_1;
			} else {
				switch (texinfo.Attributes & W3DTEXTURE_MIP_LEVELS_MASK) {

					case W3DTEXTURE_MIP_LEVELS_ALL:
						mipcount = TextureClass::MIP_LEVELS_ALL;
						break;

					case W3DTEXTURE_MIP_LEVELS_2:
						mipcount = TextureClass::MIP_LEVELS_2;
						break;

					case W3DTEXTURE_MIP_LEVELS_3:
						mipcount = TextureClass::MIP_LEVELS_3;
						break;

					case W3DTEXTURE_MIP_LEVELS_4:
						mipcount = TextureClass::MIP_LEVELS_4;
						break;

					default:
						WWASSERT (false);
						mipcount = TextureClass::MIP_LEVELS_ALL;
						break;
				}
			}
			newtex = WW3DAssetManager::Get_Instance()->Get_Texture (name, mipcount);

			if (no_lod) {
				newtex->Set_Mip_Mapping(TextureClass::FILTER_TYPE_NONE);
			}
			bool u_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_U) != 0);
			newtex->Set_U_Addr_Mode(u_clamp ? TextureClass::TEXTURE_ADDRESS_CLAMP : TextureClass::TEXTURE_ADDRESS_REPEAT);
			bool v_clamp = ((texinfo.Attributes & W3DTEXTURE_CLAMP_V) != 0);
			newtex->Set_V_Addr_Mode(v_clamp ? TextureClass::TEXTURE_ADDRESS_CLAMP : TextureClass::TEXTURE_ADDRESS_REPEAT);

			switch (texinfo.Attributes & W3DTEXTURE_TYPE_MASK) {

				case W3DTEXTURE_TYPE_COLORMAP: 
					// Do nothing.
					break;

				case W3DTEXTURE_TYPE_BUMPMAP:
				{
					TextureClass *releasetex = newtex;

					// Format is assumed to be a grayscale heightmap. Convert it to a bump map.
					newtex = WW3DAssetManager::Get_Instance()->Get_Bumpmap_Based_On_Texture (newtex);
					WW3DAssetManager::Get_Instance()->Release_Texture (releasetex);
					break;
				}

				default:
					WWASSERT (false);
					break;
			}

		} else {
			newtex = WW3DAssetManager::Get_Instance()->Get_Texture(name);
		}

		WWASSERT(newtex);
	}

	// Return a pointer to the new texture	
	return newtex;
}

// Utility function used by Save_Texture
void setup_texture_attributes(TextureClass * tex, W3dTextureInfoStruct * texinfo)
{
	texinfo->Attributes = 0;

	if (tex->Get_Mip_Mapping() == TextureClass::FILTER_TYPE_NONE) texinfo->Attributes |= W3DTEXTURE_NO_LOD;
	if (tex->Get_U_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_U;
	if (tex->Get_V_Addr_Mode() == TextureClass::TEXTURE_ADDRESS_CLAMP) texinfo->Attributes |= W3DTEXTURE_CLAMP_V;
}


void Save_Texture(TextureClass * texture,ChunkSaveClass & csave)
{
	const char * filename;
	W3dTextureInfoStruct texinfo;
	memset(&texinfo,0,sizeof(texinfo));

	filename = texture->Get_Full_Path();

	setup_texture_attributes(texture, &texinfo);

	csave.Begin_Chunk(W3D_CHUNK_TEXTURE_NAME);
	csave.Write(filename,strlen(filename)+1);
	csave.End_Chunk();

	if ((texinfo.Attributes != 0) || (texinfo.AnimType != 0) || (texinfo.FrameCount != 0)) {
		csave.Begin_Chunk(W3D_CHUNK_TEXTURE_INFO);
		csave.Write(&texinfo, sizeof(texinfo));
		csave.End_Chunk();
	}
}

// ----------------------------------------------------------------------------

BumpmapTextureClass::BumpmapTextureClass(TextureClass* texture)
	:
//	TextureClass(texture->Get_Width(),texture->Get_Height(),texture->Get_Textur4e_Format(),MIP_LEVELS_1)
	TextureClass(TextureLoader::Generate_Bumpmap(texture))
{
//	D3DTexture=TextureLoader::Generate_Bumpmap(texture);
//		TextureLoader:::Generage_Bumpmap
}

BumpmapTextureClass::~BumpmapTextureClass()
{
}

