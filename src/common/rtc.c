/*
* Time access and conversion functions for Real Time Clock API
*/

#include "rtc.h"

/* Internal Definitions */

/* Some CONFIG options */

// #define RTC_USE_DAYLIGHTSAVING

#define FUNC_GETREALTIME _rtc_unixtime

/* Number of days in 4 years: (365 * 4) + 1 */
#define DAYSIN4YEARS 1461

/* Start of 'trusted' time - 00:00:00 01/09/1999 (Wed, September) */
#define RTC_TRUSTED_TIME_START 936144000

/* For mktime() */
#define _DAY_SEC           (24L * 60L * 60L)    /* secs in a day */
#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */
#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */
#define _DEC_SEC           315532800L           /* secs in 1970-1979 */
#define _BASE_YEAR         70L                  /* 1970 is the base year */
#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */
#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */
#define _MAX_YEAR          206L                 /* 2106 is the max year */

/* ChkAdd evaluates to true if dest = src1 + src2 has overflowed */
#define ChkAdd(dest, src1, src2)   ( ((src1 >= 0L) && (src2 >= 0L) \
                                      && (dest < 0L)) || ((src1 < 0L) && (src2 < 0L) && (dest >= 0L)) )

/* ChkMul evaluates to true if dest = src1 * src2 has overflowed */
#define ChkMul(dest, src1, src2)   ( src1 ? (dest/src1 != src2) : 0 )

/* Constant Static Data */
/* For mktime() days in month comulative */
static const int16_t _days[] = { -1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333, 364};

static const uint8_t monthInfo[12] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#ifdef RTC_USE_DAYLIGHTSAVING
bool RTC_CheckForDaylightSaving(const RTCTM *aTime);
#endif

static volatile RTCTIME _rtc_unixtime;


/* Exported Functions */

bool RTC_TimeTrusted(void)
{
    /* Current time greater than an 'invalid' time? */
    if (FUNC_GETREALTIME >= RTC_TRUSTED_TIME_START)
        return true;
    return false;
}



void RTC_RawLocalTime(RTCTM *aExpand, RTCTIME time)
{
    uint32_t year;
    uint16_t month;

    /* Seconds is time mod 60 */
    aExpand->tm_sec = time % 60;
    time /= 60;

    /* Minutes is time mod 60 */
    aExpand->tm_min = time % 60;
    time /= 60;

    /* Hours is time mod 24 */
    aExpand->tm_hour = time % 24;
    time /= 24;

    /* We now have the number of days since 1970, but need days since 1900 */
    /* There are 17 leap years prior to 1970. Although 1900 was not a leap year */
    /* RTCTIME time cannot represent days before 1970 so make it a leap year to avoid */
    /* the special case (1900 not a leap year, but 2000 is */
    /* 25568 = (70 * 365) + 18 */
    time += 25568;

    /* Day of week */
    aExpand->tm_wday = time % 7;

    /* Calculate year and day within year */
    year = (time / DAYSIN4YEARS) << 2;
    time %= DAYSIN4YEARS;

    /* If day in year is greater than 365 then adjust so it is 365 or less (0 is Jan 1st) */
    if (time > 365)
    {
        year += ((time - 1) / 365);
        time = (time - 1) % 365;
    }

    aExpand->tm_year = year;

    /* Day of year 1st Jan is 0 */
    aExpand->tm_yday = time;

    /* Not a leap year and is feb 29 or greater */
    /* Then add a day to account for feb has 28, not 29 days */
    /* this is important for the month calculation below */
    if ((year & 3) && (time >= 59))
    {
        time++;
    }

    month = 0;

    /* Remove days in month till left with less or equal days than in current month */
    while (time >= monthInfo[month])
    {
        time -= monthInfo[month];
        month++;
    }

    aExpand->tm_mon = month;

    /* Day of month is what's left, but we want it to start from 1, not 0 */
    aExpand->tm_mday = time + 1;
}

void RTC_LocalTime(RTCTM *aExpand, const RTCTIME *aTimer)
{
    RTC_RawLocalTime(aExpand, *aTimer);
#ifdef RTC_USE_DAYLIGHTSAVING
    /* We don't know if we are in day light saving */
    if (RTC_CheckForDaylightSaving(aExpand))
    {
        aExpand->tm_isdst = 1;
        Rtc_RawLocalTime(aExpand, *aTimer + 60 * 60);
    }
    else
#endif
        aExpand->tm_isdst = 0;
}

void RTC_GetLocalTime(RTCTM *aExpand)
{
    RTC_RawLocalTime(aExpand, _rtc_unixtime);
}

RTCTIME RTC_MkTime(RTCTM *tb)
{
    int32_t tmptm1, tmptm2, tmptm3;

    /* First, make sure tm_year is reasonably close to being in range. */
    if (((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed years *****/

    /* Calculate days elapsed minus one, in the given year, to the given
    * month. Check for leap year and adjust if necessary. */
    tmptm2 = _days[tb->tm_mon];
    if (!(tmptm1 & 3) && (tb->tm_mon > 1))
        tmptm2++;

    /* Calculate elapsed days since base date (midnight, 1/1/70, UTC)
    *
    * 365 days for each elapsed year since 1970, plus one more day for
    * each elapsed leap year. no danger of overflow because of the range
    * check (above) on tmptm1. */
    tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2) - _LEAP_YEAR_ADJUST;

    /* elapsed days to current month (still no possible overflow) */
    tmptm3 += tmptm2;

    /* elapsed days to current date. overflow is now possible. */
    tmptm1 = tmptm3 + (tmptm2 = (long)(tb->tm_mday));
    if (ChkAdd(tmptm1, tmptm3, tmptm2))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed days *****/

    /* Calculate elapsed hours since base date */
    tmptm2 = tmptm1 * 24L;
    if (ChkMul(tmptm2, tmptm1, 24L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_hour);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed hours *****/

    /* Calculate elapsed minutes since base date */

    tmptm2 = tmptm1 * 60L;
    if (ChkMul(tmptm2, tmptm1, 60L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_min);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed minutes *****/

    /* Calculate elapsed seconds since base date */

    tmptm2 = tmptm1 * 60L;
    if (ChkMul(tmptm2, tmptm1, 60L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long)tb->tm_sec);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed seconds *****/

#ifdef RTC_USE_DAYLIGHTSAVING
    /* Finally, adjust for DST */
    switch (tb->tm_isdst)
    {
    case 1:
        tmptm1 -= 60 * 60;
        break;
    case 0:
        break;
    default:
        if (RTC_CheckForDaylightSaving(tb)) tmptm1 -= 60 * 60;
        break;
    }
#endif

    return (RTCTIME)tmptm1;

err_mktime:
    /* All errors come to here */
    return (RTCTIME)(-1);
}


/*********************************************************
RTC_CheckForDaylightSaving

Test a time value for daylight saving period

Ref:  The Eighth European Parliament and Council Directive
        on Summer Time Arrangements

Args:     RTCTM aTime   -   time value

Returns:  true if dst should apply
            false if dst is not in effect
********************************************************/
#ifdef RTC_USE_DAYLIGHTSAVING
bool RTC_CheckForDaylightSaving(const RTCTM *aTime)
{
    /*  Useful basis in the following:
            7 - wday = days until next Sunday
            mday + (7 - wday) = date of next Sunday -> if >31, its next month */

    /* we need to check for >1am, last Sun March &&  <1am last Sun Oct */

    /* check for months Apr to Sep */
    if (aTime->tm_mon > 2 && aTime->tm_mon < 9)
        return true;

    /* check for Mar */
    else if (2 == aTime->tm_mon)
    {
        /* we need to check for the last Sun in the month & after 1am */

        if (0 == aTime->tm_wday && 31 < aTime->tm_mday + 7)  /* last sunday */
            return (aTime->tm_hour >= 1);
        else if (31 < aTime->tm_mday + 7 - aTime->tm_wday)     /* after last sunday */
            return true;       /* add one hour */
    }

    /* check for Oct */
    else if (9 == aTime->tm_mon)
    {
        /* we need to check for the last Sun in the month & before 1am */
        if (0 == aTime->tm_wday && 31 < aTime->tm_mday + 7)  /* last sunday */
            return (aTime->tm_hour < 1);
#ifndef PHP_602_DST_END_WRONG
        else if (31 > aTime->tm_mday + 7 - aTime->tm_wday)   /* before last sunday */
#else
        else if (31 >= aTime->tm_mday + 7 - aTime->tm_wday)   /* before last sunday */
#endif
        {
            return true;       /* add one hour */
        }
    }
    return false;
}
#endif


void RTC_TickUnixTime(void)
{
    _rtc_unixtime++;
}

RTCTIME RTC_GetUnixTime(void)
{
    return _rtc_unixtime;
}

void RTC_SetUnixTime(RTCTIME value)
{
    _rtc_unixtime = value;
}

