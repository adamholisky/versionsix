#./configure --prefix=/usr/local/osdev --enable-tui
set disassembly-flavor att

tui enable
tui new-layout main {-horizontal { {-horizontal src 3 asm 2} 4 cmd 1 } 5  regs 1 } 1 status 0
set tui border-kind acs
layout main
winheight src -5 
focus cmd

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