#ifndef VIOS_VIT_INCLUDED
#define VIOS_VIT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/**************************************/
/* Header Config                      */
/**************************************/
#include <stdint.h>
#include <stdbool.h>

#ifdef VI_ENV_OS
	#include <kernel_common.h>
	#include <vfs.h>
	#include <fs.h>
#else
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <errno.h>
#endif

#include "lib/bitmap.h"
#include "vui/vui.h"

/**************************************/
/* System API Macro Config            */
/**************************************/

#ifdef VI_ENV_OS
	#define vmalloc kmalloc
	#define vdebugf debugf
	#define vdf debugf
#else
	#define vmalloc malloc
	#define vdebugf printf
	#define vdf printf
#endif

/**************************************/
/* Functions                          */
/**************************************/

#ifdef VI_ENV_OS
	#define vit_gui_test ;
	#define vit_gui_update ;
#else
	void vit_gui_test( void );
	void vit_gui_update( void );
#endif


#ifdef __cplusplus
}
#endif
#endif