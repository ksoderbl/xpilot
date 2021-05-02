/*
 *	VAX-11 "C" runtime compatible stat definition
 */

#ifndef	_STAT_
#define	_STAT_

/*
 *	Because of alignment differences between VAX-11 "C" and GNU "C"
 *	we can't normally access any field AFTER st_gid.  To fix this
 *	the macro stat_alignment_fix(statp) makes fields AFTER st_gid
 *	work right.  stat_alignment_fix2(statp) allows st_fab_mrs to be
 *	accessed.
 *
 *	VMS/GCC defines the CPP variable "PCC_ALIGNMENT" if UNIX PCC
 *	style alignment is in effect.  Otherwise, VAX-11 "C" style
 *	alignment is in effect.
 */
#ifdef	PCC_ALIGNMENT
#define	stat_alignment_fix(statp) \
	((struct stat *)((char *)statp - sizeof(short)))

#define	stat_alignment_fix1(statp) \
	((struct stat *)((char *)statp - sizeof(short) - sizeof(char)))
#endif	PCC_ALIGNMENT

/*
 *	If <sys/types> has not been included we need to define the following:
 *	off_t, ino_t, dev_t
 */
#ifndef	_TYPES_
#include <sys/types.h>
#endif	_TYPES_

struct  stat
{
        dev_t   st_dev;
        ino_t   st_ino[3];
        unsigned short st_mode;
        int	st_nlink;
        unsigned st_uid;
        unsigned short st_gid;	/* <---- The big screw...  VAX-11 "C" doesn't
					do alignment so everything following
					this is wrong! */
        dev_t	st_rdev;
        off_t   st_size;
        unsigned st_atime;
        unsigned st_mtime;
        unsigned st_ctime;
	char	st_fab_rfm;
	char	st_fab_rat;
	char	st_fab_fsz;	/* <---- Another screw */
	unsigned st_fab_mrs;
};

#define S_IFMT   0170000         /* type of file */
#define          S_IFDIR 0040000 /* directory */
#define          S_IFCHR 0020000 /* character special */
#define          S_IFBLK 0060000 /* block special */
#define          S_IFREG 0100000 /* regular */
#define          S_IFMPC 0030000 /* multiplexed char special */
#define          S_IFMPB 0070000 /* multiplexed block special */
#define S_ISUID  0004000         /* set user id on execution */
#define S_ISGID  0002000         /* set group id on execution */
#define S_ISVTX  0001000         /* save swapped text even after use */
#define S_IREAD  0000400         /* read permission, owner */
#define S_IWRITE 0000200         /* write permission, owner */
#define S_IEXEC  0000100         /* execute/search permission, owner */

#endif	_STAT_
