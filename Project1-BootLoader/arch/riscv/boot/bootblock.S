#include <asm/biosdef.h>

// os size location (os_size could be sectors [p1-task3] or bytes [p1-task4])
.equ os_size_loc, 0x502001fc

// kernel address (move kernel to here, and jmp here to start kernel)
.equ kernel, 0x50201000

// BIOS function entry (jump here to use BIOS APIs)
.equ bios_func_entry, 0x50150000

.equ decompressor, 0x52040000
.text
.global main

main:
	// fence on all memory and I/O
	fence

	// TODO: [p1-task1] call BIOS to print string "It's bootblock!"
	la a0,msg
	li a7,BIOS_PUTSTR
	jal bios_func_entry


	// TODO: [p1-task2] call BIOS to read kernel in SD card
	li a7,BIOS_SDREAD
	lui a0,%hi(decompressor)
	addi a0,a0,%lo(decompressor)
	li t0, os_size_loc
	lh a1, 0(t0)
	li a2,1
	jal bios_func_entry
	// TODO: [p1-task4] load task-related arguments and pass them to kernel
	call decompressor
	// TODO: [p1-task2] jump to kernel to start UCAS-OS
	j kernel
	/************************************************************/
	/* Do not touch this comment. Reserved for future projects. */
	/************************************************************/
// while(1) --> stop here
stop:
	j stop
	nop

.data

msg: .string "It's a bootloader...\n\r"

