SRC_DIR		= ../..
include $(SRC_DIR)/../config.mk  

KERNEL_LIB  = $(BUILTIN_CRT_DIR)/lib.so
LD_SCRIPT 	= $(BUILTIN_CRT_DIR)/linkscript.ld
OUTPUT_DIR	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(BUILTIN_UTILS_DIR))
CFLAGS		+= -DENV
LDFLAGS		+= -static

UTILS_SRC	= $(wildcard $(BUILTIN_UTILS_DIR)/*.c $(BUILTIN_UTILS_DIR)/*.asm)
UTILS_OBJ	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(UTILS_SRC)))
UTILS_OUT	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.out,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(UTILS_SRC)))

$(KERNEL_LIB):
	make -C $(BUILTIN_CRT_DIR) clean
	make -C $(BUILTIN_CRT_DIR) lib
	make -C $(BUILTIN_CRT_DIR) clean

$(OUTPUT_DIR)/%.out : $(OUTPUT_DIR)/%.o
	$(shell mkdir -m 777 -p $(dir $@))
	make -C $(BUILTIN_CRT_DIR) clean
	make -C $(BUILTIN_CRT_DIR) lib
	make -C $(BUILTIN_CRT_DIR) clean
	$(LD) $(LDFLAGS) -T $(LD_SCRIPT) $(KERNEL_LIB) $< -o $@
	rm -rf $(UTILS_OBJ)

all: $(UTILS_OUT)

%.test: $(OUTPUT_DIR)/%.out
	test -n "$$(find $< -size -100k)"
	dd if=$< of=$(DATA_IMG) bs=512 count=200 seek=256 conv=notrunc

.PHONY: clean all
clean:
	rm -rf $(OUTPUT_DIR)