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

IMG			= $(BUILD_DIR)/master.img
KERNEL		= $(BUILD_DIR)/kernel.bin
SYSTEM		= $(BUILD_DIR)/system.bin
SYSTEM_MAP	= $(BUILD_DIR)/system.map

CROSS_COMPILE	:= x86_64-elf-
CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
NM		:= $(CROSS_COMPILE)nm

CFLAGS	:= -m32
CFLAGS	+= -nostartfiles -nostdlib -nostdinc
CFLAGS	+= -fno-builtin -fno-pic -fno-pie -fno-stack-protector
LDFLAGS := -m elf_i386 -static -T linker.ld

DEBUG	:= -g
INCLUDE	:= -I./$(INCLUDE_DIR)

ENTRY_POINT	:= 0x10000

img: $(IMG)

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

$(SYSTEM): $(KERNEL)
	$(OBJCOPY) -O binary $< $@
	
$(SYSTEM_MAP): $(KERNEL)
	$(NM) $< | sort > $@

$(IMG): $(SYSTEM) $(SYSTEM_MAP) $(BOOT_BIN)
ifneq ($(wildcard $(IMG)),)
else
	$(shell mkdir -p $(dir $@))
	yes |qemu-img create -f raw $@ 512K
# yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(BOOT_NAME) of=$(IMG) bs=512 count=1 conv=notrunc
	dd if=$(LOADER_NAME) of=$(IMG) bs=512 count=4 seek=2 conv=notrunc
	test -n "$$(find $(SYSTEM) -size -100k)"
	dd if=$(SYSTEM) of=$(IMG) bs=512 count=200 seek=10 conv=notrunc

$(VMDK): $(IMG)
	qemu-img convert -pO vmdk $< $@

.PHONY: clean img
clean:
	rm -rf $(BUILD_DIR)