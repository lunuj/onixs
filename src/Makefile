DIR_BUILD	:= ../build
DIR_SRC		:= ../src
DIR_BOOT	:= ./boot
DIR_KERNEL	:= ./kernel
IMG		:= $(DIR_BUILD)/master.img

bochs: boot kernel
	bochs -q
boot: $(IMG)
	make -C $(DIR_BOOT)
kernel: $(IMG)
	make -C $(DIR_KERNEL)

$(IMG):
ifneq ($(wildcard $(IMG)),)
else
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif

.PHONY: clean
clean:
	make -C $(DIR_BOOT) clean
	make -C $(DIR_KERNEL) clean