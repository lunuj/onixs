BUILD_DIR 	= ./build
SRC_DIR		= ./src
IMG			= $(BUILD_DIR)/master.img

QEMUFLAG	= -m 32M\
			-boot c\
			-audiodev dbus,id=hda \
			-machine pcspk-audiodev=hda \
			-rtc base=localtime \

qemu: $(IMG)
	qemu-system-i386 $(QEMUFLAG) -drive file=$<,if=ide,index=0,media=disk,format=raw
qemu-g: $(IMG)
	qemu-system-i386 -s -S $(QEMUFLAG) -drive file=$<,if=ide,index=0,media=disk,format=raw
vmdk: $(VMDK)

$(IMG):
	make -C $(SRC_DIR) img

.PHONY: clean qemu
clean:
	rm -rf $(BUILD_DIR)