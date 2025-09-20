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

/*****************************************************************************
**                                                                          **
**                       Westwood Studios Pacific.                          **
**                                                                          **
**                       Confidential Information					                  **
**                Copyright (C) 2000 - All Rights Reserved                  **
**                                                                          **
******************************************************************************
**                                                                          **
** Project:  Dune Emperor                                                   **
**                                                                          **
** Module:  <module> (<prefix>_)                                            **
**                                                                          **
** Version:  $ID$                                                           **
**                                                                          **
** File name:  audmpool.cpp                                                 **
**                                                                          **
** Created by:  10/23/95 TR                                                 **
**                                                                          **
** Description: <description>                                               **
**                                                                          **
*****************************************************************************/

/*****************************************************************************
**            Includes                                                      **
*****************************************************************************/

#include <cstdint>
#include <cstring>

#include <wpaudio/altypes.h>
#include <wpaudio/memory.h>

// 'assignment within condition expression'.
#pragma warning(disable : 4706)


DBG_DECLARE_TYPE ( AudioMemoryPool )
DBG_DECLARE_TYPE ( MemoryItem )

/*****************************************************************************
**          Externals                                                       **
*****************************************************************************/



/*****************************************************************************
**           Defines                                                        **
*****************************************************************************/



/*****************************************************************************
**        Private Types                                                     **
*****************************************************************************/



/*****************************************************************************
**         Private Data                                                     **
*****************************************************************************/



/*****************************************************************************
**         Public Data                                                      **
*****************************************************************************/



/*****************************************************************************
**         Private Prototypes                                               **
*****************************************************************************/



/*****************************************************************************
**          Private Functions                                               **
*****************************************************************************/



/*****************************************************************************
**          Public Functions                                                **
*****************************************************************************/

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

AudioMemoryPool* MemoryPoolCreate(std::size_t items, std::size_t size)
{
        DBG_ASSERT(items > 0);
        DBG_ASSERT(size > 0);

        const std::size_t poolSize = items * (size + sizeof(MemoryItem)) + sizeof(AudioMemoryPool);
        auto* pool = static_cast<AudioMemoryPool*>(AudioMemAllocZero(poolSize));

        if (pool == NULL)
        {
                return NULL;
        }

        auto* item = reinterpret_cast<MemoryItem*>(
                reinterpret_cast<std::uintptr_t>(pool) + sizeof(AudioMemoryPool));

        pool->next = item;
        pool->itemSize = size;
        pool->itemCount = items;
        pool->itemsOut = 0;
        DBG_SET_TYPE(pool, AudioMemoryPool);

        for (std::size_t i = 0; i < items; ++i)
        {
                DBG_SET_TYPE(item, MemoryItem);

                if (i + 1 < items)
                {
                        auto* next = reinterpret_cast<MemoryItem*>(
                                reinterpret_cast<std::uintptr_t>(item) + sizeof(MemoryItem) + size);
                        item->next = next;
                        item = next;
                }
                else
                {
                        item->next = NULL;
                }
        }

        return pool;
}

		item->next = NULL;
		DBG_SET_TYPE ( item, MemoryItem );

	}

	return pool;

}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void			MemoryPoolDestroy ( AudioMemoryPool *pool )
{

	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );
	
	DBG_CODE
	(
	   	if ( pool->itemsOut > 0 )
		{
		   	DBGPRINTF ( ( "Destroying memory pool with %d items still out\n", pool->itemsOut) );
		}
	)
	
	AudioMemFree ( pool );
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void* MemoryPoolGetItem(AudioMemoryPool* pool)
{
        MemoryItem *item = NULL;

        DBG_ASSERT_TYPE ( pool, AudioMemoryPool );

        if (!(item = pool->next))
        {
                return NULL;
        }

        DBG_CODE ( pool->itemsOut++;)

        DBG_MSGASSERT ( pool->itemsOut <= pool->itemCount,( "pool overflow" ));

        DBG_ASSERT_TYPE ( item, MemoryItem ); //  !!! Memory corruption !!!

        pool->next = item->next;

        return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(item) + sizeof(MemoryItem));
}

void MemoryPoolReturnItem(AudioMemoryPool* pool, void* data)
{
        DBG_ASSERT_TYPE ( pool, AudioMemoryPool );
        DBG_ASSERT_PTR ( data );

        auto* item = reinterpret_cast<MemoryItem*>(reinterpret_cast<std::uintptr_t>(data) - sizeof(MemoryItem));

        DBG_ASSERT_TYPE ( item, MemoryItem ); //  returning invalid item to pool

        item->next = pool->next;
        pool->next = item;

        DBG_MSGASSERT ( pool->itemsOut > 0,( "Pool underflow" )); //  returning more items than were taken

        DBG_CODE ( pool->itemsOut--; )
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int 		  MemoryPoolCount ( AudioMemoryPool *pool )
{
	MemoryItem *item = NULL;
	int	count = 0;


	DBG_ASSERT_TYPE ( pool, AudioMemoryPool );

	if ( (item = pool->next) )
	{
		while ( item )
		{
		    count++;
		    item = item->next;
		}
	}
	
	return count;
}


/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

void AudioAddSlash ( char *string )
{
	int len = strlen ( string );

	if ( len )
	{
		if ( string[len-1] != '\\' )
		{
			string[len] = '\\';
			string[len+1] = 0;
		}
	}
}

/******************************************************************/
/*                                                                */
/*                                                                */
/******************************************************************/

int AudioHasPath ( const char *string )
{
	return ( strchr ( string, ':' ) || strchr ( string, '\\' ) || strchr ( string, '/' ) || strchr ( string, '.'));
}

