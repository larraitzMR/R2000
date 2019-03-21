/*
 *****************************************************************************
 *                                                                           *
 *                 IMPINJ CONFIDENTIAL AND PROPRIETARY                       *
 *                                                                           *
 * This source code is the sole property of Impinj, Inc.  Reproduction or    *
 * utilization of this source code in whole or in part is forbidden without  *
 * the prior written consent of Impinj, Inc.                                 *
 *                                                                           *
 * (c) Copyright Impinj, Inc. 2009. All rights reserved.                     *
 *                                                                           *
 *****************************************************************************
 */

/*
 *****************************************************************************
 *
 * $Id: compat_time.h 59725 2009-09-03 20:23:46Z dshaheen $
 * 
 * Description: Describes the portable structure for 64-bit times and its
 *     associated functions.
 *
 *****************************************************************************
 */

#ifndef COMPAT_TIME_
#define COMPAT_TIME_

#include <windows.h>

#include "compat_lib.h"

/****************************************************************************
 * Name:        struct CPL_TimeSpec
 * Description: Contains a timestamp in 64 bits with as much precision as
 *              possible.
 ****************************************************************************/
struct CPL_TimeSpec {
    INT32S seconds;     /* Seconds since midnight GMT, January 1, 1970 */
    INT32S nanoseconds; /* Nanoseconds */
};

/****************************************************************************
 * Name:        struct CPL_CalendarTime
 * Description: Contains a timestamp in a human-friendly format.
 ****************************************************************************/
struct CPL_CalendarTime {
    INT32U year;      /* 2006 would be a normal value,
                       * NOT the 106 that gmtime() would return */
    INT32U month;     /* The month: 1 is January, 12 is December */
    INT32U day;       /* Day of the month:        1-31 */
    INT32U hour;      /* Hour on a 24-hour clock: 0-23 */
    INT32U minute;    /* The minute hand:         0-59 */
    INT32U second;    /* The second hand:         0-59 */
    INT32U dayOfYear; /* 0 is January 1; 365 is December 31 in a leap year */
    INT32U dayOfWeek; /* 0 is Sunday; 6 is Saturday */
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 *  Function:    CPL_TimeSpecGet
 *  Description: Returns the current system time.
 *  Parameters:  tp          [OUT] The location to store the current time.
 *  Returns:     Zero for success, or a CPL_GetError() code on failure.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_TimeSpecGet(struct CPL_TimeSpec *tp);

/****************************************************************************
 *  Function:    CPL_TimeSpecCmp
 *  Description: Compares two time structures to see which is later.
 *  Parameters:  timeSpecA   [IN] The first of the two timestamps to compare.
 *               timeSpecB   [IN] The timestamp to compare timeSpecA to.
 *  Returns:     Less than zero, zero, or greater than zero if timeSpecA is
 *               found to be earlier than, identical to, or later than
 *               timeSpecB, respectively.
 ****************************************************************************/
CPL_EXPORT int CPL_TimeSpecCmp(const struct CPL_TimeSpec *timeSpecA,
                               const struct CPL_TimeSpec *timeSpecB);

/****************************************************************************
 *  Function:    CPL_TimeSpecSum
 *  Description: Adds one time structure to another in place.
 *  Parameters:
 *    timeSpecToAddTo [IN/OUT] The time value that will be modified in place.
 *    timeSpecToAdd   [IN]     The timestamp to add to the preceding argument.
 *  Returns:     None.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_TimeSpecSum(struct CPL_TimeSpec       *timeSpecToAddTo,
                                  const struct CPL_TimeSpec *timeSpecToAdd);

/****************************************************************************
 *  Function:    CPL_TimeSpecDiff
 *  Description: Subtracts one timestamp from another in place.
 *  Parameters:
 *    timeSpecSubFrom  [IN/OUT] The timestamp that will be modified.
 *    timeSpecToSub        [IN] The timestamp to subtract from arg 1.
 *  Returns:     None.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_TimeSpecDiff(struct CPL_TimeSpec       *timeSpecSubFrom,
                                   const struct CPL_TimeSpec *timeSpecToSub);

/****************************************************************************
 *  Function:    CPL_MillisecondSleep
 *  Description: Causes the active thread to sleep for a specified number of
 *               milliseconds, within the accuracy of the OS's scheduler.
 *  Parameters:  msecs  [IN] The number of milliseconds to sleep.
 *  Returns:     Nothing.
 ****************************************************************************/
inline void CPL_MillisecondSleep(int msecs) {
    Sleep(msecs);
}

/****************************************************************************
 *  Function:    CPL_TimeSpecToCalendarTime
 *  Description: Takes a time in "number of seconds since January 1, 1970"
 *               and converts it to years, months, days, hours, minutes and
 *               seconds for consumption by human beings.
 *  Parameters:  timeSpec  [IN] The time to convert to a human-friendly format.
 *               calTime  [OUT] The resulting months and minutes etc.
 *  Returns:     Nothing.
 ****************************************************************************/
CPL_EXPORT INT32U CPL_TimeSpecToCalendarTime(
                      const struct CPL_TimeSpec *timeSpec,
                      struct CPL_CalendarTime   *calTime);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* COMPAT_TIME_ */
