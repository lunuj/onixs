DIR_BUILD 	:= ../../build
DIR_SRC		:= ../../src
DIR_PWD		:= kernel

SRC		:= start.asm
OBJ		:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.o,$(notdir $(SRC)))
BIN		:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.bin,$(notdir $(SRC)))
IMG		:= $(DIR_BUILD)/master.img
KERNEL	:= $(DIR_BUILD)/kernel.bin
SYSTEM	:= $(DIR_BUILD)/system.bin
SYSTEM_MAP	:= $(DIR_BUILD)/system.map

ENTRY_POINT	:= 0x10000

img: $(IMG)

$(OBJ): $(DIR_BUILD)/$(DIR_PWD)/%.o: $(DIR_SRC)/$(DIR_PWD)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f elf32 $< -o $@

$(KERNEL): $(OBJ)
	$(shell mkdir -p $(dir $@))
	cp $(OBJ) $(KERNEL)
#	ld -arch i386 -static $^ -o $@

$(SYSTEM): $(KERNEL)
	$(shell mkdir -p $(dir $@))
	objcopy -O binary $< $@
	
$(SYSTEM_MAP): $(SYSTEM)
	$(shell mkdir -p $(dir $@))
	nm $< | sort > $@

$(IMG): $(SYSTEM)
ifneq ($(wildcard $(IMG)),)
else
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(SYSTEM) of=$@ bs=512 count=200 seek=10 conv=notrunc

.PHONY: clean
clean:
	rm -rf $(OBJ)