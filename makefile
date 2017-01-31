all: boot2 boot1

boot2: boot2.exe
	objcopy -S -O binary boot2.exe boot2
boot1: boot1.asm boot1.asm
	nasm -l boot1.list -DENTRY=`./getaddr.sh main` boot1.asm

boot2_S.o: boot2.S
	gcc -g -m32 -c -masm=intel -o boot2_S.o boot2.S
boot2_c.o: boot2.c
	gcc -g -m32 -c -o boot2_c.o boot2.c
boot2.exe: boot2_S.o boot2_c.o
	ld -g -melf_i386 -Ttext 0x10000 -e main -o boot2.exe boot2_S.o boot2_c.o


.phony clean:
clean: 
	rm *.o boot2.exe boot1 boot2 boot1.list a.img
.phony install: 
install: all
	echo "Building the floppy image and moving 1's."	
	./build_image.sh
.phony run:
	echo "Start Qemu and move 0's."
	./Start_Qemu_NOdebug.sh
.phony debug:
debug:
	./Start_Qemu_debug.sh
