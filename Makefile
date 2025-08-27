SRC_DIR		= ./src
include $(SRC_DIR)/../config.mk  

IMG			= $(BUILD_DIR)/system.image
DATA_IMG	= $(BUILD_DIR)/data.image
ISO			= $(BUILD_DIR)/system.iso

QEMUFLAG	= -m 32M\
			-audiodev dbus,id=hda \
			-machine pcspk-audiodev=hda \
			-rtc base=localtime \
			
QEMU_DISK	:= 	-boot c \
				-drive file=$(IMG),if=ide,index=0,media=disk,format=raw \
				-drive file=$(DATA_IMG),if=ide,index=1,media=disk,format=raw
QEMU_CDROM	:= -boot d -cdrom $(ISO)
QEMU_CDROM_t:= file=$(ISO),media=cdrom
QEMU_SERIAL1 := -chardev stdio,mux=on,id=com1 -serial chardev:com1
QEMU_SERIAL2 := -chardev vc,mux=on,id=com2 -serial chardev:com2
# QEMU_SERIAL2 := -chardev udp,mux=on,id=com2,port=7777,ipv4=on  -serial chardev:com2
QEMU_DEBUG	:= -s -S
qemu-nodebug: img
	qemu-system-i386 $(QEMUFLAG) $(QEMU_DISK) 
qemu-c: img
	qemu-system-i386 $(QEMU_DEBUG) $(QEMU_DISK) $(QEMUFLAG) $(QEMU_SERIAL1) $(QEMU_SERIAL2)
qemu-d: iso
	qemu-system-i386 $(QEMU_DEBUG) $(QEMU_CDROM) $(QEMUFLAG)
$(VMDK): img
	qemu-img convert -pO vmdk $< $@

img:
	make -C $(SRC_DIR) img
iso:
	make -C $(SRC_DIR) iso
env:
	make -C $(BUILTIN_CRT_DIR) all

.PHONY: clean qemu img
clean:
	rm -rf $(BUILD_DIR)/*.bin
	rm -rf $(BUILD_DIR)/system.image
	rm -rf $(BUILD_DIR)/kernel
	rm -rf $(BUILD_DIR)/fs
	rm -rf $(BUILD_DIR)/builtin
	rm -rf $(BUILD_DIR)/lib