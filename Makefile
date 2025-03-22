BUILD_DIR 	= ./build
SRC_DIR		= ./src
IMG			= $(BUILD_DIR)/master.img

QEMUFLAG	= -m 32M\
			-boot c\
			-audiodev dbus,id=hda \
			-machine pcspk-audiodev=hda \
			-rtc base=localtime \

qemu: img
	qemu-system-i386 $(QEMUFLAG) -drive file=$(IMG),if=ide,index=0,media=disk,format=raw
qemu-g: img
	qemu-system-i386 -s -S $(QEMUFLAG) -drive file=$(IMG),if=ide,index=0,media=disk,format=raw
$(VMDK): img
	qemu-img convert -pO vmdk $< $@

img:
	make -C $(SRC_DIR) img

.PHONY: clean qemu img
clean:
	rm -rf $(BUILD_DIR)