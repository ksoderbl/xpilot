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
#define SECS_PER_DAY (60*60*24)

/*    sys$gettim(&date_time);*/
    SYS$NUMTIM(timebuf, 0);
    tp->tv_usec = timebuf[6]*10000;
    /* this isn't right but it is close enough for timing purposes */
    tp->tv_sec = (timebuf[0]-1970) * (SECS_PER_DAY*366) +
                 (timebuf[1]-1) * (SECS_PER_DAY*31) +
                 (timebuf[2]-1) * SECS_PER_DAY +
                  timebuf[3] * (60*60) +
                  timebuf[4] * 60 +
                  timebuf[5];
    return 0;
}
