SRC_DIR		= ../..
include $(SRC_DIR)/../config.mk  

KERNEL_LIB  = lib.so
LD_SCRIPT 	= linkscript.ld
CFLAGS		+= -DENV
LDFLAGS		+= -static

OUTPUT_DIR	= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(BUILTIN_CRT_DIR))

CRT_SRC	= $(wildcard $(BUILTIN_CRT_DIR)/*.c $(BUILTIN_CRT_DIR)/*.asm)
CRT_OBJ	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(CRT_SRC)))

LIB_SRC = $(wildcard $(LIB_DIR)/*.c $(LIB_DIR)/*.asm)
LIB_OBJ	= $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(LIB_SRC)))

$(KERNEL_LIB): $(LIB_OBJ) $(CRT_OBJ)
	$(LD) $(LDFLAGS) -r $^ -o $@

lib: $(KERNEL_LIB)

.PHONY: clean lib
clean:
	rm -rf $(LIB_OBJ) $(CRT_OBJ)