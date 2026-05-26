#include <kernel_common.h>
#include <kmemory.h>
#include <process.h>
#include <lib/list.h>
#include <vfs.h>
#include <elf_loader.h>
#include <elf.h>
#include <page.h>

#include <process_debug.h>

kernel_proc_data global_proc_data;

void process_initalize( void ) {
	global_proc_data.pid_next = 0;

	global_proc_data.process_list = avs_list_init();

	// Setup initial process, which is just the idle loop
	pid_t kernel_pid = process_get_new_process();
	if( kernel_pid != 0 ) {
		klog( LOG_PANIC, "Kernel pid isn't 0. Got %d.", kernel_pid );
		do_immediate_shutdown();
	}

	global_proc_data.kernel_pd = process_get_data_from_pid( kernel_pid );

	global_proc_data.kernel_pd->status = PROCESS_STATUS_ACTIVE;
	strcpy( global_proc_data.kernel_pd->name, "Kernel" );
	strcpy( global_proc_data.kernel_pd->path, "/boot/versionvi.bin" );
	memset( &global_proc_data.kernel_pd->context, 0, sizeof(registers) );

	global_proc_data.current_process = global_proc_data.kernel_pd;
	global_proc_data.process_next_up = global_proc_data.root_pd;
}

void process_setup_init( void ) {
	pid_t root_proc_pid = process_get_new_process();

	if ( root_proc_pid != ROOT_PROCESS_ID ) {
		klog( LOG_PANIC, "Root pid == %d, should be %d", root_proc_pid, ROOT_PROCESS_ID );
		do_immediate_shutdown();
	}

	global_proc_data.root_pd = process_get_data_from_pid( root_proc_pid );
	if ( global_proc_data.root_pd->pid != ROOT_PROCESS_ID ) {
		klog( LOG_PANIC, "Manual root process find failed. pid == %d, should be %d", global_proc_data.root_pd->pid, ROOT_PROCESS_ID );
		do_immediate_shutdown();
	}

	strcpy( global_proc_data.root_pd->path, "/bin/first" );
	strcpy( global_proc_data.root_pd->name, "first" );
	strcpy( global_proc_data.root_pd->working_dir, "/" );
	global_proc_data.root_pd->has_own_addr_space = true;

	file_stats fstats;
	vfs_getattr( global_proc_data.root_pd->path, &fstats );
	global_proc_data.root_pd->exec_size = fstats.st_size;

	uint8_t* buff = kmalloc( global_proc_data.root_pd->exec_size );
	int bytes_read = vfs_read( global_proc_data.root_pd->path, buff, global_proc_data.root_pd->exec_size, 0 );

	elf_loader_load( global_proc_data.root_pd, buff );

	for( int i = 0; i < global_proc_data.root_pd->data_section_count; i++ ) {
		page_map( global_proc_data.root_pd->data_sections[i].virt, global_proc_data.root_pd->data_sections[i].phys );
	}

	for( int i = 0; i < global_proc_data.root_pd->text_section_count; i++ ) {
		page_map( global_proc_data.root_pd->text_sections[i].virt, global_proc_data.root_pd->text_sections[i].phys );
	}

	memset( &global_proc_data.root_pd->context, 0, sizeof(registers) );
	global_proc_data.root_pd->context.cs = 0x28;
	global_proc_data.root_pd->context.rflags = 0x200;
	global_proc_data.root_pd->context.ss = 0x30;
	global_proc_data.root_pd->stack_size = PROCESS_DEFAULT_STACK_SIZE;
	global_proc_data.root_pd->proc_stack_virt = 0x00000000A0000000;
	global_proc_data.root_pd->proc_stack_kvirt= page_allocate_kernel( PROCESS_DEFAULT_STACK_PAGES );
	global_proc_data.root_pd->proc_stack_phys = paging_virtual_to_physical( global_proc_data.root_pd->proc_stack_kvirt );
	//global_proc_data.root_pd->proc_stack = kmalloc( global_proc_data.root_pd->stack_size );
	global_proc_data.root_pd->context.rsp = global_proc_data.root_pd->proc_stack_virt + PROCESS_DEFAULT_STACK_SIZE;
	global_proc_data.root_pd->context.rip = (uint64_t)global_proc_data.root_pd->entry;
	global_proc_data.root_pd->status = PROCESS_STATUS_INACTIVE;

	//printf( "proc_stack_virt: 0x%016llX    _kvirt: 0x%016llX    phys: 0x%016llX\n", global_proc_data.root_pd->proc_stack_virt, global_proc_data.root_pd->proc_stack_kvirt, global_proc_data.root_pd->proc_stack_phys );

	//page_map( global_proc_data.root_pd->proc_stack_virt, global_proc_data.root_pd->proc_stack_phys );

	kfree(buff);
}

pid_t process_create( void ) {
	
}

pid_t process_get_new_process( void ) {
	process_data* p = kmalloc( sizeof( process_data ) );

	memset( p, 0, sizeof(process_data) );

	p->pid = global_proc_data.pid_next++;
	p->status = PROCESS_STATUS_IN_SETUP;
	p->first_run = true;

	for( int i = 0; i < PROCESS_MAX_FDS; i++ ) {
		p->file_descriptors[i].id = i;
	}

	p->file_descriptors[ STDIN_FILENO ].in_use = true;
	p->file_descriptors[ STDIN_FILENO ].vfs_fd = STDIN_FILENO;

	p->file_descriptors[ STDOUT_FILENO ].in_use = true;
	p->file_descriptors[ STDOUT_FILENO ].vfs_fd = STDOUT_FILENO;

	p->file_descriptors[ STDERR_FILENO ].in_use = true;
	p->file_descriptors[ STDOUT_FILENO ].vfs_fd = STDOUT_FILENO;

	p->heap_page_count = 0;
	p->heap_pages = NULL;
	p->heap_top = 0x00000000B0000000;

	avs_list_append( global_proc_data.process_list, p );

	//klog( LOG_INFO, "Created new process. pid=%d", p->pid );
	debugf( "New Process. pid=%d    p_d=0x%016llX\n", p->pid, p );
	return p->pid;
}

process_data *process_get_current( void ) {
	return global_proc_data.current_process;
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
	klog( LOG_INFO, "in setup for pid: %d", global_proc_data.current_process->pid );

	return;
}

void process_exit( int ret_code ) {
	exit( ret_code );
}

process_data* process_get_data_from_pid( pid_t pid ) {
	avs_node *head = global_proc_data.process_list->head;
	process_data *p = NULL;

	avs_node *n_p = avs_list_find_data( global_proc_data.process_list, &pid, avs_list_compare_pids );

	if( n_p != NULL ) {
		p = (process_data *)n_p->data;
	} else {
		//klog( LOG_DEBUG, "Couldn't find pid=%d", pid );
	}

	return p;
}

int avs_list_compare_pids( void *a, void *b ) {
	process_data *pd_b = (process_data *)b;
	pid_t pid_a = *(pid_t *)a;
	pid_t pid_b = pd_b->pid;

	//klog( LOG_DEBUG, "Compared ids: a=%d b=%d result=%d", pid_a, pid_b, pid_a == pid_b ? 0 : -1 );
	
	return pid_a == pid_b ? 0 : -1;
}

void process_set_next_up( process_data *p ) {
	global_proc_data.process_next_up = p;
}

int process_sched_yield( registers **context ) {
	static int num_sched_called = 0;

	//debugf( "In yield.\n" );

	process_data *old_p = global_proc_data.current_process;
	process_data *new_p = NULL;

	// If we don't have a next task set, round robin
	if( !global_proc_data.process_next_up) {
		bool next_process_found = false;
		avs_node *head = global_proc_data.process_list->head;
		process_data *p_candidate = NULL;

		for( int i = 0; i < global_proc_data.process_list->size; i++ ) {
			if( head->data == old_p ) {
				break;
			} else {
				head = head->next;

				if( head == NULL ) {
					// We ran out of processes... shouldn't be here?
					debugf( "Scheduler reached, impossible NULL\n" );
					klog( LOG_PANIC, "Scheduler reached impossible NULL" );
				}
			}
		}

		avs_node *next_process_node = head;

		// after this loop, task_head will always contain a good task to switch to
		while( next_process_found == false ) {
			if( next_process_node->next == NULL ) {
				next_process_node = global_proc_data.process_list->head;
			} else {
				next_process_node = next_process_node->next;
			}

			p_candidate = (process_data *)next_process_node->data;

			switch( p_candidate->status ) {
				case PROCESS_STATUS_INACTIVE:
				case PROCESS_STATUS_ACTIVE:
				case PROCESS_STATUS_SLEEP:
					//klog( LOG_DEBUG, "Candidate found. pid: %d", p_candidate->pid );
					//debugf( "Next process found: %d with status %d\n", p_candidate->pid, p_candidate->status );
					next_process_found = true;
					break;
				case PROCESS_STATUS_WAITING:
				default:
					next_process_found = false;
					break;
			}
		}
		
		// go to the new task
		new_p = p_candidate;
	} else {
		new_p = global_proc_data.process_next_up;
		global_proc_data.process_next_up = NULL;
	}

	if( new_p == old_p ) {
		//debugf( "Yield from %d to %d FAILED. Cannot yield to self.\n", old_p->pid, new_p->pid );
		//klog( LOG_PANIC, "Yield from %d to %d FAILED", old_p->pid, new_p->pid );
		return 0;
	}

	memcpy( &old_p->context, &(**context), sizeof(registers) );
	memcpy( &(**context), &new_p->context, sizeof(registers) );

	global_proc_data.current_process = new_p;

	if( old_p->status == PROCESS_STATUS_WAITING_FOR_DEATH ) {
		old_p->status = PROCESS_STATUS_DEAD;
	} else {
		if( old_p->status != PROCESS_STATUS_WAITING ) {
			old_p->status = PROCESS_STATUS_INACTIVE;
		}
	}

	process_data *addr_space_to_copy = NULL;

	if( new_p->has_own_addr_space ) {
		// this is a normal process or has otherwise been flagged to have its own address space (data + code as it stands now )
		addr_space_to_copy = new_p;
	} else {
		// this is a thread, use its parent
		if( new_p != 0 ) {
			process_data *p_parent = process_get_data_from_pid( new_p->pid_parent );
			debugf( "copying parent (%d:%s) addr space for child (%d:%s)\n", p_parent->pid, p_parent->name, new_p->pid, new_p->name );

			addr_space_to_copy = process_get_data_from_pid( new_p->pid_parent );
		}
		
	}

	// Leave the kernel out of this.

	if( new_p->pid != 0 ) {
		for( int i = 0; i < addr_space_to_copy->data_section_count; i++ ) {
			page_map( addr_space_to_copy->data_sections[i].virt, addr_space_to_copy->data_sections[i].phys );
			//klog( LOG_DEBUG, "For pid %d: mapped virt to physical: 0x%016llX -> 0x%016llX", new_p->pid, new_p->data_sections[i].virt, new_p->data_sections[i].phys );
		}

		for( int i = 0; i < new_p->text_section_count; i++ ) {
			page_map( addr_space_to_copy->text_sections[i].virt, addr_space_to_copy->text_sections[i].phys );
			//klog( LOG_DEBUG, "For pid %d: mapped virt to physical: 0x%016llX -> 0x%016llX", new_p->pid, new_p->text_sections[i].virt, new_p->text_sections[i].phys );
		}

		for( int i = 0; i < new_p->heap_page_count; i++ ) {
			page_map( addr_space_to_copy->heap_pages[i].virt, addr_space_to_copy->heap_pages[i].phys );
		}
	}

	for( int i = 0; i < PROCESS_DEFAULT_STACK_PAGES; i++ ) {
		page_map( new_p->proc_stack_virt + (i * PAGE_SIZE), new_p->proc_stack_phys + (i * PAGE_SIZE) );
		//klog( LOG_DEBUG, "[STACK] For pid %d: mapped virt to physical: 0x%016llX -> 0x%016llX", new_p->pid,  new_p->proc_stack_virt + (i * PAGE_SIZE), new_p->proc_stack_phys + (i * PAGE_SIZE) );
	}

	if( new_p->first_run ) {
		new_p->first_run = false;
	}

	new_p->status = PROCESS_STATUS_ACTIVE;

	/* if( new_p->pid == 3 ) {
		(*context)->rbx = 0xBBBB;
		(*context)->rcx = 0xCCCC;
		(*context)->rdx = 0xDDDD;
		(*context)->rsi = 0x1111;
		(*context)->rdi = 0x2222;
		(*context)->rbp = 0x3333;
		(*context)->error_no = 0x6535;
	} */

	//process_diagnostic_context( &new_p->context );
	//klog( LOG_DEBUG, "Out yield. New PID: %d  New RIP=0x%016llX  phys=0x%016llX", new_p->pid, new_p->context.rip, paging_virtual_to_physical( new_p->context.rip ) );
	
	//debugf( "%d", new_p->pid );

	//debugf( "Yield from %d to %d\n", old_p->pid, new_p->pid );
	klog( LOG_INFO, "Yield from %d to %d", old_p->pid, new_p->pid );

	return 0;
}

pid_t process_get_current_proc_id( void ) {
	return global_proc_data.current_process->pid;
}

void process_sched_set_next_yield( pid_t pid ) {
	global_proc_data.process_next_up = process_get_data_from_pid(pid);
}

void processes_diagnostic_dump( void ) {
	printf( "Current pid: %d\n", global_proc_data.current_process->pid );

	printf( "Proc list\n====================\n" );

	avs_node *n = global_proc_data.process_list->head;
	for( int i = 0; i < global_proc_data.process_list->size; i ++ )  {
		process_data *p = n->data;

		printf( "PID %d (%s) proc_data @ 0x%016llX\n", p->pid, p->name, p);
		printf( "    Status=%d     exit_code=0x%016llX    rip: 0x%016llX\n", p->status, p->exit_code, p->context.rip );
		printf( "    proc_stac_virt: 0x%016llX  _kvirt: 0x%016llX  _phys: 0x%016llX  size=0x%X\n", p->proc_stack_virt, p->proc_stack_kvirt, p->proc_stack_phys, p->stack_size );
		printf( "    argc=0x%X    argv=0x%016llX    working_dir=\"%s\"\n", p->argc, p->argv, p->working_dir );
		printf( "    entry=0x%016llX    path=\"%s\"    exec_size=0x%X    has_own_addrs: %d\n", p->entry, p->path, p->exec_size, p->has_own_addr_space );
		printf( "    ts_count: %X    ts_virt_start: 0x%016llX\n", p->text_section_count, p->text_secton_virt_start );
		for( int j = 0; j < p->text_section_count; j++ ) {
			printf( "        %d: p=0x%016llX    v=0x%016llX    kv=0x%016llX\n", j, p->text_sections[j].phys, p->text_sections[j].virt, p->text_sections[j].kern_virt );
		}
		printf( "    ds_count: %X    ds_virt_start: 0x%016llX\n", p->data_section_count, p->data_section_virt_start );
		for( int k = 0; k < p->data_section_count; k++ ) {
			printf( "        %d: p=0x%016llX    v=0x%016llX    kv=0x%016llX\n", k, p->data_sections[k].phys, p->data_sections[k].virt, p->data_sections[k].kern_virt );
		}
		
		n = n->next;
	}
}

void process_diagnostic_context( registers *context ) {
	registers *reg = context;

	printf( "================================================================================\n" );
	printf( "    rip:  0x%016llX\n", reg->rip );
	printf( "    rax:  0x%016llX  rbx:  0x%016llX  rcx:  0x%016llX\n", reg->rax, reg->rbx, reg->rcx );
	printf( "    rdx:  0x%016llX  rsi:  0x%016llX  rdi:  0x%016llX\n", reg->rdx, reg->rsi, reg->rdi );
	printf( "    rsp:  0x%016llX  rbp:  0x%016llX  cr0:  0x%016llX \n", reg->rsp, reg->rbp, 0 );
	printf( "    cr2:  0x%016llX  cr3:  0x%016llX  cr4:  0x%016llX\n", 0, 0, 0 );
	printf( "    cs:   0x%04X  num:  0x%08X  err:  0x%08X  flag: 0x%08X\n", reg->cs, 0, 0, reg->rflags);
	printf( "================================================================================\n" );
	printf( "\n" );
}

int process_alloc_fd( process_data *p ) {
	int res = 0;

	for( int i = 0; i < PROCESS_MAX_FDS; i++ ) {
		if( p->file_descriptors[i].in_use == false ) { 
			p->file_descriptors[i].pos = 0;
			res = i;
			break;
		}
	}

	return res;
}

void process_free_fd( process_data *p, int fd ) {
	if( fd > 0 ) {
		if( fd < PROCESS_MAX_FDS ) {
			p->file_descriptors[fd].in_use = false;
		} else {
			klog( LOG_ERROR, "fd greater than max fds: %d", fd );
		}
	} else {
		klog( LOG_ERROR, "fd less than 0: %d", fd );
	}
}

char* process_get_fd_path( process_data *p, int fd ) {
	// TODO: Boundry checks

	vfs_fd *v_fd = vfs_get_fd_data( p->file_descriptors[fd].vfs_fd );
	return v_fd->path;
}

char* process_get_current_path( void ) {
	return global_proc_data.current_process->path;
}

void process_set_status( pid_t pid, uint8_t status ) {
	process_data *p = process_get_data_from_pid(pid);

	p->status = status;
}

int process_get_errno( pid_t pid ) {
	process_data *p = process_get_data_from_pid(pid);

	return p->errno;
}

void process_set_errno( pid_t pid, int err ) {
	process_data *p = process_get_data_from_pid(pid);

	p->errno = err;
}