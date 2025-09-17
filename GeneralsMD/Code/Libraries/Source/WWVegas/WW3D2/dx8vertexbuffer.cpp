/*
**	Command & Conquer Generals Zero Hour(tm)
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
 *                 Project Name : ww3d                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/dx8vertexbuffer.cpp                    $*
 *                                                                                             *
 *              Original Author:: Jani Penttinen                                               *
 *                                                                                             *
 *                      $Author:: Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 5:06p                                             $*
 *                                                                                             *
 *                    $Revision:: 39                                                          $*
 *                                                                                             *
 * 06/26/02 KM VB Vertex format size update for shaders                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

//#define VERTEX_BUFFER_LOG

#include "dx8vertexbuffer.h"
#include "dx8wrapper.h"
#include "dx8fvf.h"
#include "dx8caps.h"
#include "thread.h"
#include "wwmemlog.h"
#include <D3dx8core.h>

#ifndef WW3D_BGFX_AVAILABLE
#define WW3D_BGFX_AVAILABLE 0
#endif

#if WW3D_BGFX_AVAILABLE
#define WW3D_BGFX_VERTEX_AVAILABLE 1
#include <bgfx/bgfx.h>
#include <vector>
#else
#define WW3D_BGFX_VERTEX_AVAILABLE 0
#endif

#define DEFAULT_VB_SIZE 5000

static bool _DynamicSortingVertexArrayInUse=false;
//static VertexFormatXYZNDUV2* _DynamicSortingVertexArray=NULL;
static SortingVertexBufferClass* _DynamicSortingVertexArray=NULL;
static unsigned short _DynamicSortingVertexArraySize=0;
static unsigned short _DynamicSortingVertexArrayOffset=0;	

static bool _DynamicDX8VertexBufferInUse=false;
static DX8VertexBufferClass* _DynamicDX8VertexBuffer=NULL;
static unsigned short _DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
static unsigned short _DynamicDX8VertexBufferOffset=0;

static const FVFInfoClass _DynamicFVFInfo(dynamic_fvf_type);

static int _DX8VertexBufferCount=0;

static int _VertexBufferCount;
static int _VertexBufferTotalVertices;
static int _VertexBufferTotalSize;

#if WW3D_BGFX_VERTEX_AVAILABLE
static uint8_t Get_TexCoord_Component_Count(unsigned fvf, unsigned stage)
{
        if ((fvf & D3DFVF_TEXCOORDSIZE1(stage)) == D3DFVF_TEXCOORDSIZE1(stage)) return 1;
        if ((fvf & D3DFVF_TEXCOORDSIZE2(stage)) == D3DFVF_TEXCOORDSIZE2(stage)) return 2;
        if ((fvf & D3DFVF_TEXCOORDSIZE3(stage)) == D3DFVF_TEXCOORDSIZE3(stage)) return 3;
        if ((fvf & D3DFVF_TEXCOORDSIZE4(stage)) == D3DFVF_TEXCOORDSIZE4(stage)) return 4;
        return 2;
}

struct DX8VertexBufferBgfxData
{
        DX8VertexBufferBgfxData(bool is_dynamic, uint32_t stride_bytes, uint32_t vertex_count)
                : dynamic(is_dynamic),
                  lock_active(false),
                  stride(stride_bytes),
                  vertex_count(vertex_count),
                  lock_offset(0),
                  lock_size(0),
                  data(stride_bytes * vertex_count)
        {
                dynamic_handle.idx = bgfx::kInvalidHandle;
                static_handle.idx = bgfx::kInvalidHandle;
        }

        bool dynamic;
        bool lock_active;
        uint32_t stride;
        uint32_t vertex_count;
        uint32_t lock_offset;
        uint32_t lock_size;
        std::vector<unsigned char> data;
        bgfx::VertexLayout layout;
        bgfx::DynamicVertexBufferHandle dynamic_handle;
        bgfx::VertexBufferHandle static_handle;
};

static void Build_Bgfx_Vertex_Layout(const FVFInfoClass& info, bgfx::VertexLayout& layout)
{
        const unsigned fvf = info.Get_FVF();
        layout.begin();
        layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
        if ((fvf & D3DFVF_NORMAL) == D3DFVF_NORMAL)
        {
                layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
        }
        if ((fvf & D3DFVF_DIFFUSE) == D3DFVF_DIFFUSE)
        {
                layout.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
        }
        if ((fvf & D3DFVF_SPECULAR) == D3DFVF_SPECULAR)
        {
                layout.add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true);
        }

        const uint32_t tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
        for (uint32_t stage = 0; stage < tex_count; ++stage)
        {
                bgfx::Attrib::Enum attrib = static_cast<bgfx::Attrib::Enum>(bgfx::Attrib::TexCoord0 + stage);
                layout.add(attrib, Get_TexCoord_Component_Count(fvf, stage), bgfx::AttribType::Float);
        }
        layout.end();
}
#endif // WW3D_BGFX_VERTEX_AVAILABLE

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::VertexBufferClass(unsigned type_, unsigned FVF, unsigned short vertex_count_, unsigned vertex_size)
	:
	VertexCount(vertex_count_),
	type(type_),
	engine_refs(0)
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(VertexCount);
	WWASSERT(type==BUFFER_TYPE_DX8 || type==BUFFER_TYPE_SORTING);
	WWASSERT((FVF!=0 && vertex_size==0) || (FVF==0 && vertex_size!=0));
	fvf_info=W3DNEW FVFInfoClass(FVF,vertex_size);

	_VertexBufferCount++;
	_VertexBufferTotalVertices+=VertexCount;
	_VertexBufferTotalSize+=VertexCount*fvf_info->Get_FVF_Size();
#ifdef VERTEX_BUFFER_LOG
	WWDEBUG_SAY(("New VB, %d vertices, size %d bytes\n",VertexCount,VertexCount*fvf_info->Get_FVF_Size()));
	WWDEBUG_SAY(("Total VB count: %d, total %d vertices, total size %d bytes\n",
		_VertexBufferCount,
		_VertexBufferTotalVertices,
		_VertexBufferTotalSize));
#endif
}

// ----------------------------------------------------------------------------

VertexBufferClass::~VertexBufferClass()
{
	_VertexBufferCount--;
	_VertexBufferTotalVertices-=VertexCount;
	_VertexBufferTotalSize-=VertexCount*fvf_info->Get_FVF_Size();

#ifdef VERTEX_BUFFER_LOG
	WWDEBUG_SAY(("Delete VB, %d vertices, size %d bytes\n",VertexCount,VertexCount*fvf_info->Get_FVF_Size()));
	WWDEBUG_SAY(("Total VB count: %d, total %d vertices, total size %d bytes\n",
		_VertexBufferCount,
		_VertexBufferTotalVertices,
		_VertexBufferTotalSize));
#endif
	delete fvf_info;
}

unsigned VertexBufferClass::Get_Total_Buffer_Count()
{
	return _VertexBufferCount;
}

unsigned VertexBufferClass::Get_Total_Allocated_Vertices()
{
	return _VertexBufferTotalVertices;
}

unsigned VertexBufferClass::Get_Total_Allocated_Memory()
{
	return _VertexBufferTotalSize;
}


// ----------------------------------------------------------------------------

void VertexBufferClass::Add_Engine_Ref() const
{
	engine_refs++;
}

// ----------------------------------------------------------------------------

void VertexBufferClass::Release_Engine_Ref() const
{
	engine_refs--;
	WWASSERT(engine_refs>=0);
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::WriteLockClass::WriteLockClass(VertexBufferClass* VertexBuffer, int flags)
	:
	VertexBufferLockClass(VertexBuffer)
{
	DX8_THREAD_ASSERT();
	WWASSERT(VertexBuffer);
	WWASSERT(!VertexBuffer->Engine_Refs());
	VertexBuffer->Add_Ref();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef VERTEX_BUFFER_LOG
		{
		StringClass fvf_name;
		VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("VertexBuffer->Lock(start_index: 0, index_range: 0(%d), fvf_size: %d, fvf: %s)\n",
			VertexBuffer->Get_Vertex_Count(),
			VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
#endif
		Vertices = static_cast<DX8VertexBufferClass*>(VertexBuffer)->Lock(
			0,
			0,
			flags);
		break;
	case BUFFER_TYPE_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(VertexBuffer)->VertexBuffer;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

VertexBufferClass::WriteLockClass::~WriteLockClass()
{
	DX8_THREAD_ASSERT();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef VERTEX_BUFFER_LOG
		WWDEBUG_SAY(("VertexBuffer->Unlock()\n"));
#endif
		static_cast<DX8VertexBufferClass*>(VertexBuffer)->Unlock();
		break;
	case BUFFER_TYPE_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
	VertexBuffer->Release_Ref();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

VertexBufferClass::AppendLockClass::AppendLockClass(VertexBufferClass* VertexBuffer,unsigned start_index, unsigned index_range)
	:
	VertexBufferLockClass(VertexBuffer)
{
	DX8_THREAD_ASSERT();
	WWASSERT(VertexBuffer);
	WWASSERT(!VertexBuffer->Engine_Refs());
	WWASSERT(start_index+index_range<=VertexBuffer->Get_Vertex_Count());
	VertexBuffer->Add_Ref();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef VERTEX_BUFFER_LOG
		{
		StringClass fvf_name;
		VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("VertexBuffer->Lock(start_index: %d, index_range: %d, fvf_size: %d, fvf: %s)\n",
			start_index,
			index_range,
			VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
#endif
		Vertices = static_cast<DX8VertexBufferClass*>(VertexBuffer)->Lock(
			start_index*VertexBuffer->FVF_Info().Get_FVF_Size(),
			index_range*VertexBuffer->FVF_Info().Get_FVF_Size(),
			0);
		break;
	case BUFFER_TYPE_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(VertexBuffer)->VertexBuffer+start_index;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

VertexBufferClass::AppendLockClass::~AppendLockClass()
{
	DX8_THREAD_ASSERT();
	switch (VertexBuffer->Type()) {
	case BUFFER_TYPE_DX8:
#ifdef VERTEX_BUFFER_LOG
		WWDEBUG_SAY(("VertexBuffer->Unlock()\n"));
#endif
		static_cast<DX8VertexBufferClass*>(VertexBuffer)->Unlock();
		break;
	case BUFFER_TYPE_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
	VertexBuffer->Release_Ref();
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

SortingVertexBufferClass::SortingVertexBufferClass(unsigned short VertexCount)
	:
	VertexBufferClass(BUFFER_TYPE_SORTING, dynamic_fvf_type, VertexCount)
{
	WWMEMLOG(MEM_RENDERER);
	VertexBuffer=W3DNEWARRAY VertexFormatXYZNDUV2[VertexCount];
}

// ----------------------------------------------------------------------------

SortingVertexBufferClass::~SortingVertexBufferClass()
{
	delete[] VertexBuffer;
}


// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

//	bool dynamic=false,bool softwarevp=false);

DX8VertexBufferClass::DX8VertexBufferClass(unsigned FVF, unsigned short vertex_count_, UsageType usage, unsigned vertex_size)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, FVF, vertex_count_, vertex_size),
	VertexBuffer(NULL),
	m_bgfxData(NULL)
{
	Create_Vertex_Buffer(usage);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector3* normals, 
	const Vector2* tex_coords, 
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL, VertexCount),
	VertexBuffer(NULL),
	m_bgfxData(NULL)
{
	WWASSERT(vertices);
	WWASSERT(normals);
	WWASSERT(tex_coords);

	Create_Vertex_Buffer(usage);
	Copy(vertices,normals,tex_coords,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector3* normals, 
	const Vector4* diffuse,
	const Vector2* tex_coords, 
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_NORMAL|D3DFVF_DIFFUSE, VertexCount),
	VertexBuffer(NULL),
	m_bgfxData(NULL)
{
	WWASSERT(vertices);
	WWASSERT(normals);
	WWASSERT(tex_coords);
	WWASSERT(diffuse);

	Create_Vertex_Buffer(usage);
	Copy(vertices,normals,tex_coords,diffuse,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector4* diffuse,
	const Vector2* tex_coords, 
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1|D3DFVF_DIFFUSE, VertexCount),
	VertexBuffer(NULL),
	m_bgfxData(NULL)
{
	WWASSERT(vertices);
	WWASSERT(tex_coords);
	WWASSERT(diffuse);

	Create_Vertex_Buffer(usage);
	Copy(vertices,tex_coords,diffuse,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::DX8VertexBufferClass(
	const Vector3* vertices,
	const Vector2* tex_coords, 
	unsigned short VertexCount,
	UsageType usage)
	:
	VertexBufferClass(BUFFER_TYPE_DX8, D3DFVF_XYZ|D3DFVF_TEX1, VertexCount),
	VertexBuffer(NULL),
	m_bgfxData(NULL)
{
	WWASSERT(vertices);
	WWASSERT(tex_coords);

	Create_Vertex_Buffer(usage);
	Copy(vertices,tex_coords,0,VertexCount);
}

// ----------------------------------------------------------------------------

DX8VertexBufferClass::~DX8VertexBufferClass()
{
#if WW3D_BGFX_VERTEX_AVAILABLE
if (m_bgfxData)
{
if (m_bgfxData->dynamic)
{
if (bgfx::isValid(m_bgfxData->dynamic_handle))
{
bgfx::destroy(m_bgfxData->dynamic_handle);
}
}
else if (bgfx::isValid(m_bgfxData->static_handle))
{
bgfx::destroy(m_bgfxData->static_handle);
}
delete m_bgfxData;
m_bgfxData = NULL;
}
#endif

if (VertexBuffer)
{
#ifdef VERTEX_BUFFER_LOG
WWDEBUG_SAY(("VertexBuffer->Release()\n"));
_DX8VertexBufferCount--;
WWDEBUG_SAY(("Current vertex buffer count: %d\n",_DX8VertexBufferCount));
#endif
VertexBuffer->Release();
VertexBuffer = NULL;
}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Create_Vertex_Buffer(UsageType usage)
{
	DX8_THREAD_ASSERT();
	WWASSERT(!VertexBuffer);

#if WW3D_BGFX_VERTEX_AVAILABLE
	if (DX8Wrapper::Is_Bgfx_Active())
	{
		WWASSERT(m_bgfxData == NULL);
		m_bgfxData = W3DNEW DX8VertexBufferBgfxData((usage & USAGE_DYNAMIC) != 0, FVF_Info().Get_FVF_Size(), VertexCount);
		Build_Bgfx_Vertex_Layout(FVF_Info(), m_bgfxData->layout);

		if (m_bgfxData->dynamic)
		{
			m_bgfxData->dynamic_handle = bgfx::createDynamicVertexBuffer(VertexCount, m_bgfxData->layout);
		}

		return;
	}
#endif

#ifdef VERTEX_BUFFER_LOG
	StringClass fvf_name;
	FVF_Info().Get_FVF_Name(fvf_name);
	WWDEBUG_SAY(("CreateVertexBuffer(fvfsize=%d, vertex_count=%d, D3DUSAGE_WRITEONLY|%s|%s, fvf: %s, %s)\n",
		FVF_Info().Get_FVF_Size(),
		VertexCount,
		(usage&USAGE_DYNAMIC) ? "D3DUSAGE_DYNAMIC" : "-",
		(usage&USAGE_SOFTWAREPROCESSING) ? "D3DUSAGE_SOFTWAREPROCESSING" : "-",
		fvf_name,
		(usage&USAGE_DYNAMIC) ? "D3DPOOL_DEFAULT" : "D3DPOOL_MANAGED"));
	_DX8VertexBufferCount++;
	WWDEBUG_SAY(("Current vertex buffer count: %d\n",_DX8VertexBufferCount));
#endif

	unsigned usage_flags=
		D3DUSAGE_WRITEONLY|
		((usage&USAGE_DYNAMIC) ? D3DUSAGE_DYNAMIC : 0)|
		((usage&USAGE_NPATCHES) ? D3DUSAGE_NPATCHES : 0)|
		((usage&USAGE_SOFTWAREPROCESSING) ? D3DUSAGE_SOFTWAREPROCESSING : 0);
	if (!DX8Wrapper::Get_Current_Caps()->Support_TnL()) {
		usage_flags|=D3DUSAGE_SOFTWAREPROCESSING;
	}

	// New Code
	if (!DX8Wrapper::Get_Current_Caps()->Support_TnL()) {
		usage_flags|=D3DUSAGE_SOFTWAREPROCESSING;
	}

	HRESULT ret=DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer);
	if (SUCCEEDED(ret)) {
		return;
	}

	WWDEBUG_SAY(("Vertex buffer creation failed, trying to release assets...\n"));

	// Vertex buffer creation failed, so try releasing least used textures and flushing the mesh cache.

	// Free all textures that haven't been used in the last 5 seconds
	TextureClass::Invalidate_Old_Unused_Textures(5000);

	// Invalidate the mesh cache
	WW3D::_Invalidate_Mesh_Cache();

	//@todo: Find some way to invalidate the textures too
	ret = DX8Wrapper::_Get_D3D_Device8()->ResourceManagerDiscardBytes(0);

	// Try again...
	ret=DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer);

	if (SUCCEEDED(ret)) {
		WWDEBUG_SAY(("...Vertex buffer creation succesful\n"));
	}

	// If it still fails it is fatal
	DX8_ErrorCode(ret);

	/* Old Code
	DX8CALL(CreateVertexBuffer(
		FVF_Info().Get_FVF_Size()*VertexCount,
		usage_flags,
		FVF_Info().Get_FVF(),
		(usage&USAGE_DYNAMIC) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED,
		&VertexBuffer));
	*/
}

// ----------------------------------------------------------------------------

void* DX8VertexBufferClass::Lock(unsigned offset_bytes, unsigned size_bytes, unsigned flags)
{
#if WW3D_BGFX_VERTEX_AVAILABLE
	if (m_bgfxData)
	{
		const uint32_t total_size = static_cast<uint32_t>(m_bgfxData->data.size());
		if (offset_bytes >= total_size)
		{
			return NULL;
		}

		uint32_t range = size_bytes ? size_bytes : (total_size - offset_bytes);
		if (offset_bytes + range > total_size)
		{
			range = total_size - offset_bytes;
		}

		m_bgfxData->lock_offset = offset_bytes;
		m_bgfxData->lock_size = range;
		m_bgfxData->lock_active = true;

		return &m_bgfxData->data[offset_bytes];
	}
#endif

	unsigned char* bytes = NULL;
	DX8_Assert();
	DX8_ErrorCode(VertexBuffer->Lock(offset_bytes, size_bytes, &bytes, flags));
	return bytes;
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Unlock()
{
#if WW3D_BGFX_VERTEX_AVAILABLE
        if (m_bgfxData)
        {
		const uint32_t total_size = static_cast<uint32_t>(m_bgfxData->data.size());
		uint32_t offset = m_bgfxData->lock_offset;
		uint32_t range = m_bgfxData->lock_size ? m_bgfxData->lock_size : (total_size - offset);

		if (offset < total_size && range)
		{
			if (offset + range > total_size)
			{
				range = total_size - offset;
			}

			if (m_bgfxData->dynamic)
			{
				if (bgfx::isValid(m_bgfxData->dynamic_handle))
				{
					WWASSERT((offset % m_bgfxData->stride) == 0);
					WWASSERT((range % m_bgfxData->stride) == 0);
					const bgfx::Memory* memory = bgfx::copy(&m_bgfxData->data[offset], range);
					const uint32_t vertex_offset = offset / m_bgfxData->stride;
					bgfx::update(m_bgfxData->dynamic_handle, vertex_offset, memory);
				}
			}
			else
			{
				const bgfx::Memory* memory = bgfx::copy(m_bgfxData->data.data(), total_size);
				if (bgfx::isValid(m_bgfxData->static_handle))
				{
					bgfx::destroy(m_bgfxData->static_handle);
				}
				m_bgfxData->static_handle = bgfx::createVertexBuffer(memory, m_bgfxData->layout);
			}
		}

		m_bgfxData->lock_active = false;
		m_bgfxData->lock_offset = 0;
		m_bgfxData->lock_size = 0;
		return;
	}
#endif

	DX8_Assert();
        DX8_ErrorCode(VertexBuffer->Unlock());
}

#if WW3D_BGFX_VERTEX_AVAILABLE
bool DX8VertexBufferClass::Has_Bgfx_Vertex_Buffer() const
{
        return (m_bgfxData != NULL);
}

bool DX8VertexBufferClass::Uses_Bgfx_Dynamic_Buffer() const
{
        return m_bgfxData && m_bgfxData->dynamic;
}

bgfx::VertexBufferHandle DX8VertexBufferClass::Get_Bgfx_Vertex_Handle()
{
        bgfx::VertexBufferHandle handle = { bgfx::kInvalidHandle };
        if (!m_bgfxData || m_bgfxData->dynamic)
        {
                return handle;
        }

        if (!bgfx::isValid(m_bgfxData->static_handle) && !m_bgfxData->data.empty())
        {
                const bgfx::Memory* memory = bgfx::copy(m_bgfxData->data.data(), static_cast<uint32_t>(m_bgfxData->data.size()));
                m_bgfxData->static_handle = bgfx::createVertexBuffer(memory, m_bgfxData->layout);
        }

        return m_bgfxData->static_handle;
}

bgfx::DynamicVertexBufferHandle DX8VertexBufferClass::Get_Bgfx_Dynamic_Vertex_Handle()
{
        bgfx::DynamicVertexBufferHandle handle = { bgfx::kInvalidHandle };
        if (!m_bgfxData || !m_bgfxData->dynamic)
        {
                return handle;
        }

        if (!bgfx::isValid(m_bgfxData->dynamic_handle))
        {
                m_bgfxData->dynamic_handle = bgfx::createDynamicVertexBuffer(m_bgfxData->vertex_count, m_bgfxData->layout);
        }

        return m_bgfxData->dynamic_handle;
}

const bgfx::VertexLayout* DX8VertexBufferClass::Get_Bgfx_Vertex_Layout() const
{
        return m_bgfxData ? &m_bgfxData->layout : NULL;
}

uint32_t DX8VertexBufferClass::Get_Bgfx_Vertex_Count() const
{
        return m_bgfxData ? m_bgfxData->vertex_count : 0;
}
#endif

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, unsigned first_vertex,unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(uv);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZNUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZNUV1* verts=(VertexFormatXYZNUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZNUV1* verts=(VertexFormatXYZNUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZ);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZ* verts=(VertexFormatXYZ*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZ* verts=(VertexFormatXYZ*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
		}
	}

}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector2* uv, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(uv);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZUV1* verts=(VertexFormatXYZUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZUV1* verts=(VertexFormatXYZUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZN);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZN* verts=(VertexFormatXYZN*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZN* verts=(VertexFormatXYZN*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector3* norm, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(norm);
	WWASSERT(uv);
	WWASSERT(diffuse);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZNDUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZNDUV1* verts=(VertexFormatXYZNDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZNDUV1* verts=(VertexFormatXYZNDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].nx=(*norm)[0];
			verts[v].ny=(*norm)[1];
			verts[v].nz=(*norm++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
}

// ----------------------------------------------------------------------------

void DX8VertexBufferClass::Copy(const Vector3* loc, const Vector2* uv, const Vector4* diffuse, unsigned first_vertex, unsigned count)
{
	WWASSERT(loc);
	WWASSERT(uv);
	WWASSERT(diffuse);
	WWASSERT(count<=VertexCount);
	WWASSERT(FVF_Info().Get_FVF()==DX8_FVF_XYZDUV1);

	if (first_vertex) {
		VertexBufferClass::AppendLockClass l(this,first_vertex,count);
		VertexFormatXYZDUV1* verts=(VertexFormatXYZDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
	else {
		VertexBufferClass::WriteLockClass l(this);
		VertexFormatXYZDUV1* verts=(VertexFormatXYZDUV1*)l.Get_Vertex_Array();
		for (unsigned v=0;v<count;++v) {
			verts[v].x=(*loc)[0];
			verts[v].y=(*loc)[1];
			verts[v].z=(*loc++)[2];
			verts[v].u1=(*uv)[0];
			verts[v].v1=(*uv++)[1];
			verts[v].diffuse=DX8Wrapper::Convert_Color(diffuse[v]);
		}
	}
}

// ----------------------------------------------------------------------------
//
//
//
// ----------------------------------------------------------------------------

DynamicVBAccessClass::DynamicVBAccessClass(unsigned t,unsigned fvf,unsigned short vertex_count_)
	:
	Type(t),
	FVFInfo(_DynamicFVFInfo),
	VertexCount(vertex_count_),
	VertexBuffer(0)
{
	WWASSERT(fvf==dynamic_fvf_type);
	WWASSERT(Type==BUFFER_TYPE_DYNAMIC_DX8 || Type==BUFFER_TYPE_DYNAMIC_SORTING);

	if (Type==BUFFER_TYPE_DYNAMIC_DX8) {
		Allocate_DX8_Dynamic_Buffer();
	}
	else {
		Allocate_Sorting_Dynamic_Buffer();
	}
}

DynamicVBAccessClass::~DynamicVBAccessClass()
{
	if (Type==BUFFER_TYPE_DYNAMIC_DX8) {
		_DynamicDX8VertexBufferInUse=false;
		_DynamicDX8VertexBufferOffset+=(unsigned) VertexCount;
	}
	else {
		_DynamicSortingVertexArrayInUse=false;
		_DynamicSortingVertexArrayOffset+=VertexCount;
	}
	
	REF_PTR_RELEASE (VertexBuffer);
}

// ----------------------------------------------------------------------------

void DynamicVBAccessClass::_Deinit()
{
	WWASSERT ((_DynamicDX8VertexBuffer == NULL) || (_DynamicDX8VertexBuffer->Num_Refs() == 1));
	REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
	_DynamicDX8VertexBufferInUse=false;
	_DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
	_DynamicDX8VertexBufferOffset=0;

	WWASSERT ((_DynamicSortingVertexArray == NULL) || (_DynamicSortingVertexArray->Num_Refs() == 1));
	REF_PTR_RELEASE(_DynamicSortingVertexArray);
	WWASSERT(!_DynamicSortingVertexArrayInUse);
	_DynamicSortingVertexArrayInUse=false;
	_DynamicSortingVertexArraySize=0;
	_DynamicSortingVertexArrayOffset=0;
}

void DynamicVBAccessClass::Allocate_DX8_Dynamic_Buffer()
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(!_DynamicDX8VertexBufferInUse);
	_DynamicDX8VertexBufferInUse=true;

	// If requesting more vertices than dynamic vertex buffer can fit, delete the vb
	// and adjust the size to the new count.
	if (VertexCount>_DynamicDX8VertexBufferSize) {
		REF_PTR_RELEASE(_DynamicDX8VertexBuffer);
		_DynamicDX8VertexBufferSize=VertexCount;
		if (_DynamicDX8VertexBufferSize<DEFAULT_VB_SIZE) _DynamicDX8VertexBufferSize=DEFAULT_VB_SIZE;
	}

	// Create a new vb if one doesn't exist currently
	if (!_DynamicDX8VertexBuffer) {
		unsigned usage=DX8VertexBufferClass::USAGE_DYNAMIC;
		if (DX8Wrapper::Get_Current_Caps()->Support_NPatches()) {
			usage|=DX8VertexBufferClass::USAGE_NPATCHES;
		}

		_DynamicDX8VertexBuffer=NEW_REF(DX8VertexBufferClass,(
			dynamic_fvf_type, 
			_DynamicDX8VertexBufferSize,
			(DX8VertexBufferClass::UsageType)usage));
		_DynamicDX8VertexBufferOffset=0;
	}

	// Any room at the end of the buffer?
	if (((unsigned)VertexCount+_DynamicDX8VertexBufferOffset)>_DynamicDX8VertexBufferSize) {
		_DynamicDX8VertexBufferOffset=0;
	}

	REF_PTR_SET(VertexBuffer,_DynamicDX8VertexBuffer);
	VertexBufferOffset=_DynamicDX8VertexBufferOffset;
}

void DynamicVBAccessClass::Allocate_Sorting_Dynamic_Buffer()
{
	WWMEMLOG(MEM_RENDERER);
	WWASSERT(!_DynamicSortingVertexArrayInUse);
	_DynamicSortingVertexArrayInUse=true;

	unsigned new_vertex_count=_DynamicSortingVertexArrayOffset+VertexCount;
	WWASSERT(new_vertex_count<65536);
	if (new_vertex_count>_DynamicSortingVertexArraySize) {
		REF_PTR_RELEASE(_DynamicSortingVertexArray);
		_DynamicSortingVertexArraySize=new_vertex_count;
		if (_DynamicSortingVertexArraySize<DEFAULT_VB_SIZE) _DynamicSortingVertexArraySize=DEFAULT_VB_SIZE;
	}

	if (!_DynamicSortingVertexArray) {
		_DynamicSortingVertexArray=NEW_REF(SortingVertexBufferClass,(_DynamicSortingVertexArraySize));
		_DynamicSortingVertexArrayOffset=0;
	}

	REF_PTR_SET(VertexBuffer,_DynamicSortingVertexArray);
	VertexBufferOffset=_DynamicSortingVertexArrayOffset;
}

// ----------------------------------------------------------------------------
static int dx8_lock;
DynamicVBAccessClass::WriteLockClass::WriteLockClass(DynamicVBAccessClass* dynamic_vb_access_)
	:
	DynamicVBAccess(dynamic_vb_access_)
{
	DX8_THREAD_ASSERT();
	switch (DynamicVBAccess->Get_Type()) {
	case BUFFER_TYPE_DYNAMIC_DX8:
#ifdef VERTEX_BUFFER_LOG
/*		{
		WWASSERT(!dx8_lock);
		dx8_lock++;
		StringClass fvf_name;
		DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Name(fvf_name);
		WWDEBUG_SAY(("DynamicVertexBuffer->Lock(start_index: %d, index_range: %d, fvf_size: %d, fvf: %s)\n",
			DynamicVBAccess->VertexBufferOffset,
			DynamicVBAccess->Get_Vertex_Count(),
			DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Size(),
			fvf_name));
		}
*/
#endif
		WWASSERT(_DynamicDX8VertexBuffer);
//		WWASSERT(!_DynamicDX8VertexBuffer->Engine_Refs());

		DX8_Assert();
		// Lock with discard contents if the buffer offset is zero
		Vertices = static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer)->Lock(
				DynamicVBAccess->VertexBufferOffset*_DynamicDX8VertexBuffer->FVF_Info().Get_FVF_Size(),
				DynamicVBAccess->Get_Vertex_Count()*DynamicVBAccess->VertexBuffer->FVF_Info().Get_FVF_Size(),
				D3DLOCK_NOSYSLOCK | (!DynamicVBAccess->VertexBufferOffset ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE));
		break;
	case BUFFER_TYPE_DYNAMIC_SORTING:
		Vertices=static_cast<SortingVertexBufferClass*>(DynamicVBAccess->VertexBuffer)->VertexBuffer;
		Vertices+=DynamicVBAccess->VertexBufferOffset;
//		vertices=_DynamicSortingVertexArray+_DynamicSortingVertexArrayOffset;
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

DynamicVBAccessClass::WriteLockClass::~WriteLockClass()
{
	DX8_THREAD_ASSERT();
	switch (DynamicVBAccess->Get_Type()) {
	case BUFFER_TYPE_DYNAMIC_DX8:
#ifdef VERTEX_BUFFER_LOG
/*		dx8_lock--;
		WWASSERT(!dx8_lock);
		WWDEBUG_SAY(("DynamicVertexBuffer->Unlock()\n"));
*/
#endif
		DX8_Assert();
		static_cast<DX8VertexBufferClass*>(DynamicVBAccess->VertexBuffer)->Unlock();
		break;
	case BUFFER_TYPE_DYNAMIC_SORTING:
		break;
	default:
		WWASSERT(0);
		break;
	}
}

// ----------------------------------------------------------------------------

void DynamicVBAccessClass::_Reset(bool frame_changed)
{
	_DynamicSortingVertexArrayOffset=0;
	if (frame_changed) _DynamicDX8VertexBufferOffset=0;
}

unsigned short DynamicVBAccessClass::Get_Default_Vertex_Count(void)
{
	return _DynamicDX8VertexBufferSize;
}

