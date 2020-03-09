NASM	= nasm
GCC		= gcc -m32
LD		= ld
OBJCOPY	= objcopy
CAT		= cat
DD		= dd
RM		= rm
BOCHS	= bochs


default : dogos.img


# 模式规则
%.bin : %.asm
	$(NASM) -fbin $*.asm -o $*.bin

%.o : %.asm
	$(NASM) -fmacho $*.asm -o $*.o

%.o : %.c dogos.h
	$(GCC) -c $*.c -o $*.o


# 基本规则
OBJS =	dogos.o graphic.o dsctbl.o int.o fifo.o	memory.o \
		mouse.o keyboard.o nasm_func.o myfont.o

dogos : $(OBJS)		# To be improved : __nl_symbol_ptr
	$(LD) $(OBJS) -o dogos -e _DogOS_main	\
		-macosx_version_min 10.11 -preload -image_base 0x00100000 

dogos.bin : dogos
	$(OBJCOPY) -O binary dogos dogos.bin

dogos.image : boot.bin dogos_head.bin dogos.bin
	$(CAT) boot.bin dogos_head.bin dogos.bin > dogos.image

dogos.img : dogos.image
	$(DD) if=/dev/zero of=dogos.img bs=512 count=2880
	$(DD) if=dogos.image of=dogos.img conv=notrunc


.PHONY : default run clean

run:
	$(BOCHS) -q -f bochsrc

clean: 
	-$(RM) *.bin
	-$(RM) *.o
	-$(RM) dogos
	-$(RM) dogos.image
