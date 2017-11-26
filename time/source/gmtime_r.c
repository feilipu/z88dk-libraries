/*
 * (C)2012 Michael Duane Rice All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer. Redistributions in binary
 * form must reproduce the above copyright notice, this list of conditions
 * and the following disclaimer in the documentation and/or other materials
 * provided with the distribution. Neither the name of the copyright holders
 * nor the names of contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* $Id$ */

/* Re entrant version of gmtime(). */

#include <stdlib.h>
#include <inttypes.h>

#include "time.h"

void
gmtime_r(const time_t * timer, struct tm * timeptr)
{
    uint32_t        fract;
    uint16_t        days, n, leapyear, years;

    /* break down timer into whole and fractional parts of 1 day */
    days = *timer / 86400UL;
    fract = *timer % 86400UL;

    /*
            Extract hour, minute, and second from the fractional day
        */
    timeptr->tm_sec = (uint8_t)(fract%60);
    timeptr->tm_min = (uint8_t)((fract/60)%60);
    timeptr->tm_hour = (uint8_t)(fract/3600UL);

    /* Determine day of week ( the epoch was a Saturday ) */
    n = days + SATURDAY;
    n %= 7;
    timeptr->tm_wday = n;

    /*
        * Our epoch year has the property of being at the conjunction of all three 'leap cycles',
        * 4, 100, and 400 years ( though we can ignore the 400 year cycle in this library).
        *
        * Using this property, we can easily 'map' the time stamp into the leap cycles, quickly
        * deriving the year and day of year, along with the fact of whether it is a leap year.
        */

    /* map into a 100 year cycle */
    years = (uint16_t)(((uint32_t)days/36525UL) * 100);

    /* map into a 4 year cycle */
    years += (uint16_t)((((uint32_t)days%36525UL) / 1461UL) * 4);
    days = (uint16_t)(((uint32_t)days%36525UL) % 1461UL);
    if (years > 100)
        days++;

    /*
         * 'years' is now at the first year of a 4 year leap cycle, which will always be a leap year,
         * unless it is 100. 'days' is now an index into that cycle.
         */
    leapyear = 1;
    if (years == 100)
        leapyear = 0;

    /* compute length, in days, of first year of this cycle */
    n = 364 + leapyear;

    /*
     * if the number of days remaining is greater than the length of the
     * first year, we make one more division.
     */
    if (days > n) {
        days -= leapyear;
        leapyear = 0;
        years += days/365;
        days = days%365;
    }
    timeptr->tm_year = 100 + years;
    timeptr->tm_yday = days;

    /*
            Given the year, day of year, and leap year indicator, we can break down the
            month and day of month. If the day of year is less than 59 (or 60 if a leap year), then
            we handle the Jan/Feb month pair as an exception.
        */
    n = 59 + leapyear;
    if (days < n) {
        /* special case: Jan/Feb month pair */
        timeptr->tm_mon =  days/31;
        timeptr->tm_mday = days%31;
    } else {
        /*
            The remaining 10 months form a regular pattern of 31 day months alternating with 30 day
            months, with a 'phase change' between July and August (153 days after March 1).
            We proceed by mapping our position into either March-July or August-December.
            */
        days -= n;
        timeptr->tm_mon = 2 + days/153 * 5;

        /* map into a 61 day pair of months */
        timeptr->tm_mon += ((days%153)/61) * 2;

        /* map into a month */
        timeptr->tm_mon += ((days%153)%61)/31;
        timeptr->tm_mday = ((days%153)%61)%31;
    }

    /*
            Cleanup and return
        */
    timeptr->tm_isdst = 0;  /* gmt is never in DST */
    timeptr->tm_mday++; /* tm_mday is 1 based */

}
