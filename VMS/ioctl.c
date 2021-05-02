
#include <stdio.h>
#include <iodef.h>
#include <errno.h>
#include <ucx$inetdef.h>

/*
* Functional Description
*
* Formal Parameters
*        d...file or socket descriptor
*        request...defined in ioctl.h
*        argp..."in" or "out" parameter
*
*
* Routine Value
*
*        Status code
*/



/*
*
*        MACRO DEFINITIONS
*
*/
#ifndef _IO
#define IOCPARM_MASK    0x7f            /* Parameters are < 128 bytes */
#define IOC_VOID        (int)0x20000000 /* No parameters              */
#define IOC_OUT         (int)0x40000000 /* Copy out parameters        */
#define IOC_IN          (int)0x80000000 /* Copy in parameters         */
#define IOC_INOUT       (int)(IOC_IN|IOC_OUT)
#define _IO(x,y)        (int)(IOC_VOID|('x'<<8)|y)
#define _IOR(x,y,t)     (int)(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define _IOw(x,y,t)     (int)(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#endif /* _IO */

#define VMSOK(s) (s & 01)

int ioctl(int d, int request, char *argp)
{
	int ef;                       /* Event flag number */
	int sdc;                      /* Socket device channel */
	unsigned short fun;           /* Qiow function code */
	unsigned short iosb[4];       /* Io status block */
	void *p5, *p6;                /* Args p5 & p6 of qiow */
	struct comm
	{
		int command;
		char *addr;
	} ioctl_comm;                 /* Qiow ioctl commands. */
	struct it2
	{
		unsigned short len;
		unsigned short opt;
		struct comm *addr;
	} ioctl_desc;                 /* Qiow ioctl commands descriptor */
	int status;

	/*
	 * Gets an event flag for qio
	 */
	status = LIB$GET_EF(&ef);
	if (!VMSOK(status))
	{
		/* No er available. Use 0 */
		ef = 0;
	}

	/*
	 * Get the socket device channel number.
	 */
	sdc = vaxc$get_sdc(d);
	if (sdc == 0)
	{
		/* Not an open socket descriptor. */
		errno = EBADF;
		return -1;
	}

	/*
	 * Fill in ioctl descriptor.
	 */
	ioctl_desc.opt = UCX$C_IOCTL;
	ioctl_desc.len = sizeof(struct comm);
	ioctl_desc.addr = &ioctl_comm;

	/*
	 * Decide qio function code and in/out parameter
	 */
	if (request & IOC_OUT)
	{
		fun = IO$_SENSEMODE;
		p5 = 0;
		p6 = &ioctl_desc;
	}
	else
	{
		fun = IO$_SETMODE;
		p5 = &ioctl_desc;
		p6 = 0;
	}

	/*
	 * Fill in ioctl command.
	 */
	ioctl_comm.command = request;
	ioctl_comm.addr = argp;

	/*
	 * Do ioctl
	 */
	status = SYS$QIOW(ef, sdc, fun, iosb, 0, 0,
	                  0, 0, 0, 0,                 /* p1 - p4: not used */
	                  p5, p6);
	if (!VMSOK(status))
	{
		printf("ioctl failed: status = %d\n", status);
		errno = status;
		return -1;
	}

	if (!VMSOK(iosb[0]))
	{
		printf("ioctl failed: status = %x, %x, %x%x\n", iosb[0], iosb[1],
		       iosb[3], iosb[2]);
		errno = iosb[0];
		return -1;
	}

	status = LIB$FREE_EF(&ef);
	return 0;
}

