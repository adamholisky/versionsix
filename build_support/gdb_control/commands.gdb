#./configure --prefix=/usr/local/osdev --enable-tui
set disassembly-flavor att

tui enable
tui new-layout main {-horizontal { {-horizontal src 3 asm 2} 4 cmd 1 } 5  regs 1 } 1 status 0
set tui border-kind acs
layout main
winheight src -5 
focus cmd

file /usr/local/osdev/versions/vi/build/versionvi.bin
directory /usr/local/osdev/versions/vi/kernel

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

#break _asm_kernel_start
#break stage5
#break interrupts.c:107 if interrupt_num == 0x30
#break interrupts.c:107 if interrupt_num == 0x31 
#break *0x0010aaaa
#break debug.cpp:30
#break debug.cpp:39
break page.c:166

target remote localhost:5894