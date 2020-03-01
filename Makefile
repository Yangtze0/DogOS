NASM	= nasm
GCC		= gcc
LD		= ld
OBJCOPY	= objcopy
CAT		= cat
DD		= dd
RM		= rm

default : dogos.img

boot.bin : boot.asm
	$(NASM) -fbin boot.asm -o boot.bin

dogos_head.bin : dogos_head.asm
	$(NASM) -fbin dogos_head.asm -o dogos_head.bin

dogos_func.o : dogos_func.asm
	$(NASM) -fmacho dogos_func.asm -o dogos_func.o

dogos.o : dogos.c
	$(GCC) -m32 -Os -Wall -c dogos.c -o dogos.o

dogos : dogos.o dogos_func.o
	$(LD) dogos.o dogos_func.o -e _DogOS_main -o dogos

dogos.bin : dogos
	$(OBJCOPY) -O binary dogos dogos.bin

dogos.image : boot.bin dogos_head.bin dogos.bin
	$(CAT) boot.bin dogos_head.bin dogos.bin > dogos.image

dogos.img : dogos.image
	$(DD) if=/dev/zero of=dogos.img bs=512 count=2880
	$(DD) if=dogos.image of=dogos.img conv=notrunc

.PHONY : default clean
clean: 
	-$(RM) *.bin
	-$(RM) *.o
	-$(RM) dogos
	-$(RM) dogos.image
