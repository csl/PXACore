
CROSS_COMPILE = arm-linux-
PXACORE_IMAGE = pxacore_image

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

MACH_CFLAGS = -march=armv5te -mtune=xscale -Wa,-mcpu=xscale \
	      -mno-thumb-interwork

CFLAGS = \
	-I../../include/arch/arm/mach-pxa \
	-I../../include \
	-Wall -Werror \
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

SHELL_SRC = ./main.c

HW_DEP_ASM_SRC = \
	./arch/arm/mach-pxa/start.S \
	./arch/arm/mach-pxa/asm_port.S \

HW_DEP_C_SRC = \
	./drivers/serial.c \
	./lib/l_stdio.c \
	./lib/string.c \
	./arch/arm/mach-pxa/port.c

KERNEL_SRC = \
	./kernel/kernel.c \
	./kernel/comqueue.c \
	./kernel/dim.c \
	./kernel/heap.c \
	./kernel/hellocn.c \
	./kernel/idehddrv.c \
	./kernel/iomgr.c \
	./kernel/kapi.c \
	./kernel/keyhdlr.c \
	./kernel/kmemmgr.c \
	./kernel/kthread.c \
	./kernel/ktmgr.c \
	./kernel/ktmgr2.c \
	./kernel/ktmsg.c \
	./kernel/mailbox.c \
	./kernel/mailbox_.c \
	./kernel/main.c \
	./kernel/memmgr.c \
	./kernel/objmgr.c \
	./kernel/objqueue.c \
	./kernel/pageidx.c \
	./kernel/perf.c \
	./kernel/shell.c \
	./kernel/statcpu.c \
	./kernel/syn_mech.c \
	./kernel/synobj.c \
	./kernel/synobj_.c \
	./kernel/system.c \
	./kernel/taskctrl.c \
	./kernel/timer.c \
	./kernel/types.c \
	./kernel/vmm.c \
	./kernel/stat_s.c \
	./kernel/sysd_s.c \
 


SHELL_OBJ = $(SHELL_SRC:.c=.o)
HW_DEP_C_OBJ = $(HW_DEP_C_SRC:.c=.o)
HW_DEP_ASM_OBJ = $(HW_DEP_ASM_SRC:.S=.o)
KERNEL_OBJ = $(KERNEL_SRC:.c=.o)

$(CURT_IMAGE).bin: $(CURT_IMAGE).elf
	$(OBJCOPY) $(OBJCOPYFLAGS) $< $@

$(CURT_IMAGE).elf: $(SHELL_OBJ) $(HW_DEP_C_OBJ) $(HW_DEP_ASM_OBJ) $(KERNEL_OBJ)
	$(LD) $(LDFLAGS) \
		-o $(CURT_IMAGE).elf \
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
	rm -f ../../arch/arm/mach-pxa/*.o \
	      ../../kernel/*.o \
	      ../../device/*.o \
	      ../../lib/*.o \
	      ./*.o \
	      $(CURT_IMAGE).bin $(CURT_IMAGE).elf \
	      flash-image
