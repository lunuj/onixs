DIR_BUILD 	:= ./build
DIR_SRC		:= ./src

bochs:
	make -C $(DIR_SRC) bochs

.PHONY: clean
clean:
	rm -rf $(DIR_BUILD)