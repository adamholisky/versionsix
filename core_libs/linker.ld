OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)


SECTIONS
{
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.bss : {
        *(COMMON)
        *(.bss)
    }
}