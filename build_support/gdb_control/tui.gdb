source /home/adam/.local/share/uv/tools/pwndbg/share/pwndbg/gdbinit.py
tui enable
tui new-layout pwndbg_custom {-horizontal { { -horizontal { pwndbg_code 1 pwndbg_disasm 1 } 2 { {-horizontal pwndbg_legend 8 pwndbg_control 2 } 0 pwndbg_regs 1 pwndbg_stack 1 } 3 } 7 cmd 3 } 3 { pwndbg_backtrace 2 pwndbg_expressions 2 pwndbg_threads 1 } 1 } 1 status 1
layout pwndbg_custom

focus cmd

source /usr/local/osdev/source/versionsix/build_support/gdb_control/commands.gdb