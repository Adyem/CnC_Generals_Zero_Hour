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
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando                                                     *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/systimer.h                             $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 12/09/01 6:41p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once
#ifndef _SYSTIMER_H

#include "always.h"
#include <chrono>
#include <cstdint>

#define TIMEGETTIME SystemTime.Get
#define MS_TIMER_SECOND 1000

/*
** Class that just wraps around timeGetTime()
**
**
*/
class SysTimeClass
{

	public:

		using clock_type = std::chrono::steady_clock;
		using duration_type = std::chrono::milliseconds;

		SysTimeClass(void);	//default constructor
		~SysTimeClass();	//default destructor

		/*
		** Get. Use everywhere you would use timeGetTime
		*/
		WWINLINE std::uint64_t Get_Milliseconds(void);
		WWINLINE unsigned long Get(void);
		WWINLINE unsigned long operator () (void) {return(Get());}
		WWINLINE operator unsigned long(void) {return(Get());}

		/*
		** Use periodically (like every few days!) to make sure the timer doesn't wrap.
		*/
		void Reset(void);

		/*
		** See if the timer is about to wrap.
		*/
		bool Is_Getting_Late(void);

	private:

		duration_type Get_Duration(void);

		/*
		** Time we were first called.
		*/
		clock_type::time_point StartTime;

};

extern SysTimeClass SystemTime;


/***********************************************************************************************
 * SysTimeClass::Get -- Wrapper around system timeGetTime() api call                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Current system time in ms                                                         *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/25/2001 1:38PM ST : Created                                                            *
 *=============================================================================================*/
WWINLINE SysTimeClass::duration_type SysTimeClass::Get_Duration(void)
{
	/*
	** This has to be static here since we don't know if we will get called in a global constructor of another object before our
	** constructor gets called. In fact, we don't even have a constructor because it's pointless.
	*/
	static bool is_init = false;

	if (!is_init) {
		Reset();
		is_init = true;
	}

	return std::chrono::duration_cast<duration_type>(clock_type::now() - StartTime);
}

WWINLINE std::uint64_t SysTimeClass::Get_Milliseconds(void)
{
	return static_cast<std::uint64_t>(Get_Duration().count());
}

WWINLINE unsigned long SysTimeClass::Get(void)
{
	return static_cast<unsigned long>(Get_Milliseconds());
}



#ifdef timeGetTime
#undef timeGetTime
#define timeGetTime SystemTime.Get
#endif //timeGetTime




#endif //_SYSTIMER_H