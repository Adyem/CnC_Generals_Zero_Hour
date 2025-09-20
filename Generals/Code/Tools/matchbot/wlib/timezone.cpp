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

#include "xtime.h"
#include "timezone.h"

void GetTimezoneInfo(const char * &timezone_str, int &timezone_offset) {
        timezone_str = "Unknown Timezone";
        timezone_offset = 0;
#if defined(__unix__) || defined(__APPLE__) || defined(__linux__)
        struct timeval   unixtime;
        struct timezone  unixtzone;
        gettimeofday(&unixtime,&unixtzone);

        struct tm unixtm;
        localtime_r(&unixtime.tv_sec, &unixtm);

        if (unixtm.tm_isdst > 0 && tzname[1] != nullptr) {
                // Daylight savings time
                timezone_str = tzname[1];
        } else if (tzname[0] != nullptr) {
                timezone_str = tzname[0];
        }

#if defined(__APPLE__) || (defined(__linux__) && defined(__GLIBC__))
        timezone_offset = static_cast<int>(unixtm.tm_gmtoff);
#else
        timezone_offset = -unixtzone.tz_minuteswest * 60;
        if (unixtm.tm_isdst > 0) {
                timezone_offset += 3600;
        }
#endif
#else
        struct _timeb    wintime;
        _ftime(&wintime);

        if (wintime.dstflag) {
                // Daylight savings time
                if (_daylight) {
                        timezone_str = _tzname[1];
                }
        } else {
                timezone_str = _tzname[0];
        }
        timezone_offset = wintime.timezone * 60; // its in minutes...
#endif
}

const char * TimezoneString(void) {
	const char *timezone_str;
	int timezone_offset;
	GetTimezoneInfo(timezone_str, timezone_offset);
	return timezone_str;
}

int TimezoneOffset(void) {
	const char *timezone_str;
	int timezone_offset;
	GetTimezoneInfo(timezone_str, timezone_offset);
	return timezone_offset;
}
