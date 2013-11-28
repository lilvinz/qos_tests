MEMORY 
{
	flash (RX) : ORIGIN = 0x00100000+0x4000, LENGTH = 256K-0x4000
	ram        : ORIGIN = 0x00200000, LENGTH = 64K-0x1000
}

ENTRY(_entry_point)

__stack_end__ = 0x00200000 + 64K - 4;

SECTIONS 
{
	.prog : 
	{
        . = ALIGN(4);
        *(.entry_point)
        _image_information_offs = . - 0x0104000;
        KEEP(*(.image_information))
		*(.startup_code)
		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
		*(.glue_7)
		*(.glue_7t)
        . = ALIGN(4);
        _efixed = .;
	} >flash

	__end_of_text__ = .;
    
	.relocate :
	{
        . = ALIGN(4);
		__data_beg__ = .;
        __data_beg_src__ = __end_of_text__;
		*(.vector)
		*(.ramfunc)
		*(.data)
		__data_end__ = .;
	} >ram AT>flash

	.bss (NOLOAD) : 
	{
		__bss_beg__ = .;
		*(.bss)
	} >ram

	/* Align here to ensure that the .bss section occupies space up to
	_end.  Align after .bss to ensure correct alignment even if the
	.bss section disappears because there are no input sections.  */
	. = ALIGN(32 / 8);
}
	. = ALIGN(32 / 8);
	_end = .;
	_bss_end__ = . ; __bss_end__ = . ; __end__ = . ;
	PROVIDE (end = .);
