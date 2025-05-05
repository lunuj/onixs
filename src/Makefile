SRC_DIR		= .
BUILD_DIR	= $(SRC_DIR)/../build

BOOT_DIR	= $(SRC_DIR)/boot
KERNEL_DIR	= $(SRC_DIR)/kernel
LIB_DIR		= $(SRC_DIR)/lib
INCLUDE_DIR = $(SRC_DIR)/include

BOOT_NAME	 	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(BOOT_DIR)/boot.bin)
LOADER_NAME 	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(BOOT_DIR)/loader.bin)

BOOT_ASM_SRC	= $(wildcard $(BOOT_DIR)/*.asm)
BOOT_BIN	 	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(BOOT_ASM_SRC:.asm=.bin))

KERNEL_ASM_SRC 	= $(wildcard $(KERNEL_DIR)/*.asm)
KERNEL_ASM_OBJ 	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(KERNEL_ASM_SRC:.asm=.o))

LIB_ASM_SRC 	= $(wildcard $(LIB_DIR)/*.asm)
LIB_ASM_OBJ 	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(LIB_ASM_SRC:.asm=.o))

KERNEL_C_SRC 	= $(wildcard $(KERNEL_DIR)/*.c)
KERNEL_C_OBJ	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(KERNEL_C_SRC:.c=.o))

LIB_C_SRC 		= $(wildcard $(LIB_DIR)/*.c)
LIB_C_OBJ		= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(LIB_C_SRC:.c=.o))

KERNEL		= $(BUILD_DIR)/kernel.bin
SYSTEM_BIN	= $(BUILD_DIR)/system.bin
SYSTEM_HEX	= $(BUILD_DIR)/system.hex
SYSTEM_ISO	= $(BUILD_DIR)/system.iso
SYSTEM_MAP	= $(BUILD_DIR)/system.map
SYSTEM_IMG	= $(BUILD_DIR)/system.image
DATA_IMG	= $(BUILD_DIR)/data.image

CROSS_COMPILE	:= x86_64-elf-
CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
NM		:= $(CROSS_COMPILE)nm
GRUB	:= i686-elf-grub-

MULTIBOOT2	:= 0x10000
ENTRY_INDEX	:= 0x40
ENTRY_POINT	:= $(shell echo $$(($(MULTIBOOT2) + $(ENTRY_INDEX))))  # 注意两个$符号

CFLAGS	:= -m32
CFLAGS	+= -nostartfiles -nostdlib -nostdinc
CFLAGS	+= -fno-builtin -fno-pic -fno-pie -fno-stack-protector
LDFLAGS := -m elf_i386 -static -T linker.ld

DEBUG	:= -g
INCLUDE	:= -I./$(INCLUDE_DIR)

img: $(SYSTEM_IMG) $(DATA_IMG)
iso: $(SYSTEM_ISO)

$(BOOT_BIN): $(BUILD_DIR)/%.bin: %.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(KERNEL_ASM_OBJ): $(BUILD_DIR)/%.o: %.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $(DEBUG) $< -o $@

$(KERNEL_C_OBJ): $(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	$(CC) $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@ 
	
$(LIB_ASM_OBJ): $(BUILD_DIR)/%.o: %.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $(DEBUG) $< -o $@

$(LIB_C_OBJ): $(BUILD_DIR)/%.o: %.c
	$(shell mkdir -p $(dir $@))
	$(CC) $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@ 

$(KERNEL): $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(LIB_ASM_OBJ) $(LIB_C_OBJ)
	$(LD) $(LDFLAGS) $^ -o $@

$(SYSTEM_BIN): $(KERNEL)
	$(OBJCOPY) -O binary $< $@
$(SYSTEM_HEX): $(KERNEL)
	$(OBJCOPY) -O ihex $< $@
$(SYSTEM_MAP): $(KERNEL)
	$(NM) $< | sort > $@

$(DATA_IMG):
	yes | qemu-img create -f raw $@ 32K

$(SYSTEM_IMG): $(SYSTEM_BIN) $(SYSTEM_HEX) $(SYSTEM_MAP) $(BOOT_BIN)
ifneq ($(wildcard $(SYSTEM_IMG)),)
else
	$(shell mkdir -p $(dir $@))
	yes | qemu-img create -f raw $@ 16M
# yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BOOT_NAME) of=$(SYSTEM_IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADER_NAME) of=$(SYSTEM_IMG) bs=512 count=4 seek=2 conv=notrunc
	test -n "$$(find $(SYSTEM_BIN) -size -100k)"
	dd if=$(SYSTEM_BIN) of=$(SYSTEM_IMG) bs=512 count=200 seek=10 conv=notrunc

$(SYSTEM_ISO): $(KERNEL) $(SRC_DIR)/utils/grub.cfg $(SYSTEM_MAP)
	$(GRUB)file --is-x86-multiboot2 $<
	mkdir -p $(BUILD_DIR)/iso/boot/grub
	cp $< $(BUILD_DIR)/iso/boot
	cp $(SRC_DIR)/utils/grub.cfg $(BUILD_DIR)/iso/boot/grub
	$(GRUB)mkrescue -o $@ $(BUILD_DIR)/iso
	
.PHONY: clean img
clean:
	rm -rf $(BUILD_DIR)