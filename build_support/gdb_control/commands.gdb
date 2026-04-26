#./configure --prefix=/usr/local/osdev --enable-tui
set disassembly-flavor att

file /usr/local/osdev/source/versionsix/build/versionvi.bin
directory /usr/local/osdev/source/versionsix/build/versionvi.bin

define qq
  set confirm off
  quit
end

define ds
  set $i = 10
  if $argc == 1
    set $i = $arg0
  end

  eval "x/%d%c $rsp", $i, 'x'
end

define dm 
  set $i = 10
  if $argc == 2
    set $i = $arg1
  end

  eval "x/%d%c %d", $i, 'x', $arg0
end

define ps 
  set $num_procs = *(avs_list *)global_proc_data.process_list.size
  set $head = *(avs_list *)global_proc_data.process_list.head
  set $i = 0

  print $num_procs
  print $head

end

set print pretty on

source /usr/local/osdev/source/versionsix/build_support/gdb_control/breakpoints.gdb

set logging file /usr/local/osdev/source/versionsix/build_support/logs/gdb.log
set logging overwrite on
set logging enabled on

target remote localhost:58001