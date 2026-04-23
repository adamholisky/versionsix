#include <kernel_common.h>
#include <kmemory.h>
#include <process.h>
#include <lib/list.h>
#include <vfs.h>
#include <elf_loader.h>
#include <elf.h>
#include <page.h>

avs_list *process_list;
pid_t pid_next;
process_data *root_pd;

process_data *current_process;

void process_initalize( void ) {
	pid_next = 0;

	process_list = avs_list_init();

	// Setup initial process, which is just the idle loop

	pid_t root_proc_pid = process_create();
	if ( root_proc_pid != ROOT_PROCESS_ID ) {
		klog( LOG_PANIC, "Root pid == %d, should be %d", root_proc_pid, ROOT_PROCESS_ID );
		do_immediate_shutdown();
	}

	root_pd = (process_data*)process_list->head->data;
	if ( root_pd->pid != ROOT_PROCESS_ID ) {
		klog( LOG_PANIC, "Manual root process find failed. pid == %d, should be %d", root_pd->pid, ROOT_PROCESS_ID );
		do_immediate_shutdown();
	}

	strcpy( root_pd->path, "/bin/first.exec" );

	file_stats fstats;
	vfs_getattr( root_pd->path, &fstats );
	root_pd->exec_size = fstats.st_size;

	uint8_t* buff = kmalloc( root_pd->exec_size );
	int bytes_read = vfs_read( root_pd->path, buff, root_pd->exec_size, 0 );

	elf_loader_load( root_pd, buff );

	for( int i = 0; i < root_pd->data_section_count; i++ ) {
		page_map( root_pd->data_sections[i].virt, root_pd->data_sections[i].phys );
	}

	for( int i = 0; i < root_pd->text_section_count; i++ ) {
		page_map( root_pd->text_sections[i].virt, root_pd->text_sections[i].phys );
	}

	kdebug_peek_at_n( root_pd->text_sections[0].virt, 200 );

	kdebug_peek_at_n( root_pd->data_sections[0].virt, 250 );
}

void process_start_root_process( void ) {
	current_process = root_pd;

	int (*f)(int, char *) = root_pd->entry;
	f( 0, NULL );

	debugf( "Ending very happy!\n" );

	do_immediate_shutdown();
}

pid_t process_create( void ) {
	process_data* p = kmalloc( sizeof( process_data ) );

	p->pid = pid_next++;

	avs_list_append( process_list, p );

	return p->pid;
}

process_data *process_get_current( void ) {
	return current_process;
}

void process_destroy( pid_t pid ) {

}

int process_idle_loop_entry( int argc, char* argv[] ) {
	process_idle_loop();
}

void process_idle_loop( void ) {
	static int i = 0;

	do {
		i++; // Keep track of how many times we loop
	} while ( 1 );
}

void process_env_setup( void ) {
	klog( LOG_INFO, "in setup for pid: %d", current_process->pid );
}

void process_exit( int ret_code ) {
	klog( LOG_INFO, "in exit for pid %d: ret_code=%d", current_process->pid, ret_code );
	printf( "DONE\n" );

	do_immediate_shutdown();
}

pid_t vios_fork( void ) {

}
