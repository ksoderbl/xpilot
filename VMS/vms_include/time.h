/*
 *	VAX-11 "C" runtime compatible time definitions
 */

#ifndef __GCC_TIME
#define __GCC_TIME

struct tm {
	int	tm_sec;
	int	tm_min;
	int	tm_hour;
	int	tm_mday;
	int	tm_mon;
	int	tm_year;
	int	tm_wday;
	int	tm_yday;
	int	tm_isdst;
};        	

extern struct tm *localtime();

#endif

