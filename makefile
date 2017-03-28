all: install alls
alls: boot2 boot1 

boot2: boot2.exe boot2.S
	objcopy -S -O binary boot2.exe boot2
boot1: boot1.asm boot1.asm
	nasm -l boot1.list -DENTRY=`./getaddr.sh main` boot1.asm

boot2_S.o: boot2.S
	gcc -g -m32 -c -masm=intel -o boot2_S.o boot2.S
boot2_c.o: boot2.c DataStructs.h
	gcc -g -m32 -c -o boot2_c.o boot2.c
boot2.exe: boot2_S.o boot2_c.o
	ld -g -melf_i386 -Ttext 0x10000 -e main -o boot2.exe boot2_S.o boot2_c.o


.phony clean:
clean: 
	rm *.o boot2.exe boot1 boot2 boot1.list a.img
 
install: alls
	echo "Building the floppy image and moving 1's."	
	bximage -fd -size=1.44 -q a.img

	mformat a:
	dd if=boot1 of=a.img bs=1 count=512 conv=notrunc
	mcopy -o boot2 a:BOOT2
.phony run: install
	echo "Start Qemu and move 0's."
	qemu-system-i386 -boot a -fda a.img &
.phony debug:
debug: boot2.exe boot1 boot2 install
	qemu-system-i386 -S -s -boot a -fda a.img &
	echo "At the gdb prompt enter \"target remote localhost:1234\""
	ddd boot2.exe &

