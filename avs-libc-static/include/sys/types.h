#ifndef SYS_TYPES_H__
#define SYS_TYPES_H__

#include <stddef.h>
#include <_types/_ptrdiff_t.h>
#include <_types/_size_t.h>
#include <_types/_uint16_t.h>
#include <_types/_uint32_t.h>

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define _IOFBF 0
#define _IOLBF 1
#define _IONBF 2

#define BUFSIZ 1024
#define FILENAME_MAX 4096
#define FOPEN_MAX 1000
#define TMP_MAX 10000
#define L_tmpnam 20

#define STDERR_FILENO 2
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

#undef EOF
#define EOF (-1)

typedef int __dev_t;
typedef uint32_t __ino_t;
typedef int __nlink_t;
typedef int __uid_t;
typedef int __gid_t;
typedef int __blksize_t;
typedef uint32_t __off_t;
typedef int __blkcnt_t;
//typedef uint32_t __time_t;
typedef uint32_t __syscall_ulong_t;

#define dev_t __dev_t
#define off_t __off_t
#define ino_t __ino_t
#define nlink_t __nlink_t
#define uid_t __uid_t
#define gid_t __gid_t
#define blksize_t __blksize_t
#define blkcnt_t __blkcnt_t
//#define time_t __time_t
#define syscall_ulong_t __syscall_ulong_t

#endif