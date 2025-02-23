DIR_BUILD 	:= ../../build
DIR_SRC		:= ../../src
DIR_PWD		:= boot
SRC		:= boot.asm loader.asm
OBJ		:= $(patsubst %.asm,$(DIR_BUILD)/$(DIR_PWD)/%.bin,$(notdir $(SRC)))
IMG		:= $(DIR_BUILD)/master.img

img: $(OBJ) 
	dd if=$(DIR_BUILD)/$(DIR_PWD)/boot.bin of=$(IMG) bs=512 count=1 conv=notrunc
	dd if=$(DIR_BUILD)/$(DIR_PWD)/loader.bin of=$(IMG) bs=512 count=4 seek=2 conv=notrunc

$(OBJ): $(DIR_BUILD)/$(DIR_PWD)/%.bin: $(DIR_SRC)/$(DIR_PWD)/%.asm
	$(shell mkdir -p $(dir $@))
	nasm -f bin $< -o $@

.PHONY: clean img
clean:
	rm -rf $(OBJ)