DIR_BUILD	:= ../build
DIR_SRC		:= ../src
DIR_BOOT	:= ./boot
DIR_KERNEL	:= ./kernel
IMG		:= $(DIR_BUILD)/master.img
VMDK	:= $(DIR_BUILD)/master.vmdk

qemu: $(IMG)
	qemu-system-i386 -m 32M -boot c -hda $<
qemu-g: $(IMG)
	qemu-system-i386 -s -S -m 32M -boot c -hda $<
vmdk: $(VMDK)

$(IMG):
ifneq ($(wildcard $(IMG)),)
else
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif
	make -C $(DIR_BOOT)
	make -C $(DIR_KERNEL)

$(VMDK): $(IMG)
	qemu-img convert -pO vmdk $< $@

.PHONY: clean $(IMG)
clean:
	make -C $(DIR_BOOT) clean
	make -C $(DIR_KERNEL) clean