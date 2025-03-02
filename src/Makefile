DIR_BUILD	:= ../build
DIR_SRC		:= ../src
DIR_BOOT	:= ./boot
DIR_KERNEL	:= ./kernel
IMG		:= $(DIR_BUILD)/master.img
VMDK	:= $(DIR_BUILD)/master.vmdk
QEMUFLAG:= -m 32M\
			-boot c\
			-audiodev dbus,id=hda \
			-machine pcspk-audiodev=hda \
			-rtc base=localtime \

qemu: $(IMG)
	qemu-system-i386 $(QEMUFLAG)
qemu-g: $(IMG)
	qemu-system-i386 -s -S $(QEMUFLAG) -drive file=$<,if=ide,index=0,media=disk,format=raw
vmdk: $(VMDK)

$(IMG):
ifneq ($(wildcard $(IMG)),)
else
# yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	$(shell mkdir -p $(dir $@))
	yes |qemu-img create -f raw $@ 512K
endif
	make -C $(DIR_BOOT) img
	make -C $(DIR_KERNEL) img

$(VMDK): $(IMG)
	qemu-img convert -pO vmdk $< $@

.PHONY: clean $(IMG)
clean:
	make -C $(DIR_BOOT) clean
	make -C $(DIR_KERNEL) clean