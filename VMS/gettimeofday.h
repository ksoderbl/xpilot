#ifndef GET_TIME_OF_DAY
#define GET_TIME_OF_DAY

#ifndef __SOCKET_LOADED
struct timeval {
     long tv_sec;        /* seconds since Jan. 1, 1970 */
     long tv_usec;       /* and microseconds */
};
#endif


struct timezone {
     int  tz_minuteswest;     /* of Greenwich */
     int  tz_dsttime;    /* type of dst correction to apply */
};

int gettimeofday(struct timeval *tp, struct timezone *tzp);

#endif

