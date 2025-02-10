DIR_BUILD 	:= ./build
DIR_SRC		:= ./src
DIR_BOOT	:= $(DIR_SRC)/boot
DIR_KERNEL	:= $(DIR_SRC)/kernel

bochs:
	make -C $(DIR_SRC) bochs
boot:
	make -C $(DIR_SRC) boot
kernel:
	make -C $(DIR_SRC) kernel


.PHONY: clean
clean:
	rm -rf $(DIR_BUILD)