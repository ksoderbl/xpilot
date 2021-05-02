#include <lib$routines.h>
#include <jpidef.h>
#include <ctype.h>
#include "gettimeofday.h"


int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    char  date_time[8];
    /*
     * structure of timebuf
     *
     * timebuf[0]   year since 0
     * timebuf[1]   month of year
     * timebuf[2]   day of month
     * timebuf[3]   hour of day
     * timebuf[4]   minute of hour
     * timebuf[5]   second of minute
     * timebuf[6]   hundredths of second
     */
    short timebuf[7];
    int   i;

#define SECS_PER_YEAR   (60*60*24*366)
#define SECS_PER_MONTH  (60*60*24*31)
#define SECS_PER_DAY    (60*60*24)
#define SECS_PER_HOUR   (60*60)
#define SECS_PER_MINUTE (60)

    SYS$NUMTIM(timebuf, 0);

    /* this isn't right but it is close enough for timing purposes */

    tp->tv_sec = (timebuf[0]-1970) * SECS_PER_YEAR +
                 (timebuf[1]-1)    * SECS_PER_MONTH +
                 (timebuf[2]-1)    * SECS_PER_DAY +
                  timebuf[3]       * SECS_PER_HOUR +
                  timebuf[4]       * SECS_PER_MINUTE +
                  timebuf[5]; 

    tp->tv_usec = timebuf[6]*10000;

    return 0;
}
