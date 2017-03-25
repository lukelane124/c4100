//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
#include"./DataStructs.h"
#include "./KernelFunctions.h"


//Prototype Section
void noShut();
void show_eax();
void clearScr();
void writeScr(char *string, int row, int col);
void default_handler();
void kdb_enter();
void causeFault();
char translate_scancode(int what);
void kbd_handler(unsigned int scancode);

//Global variable section
IDT idt_entry_t[256];
int current_idt_entry_count = 0;
keyboardBuffer_t keyboard_buffer;

//C Functions.




void initIDT() {
	int i;
	for (i = 0; i < 32; i++ ) {
		initIDTEntry(&idt_entry_t[current_idt_entry_count++],  (uint32_t) default_handler, 16, 0x8e);
	}

	initIDTEntry(&idt_entry_t[current_idt_entry_count++], 0, 16, 0x8e);
	initIDTEntry(&idt_entry_t[current_idt_entry_count++], (uint32_t) kdb_enter, 16, 0x8e);
	for (i = 34; i < 256; i++) {
		initIDTEntry(&idt_entry_t[current_idt_entry_count++], 0, 16, 0x8e);
	}
	gdt_r_t gdt;
	gdt.base = (uint32_t) idt_entry_t;
	gdt.limit= (uint16_t) sizeof(idt_entry_t) - 1;
	lidtr(&gdt);
}









//Main Program
int main() {
	char ch;
	char str[2];
	int row = 0;
	int col = 0;
	//show_eax();
	initializeQueue(&keyboard_buffer);
	clearScr();
	initIDT();
	setupPIC();
	asm("sti");
	str[1] = 0;
	
	

	while(1) {
		ch = k_getchar(&keyboard_buffer);

		if (ch == 0)
			continue;
		else if (ch == '\n') {
			col = 0;
			row++;
			if (row > 24) {
				row = 0;
				col = 0;
				//clearScr();
			}
			updateCursor(row * 80 + col);
		}
		else {
			str[0] = ch;
			writeScr(str, row, col);
			col++;
			if (col > 79) {
				col = 0;
				row++;
			}
			if (row > 24) {
				//clearScr();
				row = 0;
				col = 0;
			}
		}
	}
}

