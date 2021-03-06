
#include "pxa255.h"

	/* Declare the names of stacks */
	.global _start
	.global fiq_stack
	.global irq_stack
	.global abt_stack
	.global und_stack
	.global sys_stack
	.global __SwitchTo
	.global __SaveAndSwitch

	/* Stack allocation */
	.comm fiq_stack, FIQ_STACK_SIZE
	.comm irq_stack, IRQ_STACK_SIZE
	.comm abt_stack, ABT_STACK_SIZE
	.comm und_stack, UND_STACK_SIZE
	.comm sys_stack, SYS_STACK_SIZE

	.text
	/* exception handler vector table */
_start:
	b reset_handler
	b und_handler
	b swi_handler
	b abt_pref_handler
	b abt_data_handler
	b not_used
	b irq_handler
	b fiq_handler

.align 4
und_handler:
	b loop

swi_handler:
	b loop

abt_pref_handler:
	b loop

abt_data_handler:
	b loop

not_used:
	b loop

loop:
	b loop

irq_handler:
	b __Interrupt_Handler

fiq_handler:
	b loop

reset_handler:
	b mask_off_int_reg
				
//Disable all IRQs and FIQs
mask_off_int_reg:
        ldr     r1, =INT_BASE
        ldr     r2, =0x00000000
	//Interrupt Controller Mask Register
        str     r2, [r1, #INT_ICMR]
	////Interrupt Controller Level Register (determined IRQ or	
        str     r2, [r1, #INT_ICMR]

init_gpio:
        // FFUART
        ldr r12, =FFUART_BASE

        ldr r0, =0x00000000
	//Line Control Register
        str r0, [r12, #FFLCR]

        ldr r0, =0x00000000
	//Interrupt Enable Register
        str r0, [r12, #FFIER]
        ldr r0, =0x00000080

	//Line Control Register
        str r0, [r12, #FFLCR]
        ldr r0, =0x00000018

	//Divisor Latch Low Register
        str r0, [r12, #FFDLL]
        ldr r0, =0x00000000

	//Divisor Latch High Register
        str r0, [r12, #FFDLH]
        ldr r0, =0x00000000

	//Line Control Register
        str r0, [r12, #FFLCR]
        ldr r0, =0x00000003		//8-bit character
        str r0, [r12, #FFLCR]
        ldr r0, =0x00000001		//6-bit character

        str r0, [r12, #FFFCR]		
        ldr r0, =0x00000007		//Transmit and Receive FIFO Enable
					//Reset Receiver FIFO
					//Reset transmitter FIFO
        str r0, [r12, #FFFCR]
        ldr r0, =0x00000000

	//Interrupt Enable Register
        str r0, [r12, #FFIER]
        ldr r0, =0x00000000

	//Modem Control Register
        str r0, [r12, #FFMCR]
	//Infrared Selection Register
        ldr r0, =FFISR_VALUE
        str r0, [r12, #FFISR]

        ldr r12, =FFUART_BASE
        ldr r0, =0x00000040
        str r0, [r12, #FFIER]

        // clear the rx fifo errors
        ldr r0, =FFFCR_VALUE		
        and r0, r0, #0x00000002		//Reset Receiver FIFO
        str r0, [r12, #FFFCR]

        // First set the output values to a safe/disabled state before we change
        // any GPIO's outputs
        // start by settings all of them high which is the safest for most signals

        ldr r12, =GPIO_BASE

        ldr r0, =0xffffffff
        str r0, [r12, #GPIO_GPSR0]
        str r0, [r12, #GPIO_GPSR1]
        str r0, [r12, #GPIO_GPSR2]

        // GPCR0
        // Now clear any high true outputs that need to start low
        ldr r0, =(GPIO_7 | GPIO_13 | GPIO_17 | GPIO_27)	// USB speed = high
							// PRST = deasserted
							// BKL_ON
							// Passive LCD enable = off

        str r0, [r12, #GPIO_GPCR0]

        // Next, set the correct direction for out functions. A one meas output.
        ldr r0, =(GPIO_3 | GPIO_4 | GPIO_6  | GPIO_7 | GPIO_11 | GPIO_13 | GPIO_15 | GPIO_17 | GPIO_23 | GPIO_24 | GPIO_25 | GPIO_27 | GPIO_30 | GPIO_31)

        // GPDR0
        str r0, [r12, #GPIO_GPDR0]

        ldr r0, =(GPIO_32 | GPIO_33 | GPIO_39 | GPIO_40 | GPIO_41 | GPIO_43 | GPIO_45 | GPIO_47  | GPIO_48 | GPIO_49 | GPIO_50 | GPIO_51 | GPIO_52 | GPIO_53 | GPIO_54 | GPIO_55 | GPIO_58 | GPIO_59 | GPIO_60 | GPIO_61 | GPIO_62 | GPIO_63)

        //GPDR1
        str r0, [r12, #GPIO_GPDR1]

        ldr r0, =(GPIO_64 | GPIO_65 | GPIO_66 | GPIO_67 | GPIO_68 | GPIO_69 | GPIO_70 | GPIO_71 | GPIO_72 | GPIO_73 | GPIO_74 | GPIO_75 | GPIO_76 | GPIO_77 | GPIO_78 | GPIO_79 | GPIO_80 | GPIO_81 | GPIO_84)

        // GPDR2
        str r0, [r12, #GPIO_GPDR2]

        // Finally, set the alternate function registers to the correct state
        ldr r0, =GPIO_15_AF_CS1	;

        str r0, [r12, #GPIO_GAFR0L]

        ldr r0, =(GPIO_18_AF_RDY | GPIO_23_AF_SSP_CLK | GPIO_24_AF_SSP_FRM | GPIO_25_AF_SSP_TXD | GPIO_26_AF_SSP_RXD | GPIO_28_AF_AC97_BCLK_IN | GPIO_29_AF_AC97_SDIN0 | GPIO_30_AF_AC97_SDOUT | GPIO_31_AF_AC97_SYNC)

        str r0, [r12, #GPIO_GAFR0U]

        ldr r0, =(GPIO_33_AF_CS5 | GPIO_34_AF_FF_RXD | GPIO_35_AF_FF_CTS | GPIO_36_AF_FF_DCD | GPIO_37_AF_FF_DSR | GPIO_38_AF_FF_RI | GPIO_39_AF_FF_TXD | GPIO_40_AF_FF_DTR | GPIO_41_AF_FF_RTS | GPIO_42_AF_BT_RXD | GPIO_43_AF_BT_TXD | GPIO_44_AF_BT_CTS | GPIO_45_AF_BT_RTS | GPIO_46_AF_IR_RXD | GPIO_47_AF_IR_TXD)

        str r0, [r12, #GPIO_GAFR1L]

        ldr r0, =(GPIO_48_AF_POE | GPIO_49_AF_PWE | GPIO_50_AF_PIOR | GPIO_51_AF_PIOW | GPIO_52_AF_PCE1 | GPIO_53_AF_PCE2 | GPIO_54_AF_PSKTSEL | GPIO_55_AF_PREG | GPIO_56_AF_PWAIT | GPIO_57_AF_IOIS16 | GPIO_58_AF_LDD0 | GPIO_59_AF_LDD1 | GPIO_60_AF_LDD2 | GPIO_61_AF_LDD3 | GPIO_62_AF_LDD4 | GPIO_63_AF_LDD5)

        str r0, [r12, #GPIO_GAFR1U]

        ldr r0, =(GPIO_64_AF_LDD6 | GPIO_65_AF_LDD7 | GPIO_66_AF_LDD8 | GPIO_67_AF_LDD9 | GPIO_68_AF_LDD10 | GPIO_69_AF_LDD11 | GPIO_70_AF_LDD12 | GPIO_71_AF_LDD13 | GPIO_72_AF_LDD14 | GPIO_73_AF_LDD15 | GPIO_74_AF_LCD_FCLK | GPIO_75_AF_LCD_LCLK | GPIO_76_AF_LCD_PCLK | GPIO_77_AF_LCD_BIAS | GPIO_78_AF_CS2 | GPIO_79_AF_CS3)

        str r0, [r12, #GPIO_GAFR2L]

        ldr r0, =GPIO_80_AF_CS4

        str r0, [r12, #GPIO_GAFR2U]

        // Clear the peripheral control registers bits
        ldr r12, =PSSR
        ldr r0, =(PSSR_RDH | PSSR_PH)
        str r0, [r12]


//PXA250 Clock Register initialization
init_clock_reg:		
        ldr r1, =CLK_BASE	/* base of clock registers */
        ldr r2, =0x00000241     /* memory clock: 100MHz,
				   nomal core clock: 200MHz,
				   turbo mode: 400MHz */
        str r2, [r1, #CLK_CCCR]

/* PXA250 Core Clock is set to Turbo Mode */
set_core_clock:
        ldr r0,=0
        mrc p14, 0, r0, c6, c0, 0
        ldr r0,=3
        mcr p14,0, r0, c6, c0, 0

set_os_timer:		/* OS Timer initialization */
        ldr r1, =TMR_BASE	/* reset the OS Timer Count to zero */
        mov r2, #0
        str r2, [r1, #TMR_OSCR]
        ldr r4, =0x300	/* really 0x2E1 (0.27uS * 0x2E1 = 198uS) is about 200usec,   */
			/* so 0x300 should be plenty :	Wait OS Timer stabilize */
1:
        ldr r2, [r1, #TMR_OSCR]
        cmp r4, r2
        bgt 1b

/* PXA250 Memory Controller initialization */
init_mem_ctrl:
        ldr r1, =MSC_BASE	/* base of memory controller */

        /* static memory controller initialization */

        ldr r2, =0x123412c0	/* CS0 : flash , CS1 : SMSC LAN91C1111  */
        str r2, [r1, #MSC_MSC0]
        ldr r3, [r1, #MSC_MSC0]

        ldr r2, =0x3ff923e9	/* CS2 : TD242(USB2.0),
				   CS3 : extended board (FPGA board, iom board) */
        str r2, [r1, #MSC_MSC1]
        ldr r3, [r1, #MSC_MSC1]

        ldr r2, =0x3ff97ff9	/* CS4 : SL811HS(USB1.0) , CS5 : KEY PAD  */
        str r2, [r1, #MSC_MSC2]
        ldr r3, [r1, #MSC_MSC2]

        /* MECR: Memory Expansion Card Register */
        ldr r2, =CFG_MECR_VAL
        str r2, [r1, #MSC_MECR]
        ldr r2, [r1, #MSC_MECR]

        /* MCMEM0: Card Interface slot 0 timing */
        ldr r2, =CFG_MCMEM0_VAL
        str r2, [r1, #MSC_MCMEM0]
        ldr r2, [r1, #MSC_MCMEM0]

        /* MCMEM1: Card Interface slot 1 timing */
        ldr r2, =CFG_MCMEM1_VAL
        str r2, [r1, #MSC_MCMEM1]
        ldr r2, [r1, #MSC_MCMEM1]

        /* MCATT0: Card Interface Attribute Space Timing, slot 0 */
        ldr r2, =CFG_MCATT0_VAL
        str r2, [r1, #MSC_MCATT0]
        ldr r2, [r1, #MSC_MCATT0]

        /* MCATT1: Card Interface Attribute Space Timing, slot 1 */
        ldr r2, =CFG_MCATT1_VAL
        str r2, [r1, #MSC_MCATT1]
        ldr r2, [r1, #MSC_MCATT1]

        /* MCIO0: Card Interface I/O Space Timing, slot 0 */
        ldr r2, =CFG_MCIO0_VAL
        str r2, [r1, #MSC_MCIO0]
        ldr r2, [r1, #MSC_MCIO0]

        /* MCIO1: Card Interface I/O Space Timing, slot 1 */
        ldr r2, =CFG_MCIO1_VAL
        str r2, [r1, #MSC_MCIO1]
        ldr r2, [r1, #MSC_MCIO1]

        /* SDRAM Controller initialized at 100MHz */
        ldr r2, =0x03c00fff		/* build MDREFR in a specific order */
        str r2, [r1, #MSC_MDREFR]

        ldr r2, =0x03c00018		/* REF Rate = 
					     (64MS/8192 Rows) * 100MHz /32 = 24 */
        str r2, [r1, #MSC_MDREFR]

        ldr r2, =0x03c52018		/* Set K0RUN, K1RUN and K2RUN */
        str r2, [r1, #MSC_MDREFR]

        ldr r2, =0x03852018		/* Clear Self Refresh */
        str r2, [r1, #MSC_MDREFR]

        ldr r2, =0x0385b018		/* Set E0PIN and E1PIN */
        str r2, [r1, #MSC_MDREFR]

        ldr r2, =0x0ac80ac8		/* Set CNFG, but don't enable just yet,
					   CL = 3, tRP = 3clk, tRC = 10clk,
					   tRAS = 7clk, tRCD = 3clk */
        str r2, [r1, #MSC_MDCNFG]

        /* SDRAM Controller initialized at 50MHz. */
        /* pause for ~200usec for SDRAM to init */
        ldr r1, =TMR_BASE		/* reset the OS Timer Count to zero */
        mov r2, #0
        str r2, [r1, #TMR_OSCR]
        ldr r4, =0x300			/* really 0x2E1 (0.27uS * 0x2E1 = 198uS) 
					   is about 200usec, */
	                                /* so 0x300 should be plenty */
1:
        ldr r2, [r1, #TMR_OSCR]
        cmp r4, r2
        bgt 1b

        /* force 9 cycles to SDRAM : SDRAM bank all in the CBR (refrash cycle) */
        ldr r2, =SDRAM_BASE_ADDRESS
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]
        str r2, [r2]

        ldr r1, =MSC_BASE		/* base of memory controller */

        ldr r2, =0x0acb0acb		/* now enable SDRAM */
        str r2, [r1, #MSC_MDCNFG]

        /* and do an MRS */
        ldr r2, =0x00000000
        str r2, [r1, #MSC_MDMRS]

        /* sdram initialize done */

set_stack_pointer:

        /* FIQ mode */
        mrs r0, cpsr		/* move CPSR to r0 */
        bic r0, r0, #0x1f	/* clear all mode bits */
        orr r0, r0, #0xd1	/* set FIQ mode bits */
        msr CPSR_c, r0		/* move back to CPSR */
        ldr sp, =(fiq_stack + FIQ_STACK_SIZE - 4)	/* initialize the stack ptr */

        /* IRQ mode */
        mrs r0, cpsr		/* move CPSR to r0 */
        bic r0, r0, #0x1f	/* clear all mode bits */
        orr r0, r0, #0xd2	/* set IRQ mode bits */
        msr CPSR_c, r0		/* move back to CPSR */
        ldr sp, =(irq_stack + IRQ_STACK_SIZE - 4)	/* initialize the stack ptr */

        /* Abort mode */
        mrs r0, cpsr		/* move CPSR to r0 */
        bic r0, r0, #0x1f	/* clear all mode bits */
        orr r0, r0, #0xd7	/* set Abort mode bits */
        msr CPSR_c, r0		/* move back to CPSR */
        ldr sp, =(abt_stack + ABT_STACK_SIZE - 4)	/* initialize the stack ptr */

        /* Undef mode */
        mrs r0, cpsr		/* move CPSR to r0 */
        bic r0, r0, #0x1f	/* clear all mode bits */
        orr r0, r0, #0xdb	/* set Undef mode bits */
        msr CPSR_c, r0		/* move back to CPSR */
        ldr sp, =(und_stack + UND_STACK_SIZE - 4)	/* initialize the stack ptr */

        /* System mode */
        mrs r0, cpsr		/* move CPSR to r0 */
        bic r0, r0, #0x1f	/* clear all mode bits */
        orr r0, r0, #0xdf	/* set System mode bits */
        msr CPSR_c, r0		/* move back to CPSR */
        ldr sp, =(sys_stack + SYS_STACK_SIZE - 4)	/* initialize the stack ptr */


.align 4
set_svc_mode:
	/* Change (back) to SVC mode */
	mrs r0, cpsr		/* move CPSR to r0 */
	bic r0, r0, #0x1f	/* clear all mode bits */
	orr r0, r0, #0xd3	/* set System mode bits */
	msr CPSR_c, r0		/* move back to CPSR */
	/* Reset the stack pointer for the SVC mode (our current mode) */
	ldr sp, =(0xa4000000 - 4) ;
	
	/*
	 * trampoline - jump to C function main().
	 */
.align 4
relocate:

//	adr r0, _start

	// relocate the second stage loader
//	add r2, r0, #(1024 * 1024)
//	ldr r1, =0xa1000000

	/* r0 = source address
	 * r1 = target address
	 * r2 = source end address
	 */
copy_loop:
//	ldmia   r0!, {r3-r10}
//	stmia   r1!, {r3-r10}
//	cmp     r0, r2

//	ble     copy_loop

jump_to_main:
	
	bl main

	/* the C code should never return */
	b _start
.align 4

__Interrupt_Handler:
	msr CPSR_c, #(0xc0 | 0x12)  //CPSR_c (0:7 bits), IRQ stack
	stmfd sp!, {r1-r3}	  // push working registers onto IRQ stack
	mov r1, sp
	add sp, sp, #12	  //IRQ stack: sp = sp + 12 (3 items)
	sub r2, lr, #4	  //r2 = lr - 4 (return address)
	mrs r3, SPSR	  //r3 = SPSR

	//change to SVC mode
	msr CPSR_c, #(0xc0 | 0x13)     //sp => thread stack
	stmfd sp!, {r2}		     //r2 = return address (pc)
	stmfd sp!, {lr}		     //lr (r14)
	stmfd sp!, {r4-r12}

	//pop IRQ stack to working registers (r1-r3)
	ldmfd r1!, {r4-r6} //move thread'sfrom IRQ stack to SVC stack

	stmfd sp!, {r4-r6} //sp = thread stack (r1-r3)
	stmfd sp!, {r0}	 //push thread's r0 onto thread's stack
	stmfd sp!, {r3}	 //push thread's CPSR (IRQ's SPSR)

	//Determind interrupt_nesting -> FIQ
	//change to FIQ stack

	//KernelThreadManager.lpKernelThread->lpInitStackPointer
	//write back to sp, thread stack = sp
	mov r0, sp	//parameter 1-3: r0-r3, parameter 4, 5... : sp, sp+4...
	bl SetSP

	// re-enable FIQ, chagen to IRQ mode
	msr CPSR_c, #(0x80 | 0x12)	//NO_IRQ | IRQ32_MODE
	bl InterruptHandler

	//chagen to SVC32
	msr CPSR_c, #(0xc0 | 0x13)	//NO_INT | SVC32_MODE
	bl ExitInterrupt

	ldmfd sp!, {r4}
	msr SPSR_cxsf, r4
	ldmfd sp!, {r0-r12, lr, pc}^

__SaveAndSwitch:
	// save lpPrev thread's context
	mov r5,r1
	stmfd sp!, {lr}		
	stmfd sp!, {lr}
	stmfd sp!, {r0-r12, lr}
	mrs r4, SPSR
	stmfd sp!, {r4}
	mov r0, sp 		//r0: lpPrev->stack_ptr, lpPrev->stack_ptr = sp (restore)
	bl SetSP

	// restor next thread sp
	mov sp, r5		//r1: lpNext->stack_ptr, sp = lpNext->stack_ptr
	
	// restore next thread's context
	ldmfd sp!, {r4}
	msr SPSR_cxsf, r4
	ldmfd sp!, {r0-r12, lr, pc}^


__SwitchTo:
	mov sp,r0	//sp = nextthread->sp
        ldmfd sp!, {r4}		//restore CPSR
        msr SPSR_cxsf, r4
        ldmfd sp!, {r0-r12, lr, pc}^	//jump to new thread
