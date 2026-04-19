#if !defined(RFS_V2_INCLUDED)
#define RFS_V2_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "vfs.h"

#ifndef VIFS_OS
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>
#endif

#define RFS_FILE_TYPE_DIR 0
#define RFS_FILE_TYPE_FILE 1
#define RFS_FILE_TYPE_LINK 2
#define RFS_FILE_TYPE_DEVICE 3

typedef struct {
    inode_id vfs_inode_id;
    inode_id vfs_parent_inode_id;

    uint8_t rfs_file_type;
    char pathname[1024];
    char name[VFS_NAME_MAX];
    uint8_t *data;
    uint64_t size;

    void *dir_list;
} rfs_file;

typedef struct {
    void *next;
    rfs_file *file;
} rfs_file_list_el;

typedef struct {
    rfs_file_list_el *head;
    rfs_file_list_el *tail;
    uint64_t count;
} rfs_file_list;

typedef struct {
    char path[255];
    inode_id id;
    uint8_t fs_id;

    rfs_file_list file_list;

    void *next;
} rfs_mounted;

int rfs_initalize( void );
rfs_file* rfs_lookup_by_pathname( char *pathname );
int rfs_avs_list_compare_file_pathnames( void *a, void *b );
int rfs_open(  char *pathname, int flags, mode_t mode );
int rfs_getattr( char *pathname, struct stat *stbuff );
int rfs_create( char *pathname, int mode );
int rfs_read( char *pathname, char *buf, off_t offset, size_t length );
int rfs_write( char *pathname, char *buff, off_t offset, size_t size );
vfs_directory_list *rfs_dir_list( inode_id id, vfs_directory_list *list );

#ifdef __cplusplus
}
#endif

#endif