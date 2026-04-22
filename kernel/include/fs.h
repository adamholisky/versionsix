#if !defined(FS_INCLUDED)
#define FS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <unistd.h>

typedef uint32_t mode_t;
typedef mode_t __mode_t;
typedef int __dev_t;
typedef uint32_t __ino_t;
typedef int __nlink_t;
typedef int __uid_t;
typedef int __gid_t;
typedef int __blksize_t;
typedef uint32_t __off_t;
typedef int __blkcnt_t;
typedef uint32_t __time_t;
typedef uint32_t __syscall_ulong_t;

#define dev_t __dev_t
#define off_t __off_t
#define ino_t __ino_t
#define nlink_t __nlink_t
#define uid_t __uid_t
#define gid_t __gid_t
#define blksize_t __blksize_t
#define blkcnt_t __blkcnt_t
#define time_t __time_t
#define syscall_ulong_t __syscall_ulong_t

/* From linux kernel/x86_64 linux gnu/bits/stats.h File types.  */
#define	S_IFDIR	0040000	/* Directory.  */
#define	S_IFCHR	0020000	/* Character device.  */
#define	S_IFBLK	0060000	/* Block device.  */
#define	S_IFREG	0100000	/* Regular file.  */
#define	S_IFIFO	0010000	/* FIFO.  */
#define	S_IFLNK	0120000	/* Symbolic link.  */
#define	S_IFSOCK	0140000	/* Socket.  */

struct stat {
	dev_t st_dev;		/* Device.  */
	ino_t st_ino;		/* File serial number.	*/
	mode_t st_mode;			/* File mode.  */
	nlink_t st_nlink;			/* Link count.  */
	uid_t st_uid;		/* User ID of the file's owner.	*/
	gid_t st_gid;		/* Group ID of the file's group.*/
	dev_t st_rdev;		/* Device number, if device.  */
	off_t st_size;			/* Size of file, in bytes.  */
	blksize_t st_blksize;	/* Optimal block size for I/O.  */
	blkcnt_t st_blocks;		/* Number 512-byte blocks allocated. */
	time_t st_atime;			/* Time of last access.  */
	syscall_ulong_t st_atimensec;	/* Nscecs of last access.  */
	time_t st_mtime;			/* Time of last modification.  */
	syscall_ulong_t st_mtimensec;	/* Nsecs of last modification.  */
	time_t st_ctime;			/* Time of last status change.  */
	syscall_ulong_t st_ctimensec;	/* Nsecs of last status change.  */

	uint8_t type;
};

typedef struct stat file_stats;

void fs_initalize_part1( void );
void fs_initalize_part2( void );

#ifdef __cplusplus
}
#endif

#endif