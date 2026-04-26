#include <process_debug.h>

// ─── Helpers ─────────────────────────────────────────────────────────────────

/* Indentation: 4 spaces per level, max 8 levels */
static void print_indent(int indent) {
    static const char spaces[] = "                                "; // 32 spaces
    int count = indent * 4;
    if (count > 32) count = 32;
    printf("%.*s", count, spaces);
}

static void print_field_str(int indent, const char *label, const char *value) {
    print_indent(indent);
    printf("%-26s %s\n", label, value);
}

static void print_field_u8(int indent, const char *label, uint8_t value) {
    print_indent(indent);
    printf("%-26s %u\n", label, (unsigned)value);
}

static void print_field_u16(int indent, const char *label, uint16_t value) {
    print_indent(indent);
    printf("%-26s %u\n", label, (unsigned)value);
}

static void print_field_u32(int indent, const char *label, uint32_t value) {
    print_indent(indent);
    printf("%-26s %u\n", label, (unsigned)value);
}

static void print_field_u64(int indent, const char *label, uint64_t value) {
    print_indent(indent);
    printf("%-26s %llu\n", label, (unsigned long long)value);
}

static void print_field_i32(int indent, const char *label, int32_t value) {
    print_indent(indent);
    printf("%-26s %d\n", label, value);
}

static void print_field_bool(int indent, const char *label, bool value) {
    print_indent(indent);
    printf("%-26s %s\n", label, value ? "true" : "false");
}

static void print_field_ptr(int indent, const char *label, const void *ptr) {
    print_indent(indent);
    printf("%-26s %s\n", label, ptr ? "0x" : "(null)");
    if (ptr)
        printf("%#018llx\n", (unsigned long long)(uintptr_t)ptr);
}

/* Cleaner pointer-on-same-line variant used for most fields */
static void print_field_ptr_inline(int indent, const char *label, const void *ptr) {
    print_indent(indent);
    if (ptr)
        printf("%-26s %#018llx\n", label, (unsigned long long)(uintptr_t)ptr);
    else
        printf("%-26s (null)\n", label);
}

static void print_section_header(int indent, const char *title) {
    print_indent(indent);
    printf("=== %s ===\n", title);
}

static void print_separator(int indent) {
    print_indent(indent);
    printf("---\n");
}

// ─── process_exec_section ────────────────────────────────────────────────────

void print_process_exec_section(const process_exec_section *s, int indent) {
    if (!s) { print_indent(indent); printf("(null)\n"); return; }
    print_field_ptr_inline(indent, "phys:",      (void*)s->phys);
    print_field_ptr_inline(indent, "virt:",      (void*)s->virt);
    print_field_ptr_inline(indent, "kern_virt:", (void*)s->kern_virt);
}

// ─── symbol_index ────────────────────────────────────────────────────────────

void print_symbol_index(const symbol_index *s, int indent) {
    if (!s) { print_indent(indent); printf("(null)\n"); return; }
    print_field_u16(indent, "elf_index:", s->elf_index);
    print_field_str(indent, "name:",      s->name);
    print_field_u64(indent, "value:",     s->value);
    print_field_u64(indent, "offset:",    s->offset);
    print_field_u64(indent, "info:",      s->info);
    print_field_u64(indent, "size:",      s->size);
}

// ─── process_data ────────────────────────────────────────────────────────────

void print_process_data(const process_data *pd, int indent) {
    if (!pd) { print_indent(indent); printf("(null)\n"); return; }

    print_section_header(indent, "process_data");

    // Identity
    print_field_u32   (indent, "pid:",            pd->pid);
    print_field_u8    (indent, "status:",          pd->status);
    print_field_i32   (indent, "exit_code:",       pd->exit_code);
    print_field_str   (indent, "name:",            pd->name);
    print_field_str   (indent, "working_dir:",     pd->working_dir);

    // Execution
    print_separator(indent);
    print_field_ptr_inline(indent, "entry:",       pd->entry);
    print_field_str   (indent, "path:",            pd->path);
    print_field_u32   (indent, "exec_size:",       pd->exec_size);
    print_field_bool  (indent, "has_own_addr_space:", pd->has_own_addr_space);

    // Stack
    print_separator(indent);
    print_field_ptr_inline(indent, "proc_stack:",  pd->proc_stack);
    print_field_u32   (indent, "stack_size:",      pd->stack_size);

    // Arguments
    print_separator(indent);
    print_field_i32(indent, "argc:", pd->argc);
    print_indent(indent); printf("%-26s\n", "argv:");
    for (int i = 0; i < pd->argc && pd->argv; i++) {
        print_indent(indent + 1);
        printf("[%d] %s\n", i, pd->argv[i] ? pd->argv[i] : "(null)");
    }

    // Text sections
    print_separator(indent);
    print_field_u16(indent, "text_section_count:", pd->text_section_count);
    print_field_ptr_inline(indent, "text_sections:", pd->text_sections);
    print_field_ptr_inline(indent, "text_virt_start:", (void*)pd->text_secton_virt_start);
    for (uint16_t i = 0; i < pd->text_section_count && pd->text_sections; i++) {
        print_indent(indent + 1); printf("[text %u]\n", (unsigned)i);
        print_process_exec_section(&pd->text_sections[i], indent + 2);
    }

    // Data sections
    print_separator(indent);
    print_field_u16(indent, "data_section_count:", pd->data_section_count);
    print_field_ptr_inline(indent, "data_sections:",     pd->data_sections);
    print_field_ptr_inline(indent, "data_virt_start:",   (void*)pd->data_section_virt_start);
    for (uint16_t i = 0; i < pd->data_section_count && pd->data_sections; i++) {
        print_indent(indent + 1); printf("[data %u]\n", (unsigned)i);
        print_process_exec_section(&pd->data_sections[i], indent + 2);
    }

    // Symbols
    print_separator(indent);
    print_field_u16(indent, "num_dyn_syms:", pd->num_dyn_syms);
    print_indent(indent); printf("%-26s\n", "rela_sym_index:");
    print_symbol_index(pd->rela_sym_index, indent + 1);
    print_indent(indent); printf("%-26s\n", "dyn_sym_index:");
    for (uint16_t i = 0; i < pd->num_dyn_syms && pd->dyn_sym_index; i++) {
        print_indent(indent + 1); printf("[dyn sym %u]\n", (unsigned)i);
        print_symbol_index(&pd->dyn_sym_index[i], indent + 2);
    }

    // Misc
    print_separator(indent);
    print_field_ptr_inline(indent, "binary_format_data:", pd->binary_format_data);
    // Note: 'context' (registers) omitted — print separately with a dedicated registers printer
}

// ─── kernel_proc_data ────────────────────────────────────────────────────────

void print_kernel_proc_data(const kernel_proc_data *kpd, int indent) {
    if (!kpd) { print_indent(indent); printf("(null)\n"); return; }

    print_section_header(indent, "kernel_proc_data");
    print_field_ptr_inline(indent, "process_list:",    kpd->process_list);
    print_field_u32       (indent, "pid_next:",        kpd->pid_next);

    print_separator(indent);
    print_indent(indent); printf("%-26s\n", "kernel_pd:");
    print_process_data(kpd->kernel_pd, indent + 1);

    print_separator(indent);
    print_indent(indent); printf("%-26s\n", "root_pd:");
    print_process_data(kpd->root_pd, indent + 1);

    print_separator(indent);
    print_indent(indent); printf("%-26s\n", "current_process:");
    print_process_data(kpd->current_process, indent + 1);

    print_separator(indent);
    print_indent(indent); printf("%-26s\n", "process_next_up:");
    print_process_data(kpd->process_next_up, indent + 1);
}