ENTRY(_start)

SECTIONS
{
    /* Begin putting sections at 1 MiB */
    . = 1M;

    /* Multiboot header first */
    .text BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.multiboot))
        *(.text)
    }

    /* Read-only data */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    /* Read-write data (initialized) */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* Read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bootstrap_stack)
    }

    /* Remove unused sections */
    /DISCARD/ : 
    {
        *(.comment)
        *(.eh_frame)
        *(.note.gnu.build-id)
    }
}

