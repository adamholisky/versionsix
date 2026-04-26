#include <kernel_common.h>
#include <process.h>

void print_process_exec_section(const process_exec_section *s, int indent);
void print_symbol_index(const symbol_index *s, int indent);
void print_process_data(const process_data *pd, int indent);
void print_kernel_proc_data(const kernel_proc_data *kpd, int indent);