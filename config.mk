# 文件目录
BUILD_DIR	= $(SRC_DIR)/../build
BOOT_DIR	= $(SRC_DIR)/boot
KERNEL_DIR	= $(SRC_DIR)/kernel
FS_DIR		= $(SRC_DIR)/fs
LIB_DIR		= $(SRC_DIR)/lib
UTILS_DIR 	= $(SRC_DIR)/utils
BUILTIN_DIR = $(SRC_DIR)/builtin
CRT_DIR	 	= $(BUILTIN_DIR)/crt
INCLUDE_DIR = $(SRC_DIR)/include
# 输出镜像
KERNEL		= $(BUILD_DIR)/kernel.bin
SYSTEM_BIN	= $(BUILD_DIR)/system.bin
SYSTEM_HEX	= $(BUILD_DIR)/system.hex
SYSTEM_ISO	= $(BUILD_DIR)/system.iso
SYSTEM_MAP	= $(BUILD_DIR)/system.map
SYSTEM_IMG	= $(BUILD_DIR)/system.image
DATA_IMG	= $(BUILD_DIR)/data.image
# 编译工具
CROSS_COMPILE	:= x86_64-elf-
CC		:= $(CROSS_COMPILE)gcc
LD		:= $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
NM		:= $(CROSS_COMPILE)nm
NASM	:= nasm
GRUB	:= i686-elf-grub-
# 编译标志
CFLAGS	:= -m32
CFLAGS	+= -nostartfiles -nostdlib -nostdinc
CFLAGS	+= -fno-builtin -fno-pic -fno-pie -fno-stack-protector
LDFLAGS := -m elf_i386
NFLAGS  += -f elf32
DEBUG	:= -g
INCLUDE	:= -I./$(INCLUDE_DIR)
# 编译命令
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	$(shell mkdir -m 777 -p $(dir $@))
	$(NASM) $(NFLAGS) $< -o $@
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(shell mkdir -m 777 -p $(dir $@))
	$(CC) $(CFLAGS) $(DEBUG) $(INCLUDE) -c $< -o $@ 