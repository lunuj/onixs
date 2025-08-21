SRC_DIR		= ../..
include $(SRC_DIR)/../config.mk  
ENV_OUT		= $(CRT_DIR)/env.out

KERNEL_LIB  = lib.so
LD_SCRIPT 	= linkscript.ld
CFLAGS		+= -DENV
LDFLAGS		+= -static

CRT_SRC	= $(wildcard $(CRT_DIR)/*.c $(CRT_DIR)/*.asm)
CRT_OBJ	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(CRT_SRC)))

LIB_SRC = $(wildcard $(LIB_DIR)/*.c $(LIB_DIR)/*.asm)
LIB_OBJ	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(LIB_SRC)))

$(KERNEL_LIB): $(LIB_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $@

$(ENV_OUT): $(CRT_OBJ) $(KERNEL_LIB)
	$(LD) $(LDFLAGS) -T $(LD_SCRIPT) $^ -o $@
	
all: $(ENV_OUT)
	dd if=$(ENV_OUT) of=$(DATA_IMG) bs=512 count=200 seek=16 conv=notrunc

.PHONY: clean all
clean:
	rm -rf $(BUILD_DIR)/*