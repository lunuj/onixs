DIR_BUILD 	:= ./build
DIR_SRC		:= ./src
DIR_BOOT	:= $(DIR_SRC)/boot
DIR_KERNEL	:= $(DIR_SRC)/kernel

all: clean qemu
qemu:
	make -C $(DIR_SRC) qemu-g
bochs:
	make -C $(DIR_SRC) bochs
boot:
	make -C $(DIR_SRC) boot
kernel:
	make -C $(DIR_SRC) kernel
vmdk:
	make -C $(DIR_SRC) vmdk

.PHONY: clean qemu
clean:
	rm -rf $(DIR_BUILD)