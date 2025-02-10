DIR_BUILD 	:= ../../build
DIR_SRC		:= ../../src
DIR_PWD		:= boot
SRC		:= boot.asm loader.asm
OBJ		:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.bin,$(notdir $(SRC)))
IMG		:= $(DIR_BUILD)/master.img

img: $(IMG)
asm: $(OBJ)

$(OBJ): $(DIR_BUILD)/$(DIR_PWD)/%.bin: $(DIR_SRC)/$(DIR_PWD)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

$(IMG): $(OBJ)
ifneq ($(wildcard $(IMG)),)
else
	yes | bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
endif
	dd if=$(DIR_BUILD)/$(DIR_PWD)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(DIR_BUILD)/$(DIR_PWD)/loader.bin of=$@ bs=512 count=4 seek=2 conv=notrunc

.PHONY: clean clean_img clean_asm
clean: clean_img clean_asm
clean_img:
	rm -rf $(IMG)
clean_asm:
	rm -rf $(OBJ)