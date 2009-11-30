#define SDRAM_BASE_ADDRESS	0xa1000000

#define CFG_MECR_VAL		0x00000000
#define CFG_MCMEM0_VAL		0x00004204
#define CFG_MCMEM1_VAL		0x00000000
#define CFG_MCATT0_VAL		0x00010504
#define CFG_MCATT1_VAL		0x00000000
#define CFG_MCIO0_VAL		0x00008407
#define CFG_MCIO1_VAL		0x00000000
	
#define FIQ_STACK_SIZE		0x10000
#define IRQ_STACK_SIZE		0x10000
#define ABT_STACK_SIZE		0x10000
#define UND_STACK_SIZE		0x10000
#define SYS_STACK_SIZE		0x10000

//Interrupt Control, in datasheet 25-6
#define INT_BASE	0x40D00000	/* Interrupt controller IRQ pending register */
#define INT_REG(_offset_)	*(volatile unsigned long *)(INT_BASE + _offset_)	
#define INT_ICIP	0x00	/* Interrupt controller IRQ pending register */
#define INT_ICMR	0x04	/* Interrupt controller mask register */
#define INT_ICLR	0x08	/* Interrupt controller level register */
#define INT_ICFP	0x0C	/* Interrupt controller FIQ pending register */
#define INT_ICPR	0x10	/* Interrupt controller pending register */
#define INT_ICCR	0x14	/* Interrupt controller control register */

//Clocks Manager, in datasheet 3-94
#define CLK_BASE	0x41300000   
#define CLK_CCCR	0x00	/* Core Clock Configuration Register */
#define CLK_CKEN	0x04	/* Clock Enable Register */
#define CLK_OSCC	0x08	/* Oscillator Configuration Register */
#define CLK_REG(_addr_)	*(volatile unsigned long *)(CLK_BASE + _addr_)	

//Full Function UART, in datasheet 
#define FFUART_BASE	0x40100000
#define FFUART_REG(_addr_)	*(volatile unsigned long *)(FFUART_BASE + _addr_)	
#define FFRBR		0x00
#define FFTHR		0x00
#define FFDLL		0x00
#define FFIER		0x04
#define FFDLH		0x04
#define FFIIR		0x08
#define FFFCR		0x08
#define FFLCR		0x0C
#define FFMCR		0x10
#define FFLSR		0x14
#define FFMSR		0x18
#define FFSPR		0x1C
#define FFISR		0x20

//OS Timer
#define TMR_BASE	0x40A00000   
#define TMR_REG(_x_)	*(volatile unsigned long *)(TMR_BASE + _x_)	
#define TMR_OSMR0	0x00	/* OS timer match registers<3:0> */
#define TMR_OSMR1	0x04	/* */
#define TMR_OSMR2	0x08	/* */
#define TMR_OSMR3	0x0C	/* */
#define TMR_OSCR	0x10	/* OS timer counter register */
#define TMR_OSSR	0x14	/* OS timer status register */
#define TMR_OWER	0x18	/* OS timer watchdog enable register */
#define TMR_OIER	0x1C	/* OS timer interrupt enable register */
#define TMR_OSNR	0x20	/* OS Timer Snapshot register */


//GPIO Controller Register Summary
#define GPIO_BASE	0x40E00000

#define GAFR1_L		(*((volatile DWORD *)(GPIO_BASE+0x5c)))
#define GPDR1		(*((volatile DWORD *)(GPIO_BASE+0x10)))

#define GPIO_REG(_addr_)	*(volatile unsigned long *)(GPIO_BASE + _addr_)

#define GPIO_GPLR0	0x00	/* GPIO<31: 0>	status register */
#define GPIO_GPLR1	0x04	/* GPIO<63:32>	status register */
#define GPIO_GPLR2	0x08	/* GPIO<95:64>	status register */
#define GPIO_GPLR3	0x100	/* GPIO<120:96>	status register */

#define GPIO_GPDR0	0x0C	/* GPIO<31: 0>	direction register */
#define GPIO_GPDR1	0x10	/* GPIO<63:32>	direction register */
#define GPIO_GPDR2	0x14	/* GPIO<95:64>	direction register */
#define GPIO_GPDR2	0x10C	/* GPIO<120:96>	direction register */

#define GPIO_GPSR0	0x18	/* GPIO<31: 0>	output set register */
#define GPIO_GPSR1	0x1C	/* GPIO<63:32>	output set register */
#define GPIO_GPSR2	0x20	/* GPIO<95:64>	output set register */
#define GPIO_GPSR3	0x110	/* GPIO<120:96>	output set register */

#define GPIO_GPCR0	0x24	/* GPIO<31: 0>	output clear register */
#define GPIO_GPCR1	0x28	/* GPIO<63:32>	output clear register */
#define GPIO_GPCR2	0x2C	/* GPIO<95:64>	output clear register */
#define GPIO_GPCR3	0x124	/* GPIO<120:96>	output clear register */

#define GPIO_GRER0	0x30	/* GPIO<31: 0>	rising-edge detect register */
#define GPIO_GRER1	0x34	/* GPIO<63:32>  rising-edge detect register */
#define GPIO_GRER2	0x38	/* GPIO<95:65>	rising-edge detect register */
#define GPIO_GRER3	0x130	/* GPIO<120:96>	rising-edge detect register */

#define GPIO_GFER0	0x3C	/* GPIO<31: 0>	falling-edge detect register */
#define GPIO_GFER1	0x40	/* GPIO<63:32>	falling-edge detect register */
#define GPIO_GFER2	0x44	/* GPIO<95:64>	falling-edge detect register */
#define GPIO_GFER3	0x13C	/* GPIO<120:96>	falling-edge detect register */

#define GPIO_GEDR0	0x48	/* GPIO<31: 0>	edge detect status register */
#define GPIO_GEDR1	0x4C	/* GPIO<63:32>	edge detect status register */
#define GPIO_GEDR2	0x50	/* GPIO<95:64>	edge detect status register */
#define GPIO_GEDR3	0x148	/* GPIO<120:65>	edge detect status register */

#define GPIO_GAFR0L	0x54	/* GPIO<15: 0>	alternate function select
				   		register 0 Lower */
#define GPIO_GAFR0U	0x58	/* GPIO<31:16>	alternate function select
				   		register 0 Upper */
#define GPIO_GAFR1L	0x5C	/* GPIO<47:32>	alternate function select
				   		register 1 Lower */
#define GPIO_GAFR1U	0x60	/* GPIO<63:48>	alternate function select
				   		register 1 Upper */
#define GPIO_GAFR2L	0x64	/* GPIO<79:64>	alternate function select
				   		register 2 Lower */
#define GPIO_GAFR2U	0x68	/* GPIO80	alternate function select
				   		register 2 Upper */

//Memory Controller Register
#define MSC_BASE	0x48000000
#define MSC_REG(_addr_)	*(volatile unsigned long *)(MSC_BASE + _addr_)	

#define MSC_MDCNFG	0x00	/* SDRAM configuration register 0 */
#define MSC_MDREFR	0x04	/* SDRAM refresh control register */
#define MSC_MSC0	0x08	/* Static memory control register 0 */
#define MSC_MSC1	0x0C	/* Static memory control register 1 */
#define MSC_MSC2	0x10	/* Static memory control register 2 */
#define MSC_MECR	0x14	/* Expansion memory (PCMCIA / Compact Flash)
				   bus configuration register */
#define MSC_SXCNFG	0x1C	/* Synchronous static memory control register */
#define MSC_SXMRS	0x24	/* MRS value to be written to Synchronous
				   Flash or SMROM */
#define MSC_MCMEM0	0x28	/* Card interface Common Memory Space
				   Socket 0 Timing Configuration */
#define MSC_MCMEM1	0x2C	/* Card interface Common Memory Space
				   Socket 1 Timing Configuration */
#define MSC_MCATT0	0x30	/* Card interface Attribute Space
				   Socket 0 Timing Configuration */
#define MSC_MCATT1	0x34	/* Card interface Attribute Space
				   Socket 1 Timing Configuration */
#define MSC_MCIO0	0x38	/* Card interface I/O Space Socket 0
				   Timing Configuration */
#define MSC_MCIO1	0x3C	/* Card interface I/O Space Socket 1
				   Timing Configuration */
#define MSC_MDMRS	0x40	/* MRS value to be written to SDRAM */
#define MSC_BOOTDEF	0x44	/* Read-Only Boot-time register.
				   Contains BOOT_SEL and PKG_SEL values. */

