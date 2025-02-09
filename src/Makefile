DIR_BUILD	:= ../../build
DIR_SRC		:= ../src
DIR_BOOT	:= ./boot

bochs:
	make -C $(DIR_BOOT) img
	bochs -q

.PHONY: clean
clean:
	make -C $(DIR_BOOT) clean