/*
* Time definitions and prototypes for Real Time Clock API
*/

#ifndef RTC_H
#define RTC_H


/* The Base year is the value to add to the RTCTM tm_year field to get the actual year */
#define RTC_BASEYEAR 1900

/* Maximum time. */
#define RTC_MAXTIME 0x7fffffff

/* Expanded time. Note that this is not called tm so that there is no conflict if a standard C library is available.*/
typedef struct s_tm
{
    /* seconds after the minute: 0 - 59 */
    uint8_t tm_sec;
    /* minutes after the hour: 0 - 59 */
    uint8_t tm_min;
    /* hours since midnight: 0 - 23 */
    uint8_t tm_hour;
    /* day of the month: 1 - 31 */
    uint8_t tm_mday;
    /* months since January: 0 - 11 */
    uint8_t tm_mon;
    /* years since 1900 */
    uint8_t tm_year;
    /* days since Sunday: 0 - 6 */
    uint8_t tm_wday;
    /* days since Jan 1st: 0 - 365 */
    uint16_t tm_yday;
    /* daylight savings flag: -1 unknown, 0 not in DST, 1 in DST */
    int8_t tm_isdst;
    /* time reported is valid flag */
    uint8_t tm_isvalid;
} RTCTM;

typedef uint32_t RTCTIME;   /* Number of seconds since 1st January 1970 */
//typedef struct s_tm RTCTM;    /* Expanded time structure, containing time, date, and day of week */


/***********************
*  Function Prototypes *
***********************/

void rtc_Time(RTCTIME *aTimer);

/**Setting the current real time.
Sets the current real time.
@param aTimer Pointer to the real time.
@return None
*/
void rtc_SetTime(const RTCTIME *aTimer);

/**Converting real time to separate fields.
Converts the specified real time into expanded fields.
@param aExpand Pointer to expand the real time into
@param aTimer Pointer to a real time value. If the real time represents an invalid time, the expanded time will be 0:00:00 1/1/1900
@return None
*/
void RTC_LocalTime(RTCTM *aExpand, const RTCTIME *aTimer);
void RTC_RawLocalTime(RTCTM *aExpand, RTCTIME time);
void RTC_GetLocalTime(RTCTM *aExpand);
RTCTIME RTC_GetUnixTime(void);
void RTC_SetUnixTime(RTCTIME value);

/**Converting Local Time to Real Time.
Converts local time (as returned by Rtc_LocalTime) into RTCTIME
@param aTime Local time
@rturn (RTCTIME) time equivalent of Local Time (RTCTM)
*/
RTCTIME RTC_MkTime(RTCTM *aTime);

/**Test for trusted time.
Checks if the current time is trusted
@return TRUE if trusted, FALSE if not
*/
bool RTC_TimeTrusted(void);

void RTC_TickUnixTime(void);

#endif /* !RTC_H */
