CROSS_COMPILE = arm-linux-
PXACORE_IMAGE = pxacore_image

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

MACH_CFLAGS = -march=armv5te -mtune=xscale -Wa,-mcpu=xscale \
	      -mno-thumb-interwork

CFLAGS = \
	-DDEBUG \
	-I./include/arch/arm/mach-pxa \
	-I./include \
	#-Wall -Werror \
	-fno-builtin \
	-O0 -g $(MACH_CFLAGS) 

LDSCRIPT := ld-script.lds
LDFLAGS = \
	-nostdlib -static -e _start \
	-p --no-undefined -X \
	-T $(LDSCRIPT)

OBJCOPYFLAGS = \
	-O binary \
	-R .note -R .note.gnu.build-id -R .comment \
	-S

SHELL_SRC = ./kernel/kernel.c

HW_DEP_ASM_SRC = \
	./arch/arm/mach-pxa/start.S \

HW_DEP_C_SRC = \
	./arch/arm/mach-pxa/arch_pxa.c \
	./drivers/serial.c \
	./lib/l_stdio.c \
#	./arch/arm/mach-pxa/port.c \
#	./lib/string.c \

KERNEL_SRC = \
	./kernel/ktmgr.c \
	./kernel/ktmgr2.c \
	./kernel/objmgr.c \
	./kernel/kmemmgr.c \
	./kernel/buffmgr.c \
	./kernel/objqueue.c \
	./kernel/shell.c \

#	./kernel/main.c \
#	./kernel/comqueue.c \
#	./kernel/dim.c \
#	./kernel/heap.c \
#	./kernel/hellocn.c \
#	./kernel/idehddrv.c \
#	./kernel/iomgr.c \
#	./kernel/kapi.c \
#	./kernel/keyhdlr.c \
#	./kernel/kmemmgr.c \
#	./kernel/kthread.c \
#	./kernel/ktmsg.c \
#	./kernel/mailbox.c \
#	./kernel/mailbox_.c \
#	./kernel/memmgr.c \
#	./kernel/objqueue.c \
#	./kernel/pageidx.c \
#	./kernel/perf.c \
#	./kernel/shell.c \
#	./kernel/statcpu.c \
#	./kernel/syn_mech.c \
#	./kernel/synobj.c \
#	./kernel/synobj_.c \
#	./kernel/system.c \
#	./kernel/taskctrl.c \
#	./kernel/timer.c \
#	./kernel/types.c \
#	./kernel/vmm.c \
#	./kernel/stat_s.c \
#	./kernel/sysd_s.c
 
SHELL_OBJ = $(SHELL_SRC:.c=.o)
HW_DEP_C_OBJ = $(HW_DEP_C_SRC:.c=.o)
HW_DEP_ASM_OBJ = $(HW_DEP_ASM_SRC:.S=.o)
KERNEL_OBJ = $(KERNEL_SRC:.c=.o)

all:	$(PXACORE_IMAGE).bin

$(PXACORE_IMAGE).bin: $(PXACORE_IMAGE).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@
	cp $(PXACORE_IMAGE).bin ../bootloader/src/

$(PXACORE_IMAGE).elf: $(SHELL_OBJ) $(HW_DEP_C_OBJ) $(HW_DEP_ASM_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) \
		-o $(PXACORE_IMAGE).elf \
		$(SHELL_OBJ) $(HW_DEP_C_OBJ) $(HW_DEP_ASM_OBJ) $(KERNEL_OBJ)

$(SHELL_OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(HW_DEP_C_OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(HW_DEP_ASM_OBJ) : %.o : %.S
	$(CC) -c $(CFLAGS) $< -o $@

$(KERNEL_OBJ) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -f ./arch/arm/mach-pxa/*.o \
	      ./kernel/*.o \
	      ./lib/*.o \
	      ./*.o \
	      $(PXACORE_IMAGE).bin $(PXACORE_IMAGE).elf \
	      flash-image
