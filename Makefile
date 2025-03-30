BUILD_DIR 	= ./build
SRC_DIR		= ./src
IMG			= $(BUILD_DIR)/system.image
ISO			= $(BUILD_DIR)/system.iso

QEMUFLAG	= -m 32M\
			-audiodev dbus,id=hda \
			-machine pcspk-audiodev=hda \
			-rtc base=localtime \
			
QEMU_DISK	:= -boot c -drive file=$(IMG),if=ide,index=0,media=disk,format=raw
QEMU_CDROM	:= -boot d -cdrom $(ISO)
QEMU_CDROM_t:= file=$(ISO),media=cdrom
QEMU_DEBUG	:= -s -S

qemu-nodebug: img
	qemu-system-i386 $(QEMUFLAG) $(QEMU_DISK) 
qemu-c: img
	qemu-system-i386 $(QEMU_DEBUG) $(QEMU_DISK) $(QEMUFLAG)
qemu-d: iso
	qemu-system-i386 $(QEMU_DEBUG) $(QEMU_CDROM) $(QEMUFLAG)
$(VMDK): img
	qemu-img convert -pO vmdk $< $@

img:
	make -C $(SRC_DIR) img
iso:
	make -C $(SRC_DIR) iso

.PHONY: clean qemu img
clean:
	rm -rf $(BUILD_DIR)