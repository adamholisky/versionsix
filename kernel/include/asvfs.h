#if !defined(ASVFS_INCLUDED)
#define ASVFS_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef VIOS
#include <bootstrap.h>
#include <debug.h>
#include <fs.h>
#include <vfs.h>
#endif
/* #include <sys/stat.h>
#include <dirent.h> */
//#include <syslog.h>
/* #include <fcntl.h> */


/* 

Disk layout
--------------

0		Drive header (asvfs_drive)
x		Block meta data
n		Free blocks

Where:
x = sizeof(asvfs_drive);
n = sizeof(asvfs_drive) + (sizeof(asvfs_block_meta_data) * asvfs_drive.block_count)

Block Calculation

1 GiB drive
	1073741824 bytes
	4096 bytes/block
	262,144 blocks
	14417920 bytes of block meta data

*/

#define ASVFS_VERSION_1 2

#define ASVFS_DEFAULT_BLOCK_SIZE 4096
#define ASVFS_MAX_NAME_SIZE 50

#define ASVFS_BLOCK_TYPE_UNKNOWN 0
#define ASVFS_BLOCK_TYPE_FILE 1
#define ASVFS_BLOCK_TYPE_DIRECTORY 2
#define ASVFS_BLOCK_TYPE_META 3
#define ASVFS_BLOCK_TYPE_SYSTEM 4
#define ASVFS_BLOCK_TYPE_NOT_SET 5

#define ASVFS_INODE_TYPE_FILE 1
#define ASVFS_INODE_TYPE_DIR 2
#define ASVFS_INODE_TYPE_DEVICE 3
#define ASVFS_INODE_TYPE_PROC 4
#define ASVFS_INODE_TYPE_LINK 5

#define ASVFS_ERROR_NONE 0
#define ASVFS_ERROR_UNKNOWN -1
#define ASVFS_ERROR_MEMORY -2
#define ASVFS_ERROR_PATH_NOT_FOUND -3
#define ASVFS_ERROR_OBJECT_ALREADY_IN_USE -4
#define ASVFS_ERROR_NOT_A_DIRECTORY -5
#define ASVFS_ERROR_NOT_A_FILE -6
#define ASVFS_ERROR_UNKNOWN_FS -7
#define ASVFS_ERROR_FILE_NOT_FOUND -8
#define ASVFS_ERROR_NOT_A_DEVICE -9
#define ASVFS_ERROR_ALREADY_EXISTS -10

#define ASVFS_OPEN_READ_ONLY 0
#define ASVFS_OPEN_WRITE_ONLY 1
#define ASVFS_OPEN_READ_WRITE 2
#ifndef O_CREAT
	#define O_CREAT 0100
#endif 

// From sys/stat.h
#define	S_IRUSR	__S_IREAD	/* Read by owner.  */
#define	S_IWUSR	__S_IWRITE	/* Write by owner.  */
#define	S_IXUSR	__S_IEXEC	/* Execute by owner.  */
#define	S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)
#define	S_IRGRP	(S_IRUSR >> 3)	/* Read by group.  */
#define	S_IWGRP	(S_IWUSR >> 3)	/* Write by group.  */
#define	S_IXGRP	(S_IXUSR >> 3)	/* Execute by group.  */
#define	S_IRWXG	(S_IRWXU >> 3)
#define	S_IROTH	(S_IRGRP >> 3)	/* Read by others.  */
#define	S_IWOTH	(S_IWGRP >> 3)	/* Write by others.  */
#define	S_IXOTH	(S_IXGRP >> 3)	/* Execute by others.  */
#define	S_IRWXO	(S_IRWXG >> 3)

#define ASVFS_DEFAULT_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH

/* From fcntl-linux.h
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
 */

#ifdef VIOS
	#include <kmemory.h>

	#define asvfs_stderr vios_stderr
	#define asvfs_fprintf vios_fprintf
	#define asvfs_malloc kmalloc
	#define asvfs_realloc krealloc
	#define asvfs_free kfree
	#define asvfs_strlen kstrlen
	#define asvfs_strcmp kstrcmp

	#define asvfs_log(...) klog( LOG_DEBUG, __VA_ARGS__ )
#else
	#define asvfs_stderr stderr
	#define asvfs_fprintf fprintf
	#define asvfs_malloc malloc
	#define asvfs_realloc realloc
	#define asvfs_free free
	#define asvfs_strlen strlen
	#define asvfs_strcmp strcmp
#endif

#ifndef VIOS
	#ifdef ASVFS_DEBUG
		#define asvfs_log printf
	#else
		#define asvfs_log(...) syslog(LOG_DEBUG, __VA_ARGS__)
	#endif
#endif

typedef struct {
	char		magic[4];		// "AFS "
	uint8_t 	version;		// version of the drive struct
	uint64_t 	size;			// overall size of the drive, in bytes
	uint32_t	block_size;		// Size of blocks
	uint32_t	block_count;	// Number of blocks in the dirve
	uint32_t	root_directory;	// starting block of the dir struct for root
	uint32_t	next_free;		// next free block
	uint32_t	reserved_1;
	uint32_t	reserved_2;
	uint32_t	reserved_3;
	uint32_t	reserved_4;
	uint32_t	reserved_5;
	uint32_t	reserved_6;
	uint32_t	reserved_7;
	uint32_t	reserved_8;
} __attribute__((packed)) asvfs_header;

typedef struct {
	uint32_t	id;				// unique block id (aka: inode)
	uint8_t		block_type;		// type of data the block holds
	char		name[ASVFS_MAX_NAME_SIZE];		// name
	uint32_t	file_size;
	uint32_t	starting_block;
	uint32_t	num_blocks;
	bool		in_use;
	uint16_t	permissions;
	uint16_t	reserved_2;
	uint32_t	reserved_3;
	uint32_t	reserved_4;
	uint32_t	reserved_5;
	uint32_t	reserved_6;
	uint32_t	reserved_7;
	uint32_t	reserved_8;
} __attribute__((packed)) asvfs_block_meta_data;

typedef struct {
	uint8_t		data[ ASVFS_DEFAULT_BLOCK_SIZE ];
} __attribute__((packed)) asvfs_generic_block;

typedef struct {
	uint8_t 	data[ ASVFS_DEFAULT_BLOCK_SIZE ];
} __attribute__((packed)) asvfs_file;

typedef struct {
	uint32_t 	type;			// Type, always ASVFS_BLOCK_TYPE_DIRECTORY
	uint32_t	index[256];		// Block index for things in this directory
	uint32_t	next_index;		// next index free
	uint32_t	reserved_1;
	uint32_t	reserved_2;
	uint32_t	reserved_3;
	uint32_t	reserved_4;
	uint32_t	reserved_5;
	uint32_t	reserved_6;
	uint32_t	reserved_7;
	uint32_t	reserved_8;
} __attribute__((packed)) asvfs_block_directory;

typedef uint64_t anode_id;

typedef struct {
	anode_id vfs_id;
	uint32_t block_id;
	bool open;
	void *next;
} asvfs_inode;

typedef struct {
	char name[ASVFS_MAX_NAME_SIZE];
	uint8_t type;
	uint32_t block_id;
} asvfs_dir_t_entry;

typedef struct {
	int8_t size;
	asvfs_dir_t_entry dir_entry[100];
} asvfs_dir_t_entries;

typedef struct {
	int (*read)(const char *, char *, size_t, off_t);
	int (*read_from_disk)(uint64_t, uint8_t *, uint64_t, uint64_t );
	int (*write_to_disk)(uint32_t, off_t, size_t, char *); 
} asvfs_drive_ops;

#define asvfs_drive_read( drive, data, size, offset ) asvfs_instance_data.drive_ops.read_from_disk( drive, data, size, offset )
//#define asvfs_drive_write( block_id, size, data ) asvfs_instance.drive_ops.write_to_disk( block_id, size, data )

typedef struct {
	asvfs_drive_ops drive_ops;
	asvfs_header *header;
} asvfs_instance;

int asvfs_initalize( asvfs_drive_ops *ops );
int asvfs_open( char *pathname, int flags, mode_t mode );
int asvfs_getattr( char *pathname, struct stat *stbuff );
int asvfs_read( char *pathname, char *buf, size_t size, off_t offset );
int asvfs_read_file_block_id( uint32_t block_id, char *buf, off_t offset, size_t length );
int asvfs_read_block( uint32_t block_id, size_t size, char *data ); // TODO: Update param order
int asvfs_readdir(uint32_t block_id, asvfs_dir_t_entries *dir_entries);
int asvfs_get_meta_data(uint32_t block_id, asvfs_block_meta_data *data);
uint32_t asvfs_get_block_id_from_pathname( char *pathname );
int asvfs_get_block_meta_from_dir( uint32_t dir_block_id, char *dir_entry_name );
int asvfs_write_block( uint32_t block_id, uint32_t size, char *data );
int asvfs_write_meta_data( uint32_t block_id, asvfs_block_meta_data *meta );
int asvfs_write_header( asvfs_header *h );
int asvfs_write( char *pathname, char *buff, off_t offset, size_t size );
int asvfs_write_file_block_id( uint32_t block_id, char *buff, off_t offset, size_t size );
int asvfs_create( char *pathname, int mode );
int asvfs_chmod( char *pathname, int mode );
int asvfs_get_path_parts( char *pathname, char *parent, char *child );
int asvfs_mkdir( char *pathname, int mode, int rdev );
int asvfs_mknod( char *pathname, int mode, int rdev );
int asvfs_truncate( char *pathname, off_t length );
int asvfs_truncate_file_block_id( uint32_t block_id, off_t length );
asvfs_header *asvfs_get_header( void );
int asvfs_unlink( char *pathname );
int asvfs_rename( char *from, char *to );

#ifdef __cplusplus
}
#endif

#endif
