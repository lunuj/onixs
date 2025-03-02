DIR_BUILD 	:= ../../build
DIR_SRC		:= ../../src
DIR_PWD		:= kernel

SRC_ASM	:= start.asm\
			io.asm\
			schedule.asm\
			handler.asm\
			
OBJ_ASM	:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.o,$(notdir $(SRC_ASM)))
BIN_ASM	:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.bin,$(notdir $(SRC_ASM)))

SRC_C	:= main.c\
			string.c\
			console.c\
			printk.c\
			vsprintf.c\
			assert.c\
			debug.c\
			global.c\
			task.c\
			interrupt.c\
			stdlib.c\
			clock.c\
			time.c\
			rtc.c\
			
OBJ_C	:= $(patsubst %.c,$(DIR_BUILD)/$(DIR_PWD)/%.o,$(notdir $(SRC_C)))
BIN_C	:= $(patsubst %.c,$(DIR_BUILD)/$(DIR_PWD)/%.bin,$(notdir $(SRC_C)))

IMG		:= $(DIR_BUILD)/master.img
KERNEL	:= $(DIR_BUILD)/kernel.bin
SYSTEM	:= $(DIR_BUILD)/system.bin
SYSTEM_MAP	:= $(DIR_BUILD)/system.map

CROSS_COMPILE	:= x86_64-elf-
CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
NM		:= $(CROSS_COMPILE)nm

CFLAGS	:= -m32 -e kernel_init
CFLAGS	+= -nostartfiles -nostdlib -nostdinc
CFLAGS	+= -fno-builtin -fno-pic -fno-pie -fno-stack-protector
LDFLAGS := -m elf_i386 -static

DEBUG	:= -g
INCLUDE	:= -I./include

ENTRY_POINT	:= 0x10000
img: $(IMG) $(SYSTEM) $(SYSTEM_MAP)
	dd if=$(SYSTEM) of=$(IMG) bs=512 count=200 seek=10 conv=notrunc

$(OBJ_ASM): $(DIR_BUILD)/$(DIR_PWD)/%.o: $(DIR_SRC)/$(DIR_PWD)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $(DEBUG) $< -o $@

$(OBJ_C): $(DIR_BUILD)/$(DIR_PWD)/%.o: $(DIR_SRC)/$(DIR_PWD)/%.c
	$(shell mkdir -p $(dir $@))
	$(CC) $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@ 

$(KERNEL): $(OBJ_ASM) $(OBJ_C)
	$(shell mkdir -p $(dir $@))
	$(LD) $(LDFLAGS) $^ -o $@  -Ttext $(ENTRY_POINT)

$(SYSTEM): $(KERNEL)
	$(shell mkdir -p $(dir $@))
	$(OBJCOPY) -O binary $< $@
	
$(SYSTEM_MAP): $(KERNEL)
	$(shell mkdir -p $(dir $@))
	$(NM) $< | sort > $@

.PHONY: clean img
clean:
	rm -rf $(OBJ_ASM)
	rm -rf $(OBJ_C)