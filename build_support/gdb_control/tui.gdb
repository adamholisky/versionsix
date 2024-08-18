tui enable
tui new-layout main {-horizontal { {-horizontal src 3 asm 2} 4 cmd 1 } 5  regs 1 } 1 status 0
set tui border-kind acs
layout main
winheight src -5 
focus cmd

source /usr/local/osdev/versions/vi/build_support/gdb_control/commands.gdb