#./configure --prefix=/usr/local/osdev --enable-tui
set disassembly-flavor att

file /usr/local/osdev/versions/versionsix/build/versionvi.bin
directory /usr/local/osdev/versions/versionsix/kernel

define qq
  set confirm off
  quit
end

define ds
  set $i = 10
  if $argc == 1
    set $i = $arg0
  end

  eval "x/%d%c $esp", $i, 'x'
end

define dm 
  set $i = 10
  if $argc == 2
    set $i = $arg1
  end

  eval "x/%d%c %d", $i, 'x', $arg0
end

source /usr/local/osdev/versions/versionsix/build_support/gdb_control/breakpoints.gdb

target remote localhost:5894