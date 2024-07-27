#include <kernel_common.h>
#include <kshell_app.h>
#include <fs.h>
#include <afs.h>

KSHELL_COMMAND( afsdd, kshell_app_afsdd_main )

int kshell_app_afsdd_main( int argc, char *argv[] ) {
    afs_dump_diagnostic_data();
    vfs_cache_diagnostic();

	return 0;
}