tui enable
tui new-layout main {-horizontal { {-horizontal src 2 asm 2} 4 regs 1 } 4 cmd 2 } 1 status 0
set tui border-kind acs
layout main
winheight src -5 
focus cmd

source /usr/local/osdev/source/versionsix/build_support/gdb_control/commands.gdb