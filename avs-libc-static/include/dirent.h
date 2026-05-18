#ifndef DIRENT_H_
#define DIRENT_H_

#ifndef __cplusplus

#include <sys/types.h>

struct dirent {
	ino_t d_ino;
	off_t d_off;
	unsigned short d_reclen;
	unsigned char d_type;
	char d_name[256];
};

struct dirstream
{
	off_t tell;
	int fd;
	int buf_pos;
	int buf_end;
	volatile int lock[1];
	/* Any changes to this struct must preserve the property:
	 * offsetof(struct __dirent, buf) % sizeof(off_t) == 0 */
	char buf[2048];
};

typedef struct dirstream DIR;


#endif

#endif 