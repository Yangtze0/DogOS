NASM	= nasm
GCC		= gcc
LD		= ld
OBJCOPY	= objcopy
CAT		= cat
DD		= dd
RM		= rm
BOCHS	= bochs

default : dogos.img

boot.bin : boot.asm
	$(NASM) -fbin boot.asm -o boot.bin

dogos_head.bin : dogos_head.asm
	$(NASM) -fbin dogos_head.asm -o dogos_head.bin

nasm_func.o : nasm_func.asm
	$(NASM) -fmacho nasm_func.asm -o nasm_func.o

dogos.o : dogos.c
	$(GCC) -m32 -O0 -c dogos.c -o dogos.o

dogos : dogos.o nasm_func.o
	$(LD) dogos.o nasm_func.o -e _DogOS_main -o dogos

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
