#include <kernel_common.h>
#include <kmemory.h>
#include <elf_loader.h>
#include <elf.h>
#include <page.h>
#include <vfs.h>

#include <syscall.h>
#include <process.h>

int execve( char *path, char *argv[], char *envp[] ) {
	syscall_args args = {
		.arg_1 = path,
		.arg_2 = argv,
		.arg_3 = envp
	};

	return syscall( SYSCALL_EXECVE, 3, &args );
}

int execm( void *exec_data, char *argv[], char *envp[] ) {
	syscall_args args = {
		.arg_1 = exec_data,
		.arg_2 = argv,
		.arg_3 = envp
	};

	return syscall( SYSCALL_EXECM, 3, &args );
}

int execve_syscall_handler( registers **_context, char *path, char *argv[], char *envp[] ) {
	klog( LOG_DEBUG, "In execve handler. path=\"%s\"", path );

	// Initial sanity checks
	file_stats st;
	int getattr_err = vfs_getattr( path, &st );
	if( getattr_err != VFS_ERROR_NONE ) {
		klog( LOG_ERROR, "Could not open %s", path );
		return -1;
	}

	// Get the current process to overwrite
	pid_t pid = process_get_current_proc_id();
	process_data *p = process_get_current();

	// Read the file
	uint8_t* buff = kmalloc( st.st_size ) ;
	int bytes_read = vfs_read( path, buff, st.st_size, 0 );

	// Setup admin vars
	p->status = PROCESS_STATUS_IN_SETUP;
	p->exec_size = st.st_size;
	strcpy( p->path, path );
	strcpy( p->name, path ); // TODO: basename
	p->argv = argv;
	p->argc = 1;
	strcpy( p->working_dir, "/" );
	p->has_own_addr_space = true;

	// Load ELF information
	int loadelf_err = elf_loader_load( p, buff );

	// Handle argc, argv setup
	int argc = 0;

	//printf( "argv: 0x%016llX\n", argv );

	if( argv != NULL ) {
		for( bool keep_going = true; keep_going == true ; ) {
			if( argv[argc] == 0 ) {
				keep_going = false;
			} else {
				argc++;
			}
		}

		if( argc == 20 ) { argc = 0; }
	}
	

	//printf( "argc: %d\n", argc );

	// reconstruct argv, ensuring it's not located in unaccessible pages to the child process
	char **argv_new = kmalloc( argc * sizeof(char *) );
	
	for( int i = 0; i < argc; i++ ) {
		argv_new[i] = kmalloc( kstrlen(argv[i]) );
		kstrcpy( argv_new[i], argv[i] );

		//printf( "argv_n[%d] = 0x%016llX, \"%s\"\n", i, argv_new[i], argv_new[i] );
	}

	

	// Stack setup
	p->proc_stack_virt = 0x00000000A0000000;
	p->proc_stack_kvirt= page_allocate_kernel( PROCESS_DEFAULT_STACK_PAGES );
	p->proc_stack_phys = paging_virtual_to_physical( p->proc_stack_kvirt );
	p->stack_size = PROCESS_DEFAULT_STACK_SIZE;

	page_map( p->proc_stack_virt, p->proc_stack_phys );
	
	// Code segmnet setup
	for( int i = 0; i < p->text_section_count; i++ ) {
		page_map( p->text_sections[i].virt, p->text_sections[i].phys );
	}

	// Data segment setup
	for( int i = 0; i < p->data_section_count; i++ ) {
		page_map( p->data_sections[i].virt, p->data_sections[i].phys );
	}

	for( int i = 0; i < PROCESS_DEFAULT_STACK_PAGES; i++ ) {
		page_map( p->proc_stack_virt + (i * PAGE_SIZE), p->proc_stack_phys + (i * PAGE_SIZE) );
		//klog( LOG_DEBUG, "[STACK] For pid %d: mapped virt to physical: 0x%016llX -> 0x%016llX", new_p->pid,  new_p->proc_stack_virt + (i * PAGE_SIZE), new_p->proc_stack_phys + (i * PAGE_SIZE) );
	}

	// Context setup
	memset( &p->context, 0, sizeof(registers) );
	p->context.rip = (uint64_t)p->entry;
	p->context.rsp = p->proc_stack_virt + p->stack_size;
	p->context.cs = 0x28;
	p->context.rflags = 0x200;
	p->context.ss = 0x30;
	p->context.rdi = argc;
	p->context.rsi = argv_new;

	// Final setup
	p->status = PROCESS_STATUS_ACTIVE;
	//process_set_next_up( p );
	
	/* printf( "Before: \n" );
	process_diagnostic_context(*_context); */

	memcpy( &(**_context), &p->context, sizeof(registers) );

	/* printf( "After:\n" );
	process_diagnostic_context(*_context); */

	kfree(buff);

	klog( LOG_DEBUG, "Out execve handler." );
}

int execm_syscall_handler( registers **_context, void *exec_data, char *argv[], char *envp[] ) {
	klog( LOG_DEBUG, "In execve handler. data=0x%016llX", exec_data );

	// Get the current process to overwrite
	pid_t pid = process_get_current_proc_id();
	process_data *p = process_get_current();

	// Read the file
	uint8_t *buff = exec_data;

	// Setup admin vars
	p->status = PROCESS_STATUS_IN_SETUP;
	p->exec_size = 0;
	strcpy( p->path, "mem" );
	strcpy( p->name, "mem_exec" ); // TODO: basename
	p->argv = argv;
	p->argc = 1;
	strcpy( p->working_dir, "/" );
	p->has_own_addr_space = true;

	// Load ELF information
	int loadelf_err = elf_loader_load( p, buff );

	// Stack setup
	p->proc_stack_virt = 0x00000000A0000000;
	p->proc_stack_kvirt= page_allocate_kernel( PROCESS_DEFAULT_STACK_PAGES );
	p->proc_stack_phys = paging_virtual_to_physical( p->proc_stack_kvirt );
	p->stack_size = PROCESS_DEFAULT_STACK_SIZE;

	page_map( p->proc_stack_virt, p->proc_stack_phys );
	
	// Code segmnet setup
	for( int i = 0; i < p->text_section_count; i++ ) {
		page_map( p->text_sections[i].virt, p->text_sections[i].phys );
	}

	// Data segment setup
	for( int i = 0; i < p->data_section_count; i++ ) {
		page_map( p->data_sections[i].virt, p->data_sections[i].phys );
	}

	for( int i = 0; i < PROCESS_DEFAULT_STACK_PAGES; i++ ) {
		page_map( p->proc_stack_virt + (i * PAGE_SIZE), p->proc_stack_phys + (i * PAGE_SIZE) );
		//klog( LOG_DEBUG, "[STACK] For pid %d: mapped virt to physical: 0x%016llX -> 0x%016llX", new_p->pid,  new_p->proc_stack_virt + (i * PAGE_SIZE), new_p->proc_stack_phys + (i * PAGE_SIZE) );
	}

	// Context setup
	memset( &p->context, 0, sizeof(registers) );
	p->context.rip = (uint64_t)p->entry;
	p->context.rsp = p->proc_stack_virt + p->stack_size;
	p->context.cs = 0x28;
	p->context.rflags = 0x200;
	p->context.ss = 0x30;

	// Final setup
	p->status = PROCESS_STATUS_ACTIVE;
	//process_set_next_up( p );
	
	/* printf( "Before: \n" );
	process_diagnostic_context(*_context); */

	memcpy( &(**_context), &p->context, sizeof(registers) );

	/* printf( "After:\n" );
	process_diagnostic_context(*_context); */

	kfree(buff);

	klog( LOG_DEBUG, "Out execve handler." );
}