//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
//#include"./DataStructs.h"
#include "./KernelFunctions.h"


//Prototype Section
void noShut();
void show_eax();
void clearScr();
void writeScr(char *string, int row, int col);
void default_handler();
void kdb_enter();
void schedule();
void go();
void causeFault();
char translate_scancode(int what);
void kbd_handler(unsigned int scancode);
void init_timer_dev();

//Global variable section
extern uint8_t color;
extern PQ process_queue;
IDT idt_entry_t[256];						//IDT table.
int current_idt_entry_count = 0;			//Indes for filling table.
keyboardBuffer_t keyboard_buffer;			//Keyboard buffer.
uint32_t stacks [10][1024];					//Stack table for process allocation.
int next_stack = 0;								//Stack index.
extern PCB_t pcbs [10][sizeof(PCB_t)];				//PCB table for BPC allocation.
PCB_t pcbs [10][sizeof(PCB_t)];
int pcb_count = 0;								//Index for pcb table.
PQ process_queue;
PCB_t *currentProcess;						//Current running process.

//C Functions.
void initIDT() {
	int i;
	for (i = 0; i < 32; i++ ) {
		initIDTEntry(&idt_entry_t[current_idt_entry_count++],  (uint32_t) default_handler, 16, 0x8e);
	}

	initIDTEntry(&idt_entry_t[current_idt_entry_count++], (uint32_t) schedule, 16, 0x8e);
	initIDTEntry(&idt_entry_t[current_idt_entry_count++], (uint32_t) kdb_enter, 16, 0x8e);
	for (i = 34; i < 256; i++) {
		initIDTEntry(&idt_entry_t[current_idt_entry_count++], 0, 16, 0x8e);
	}
	gdt_r_t gdt;
	gdt.base = (uint32_t) idt_entry_t;
	gdt.limit= (uint16_t) sizeof(idt_entry_t) - 1;
	lidtr(&gdt);
}


#define DATA_SEL 8
#define STACK_SEL 24
#define STACK_SIZE 1024
#define CODE_SEL 16

void p1() {
	int i = 0;
	char buff[20];
	writeScr("Process 1, thanks...", 0, 0);
	while (1) {
		i += 1;
		convert_num(i, buff);
		writeScr(buff, 1, 0);
	}
}

void p2() {
	int i2 = 0;
	char buff2[20];
	writeScr("Process 2, thanks...", 10,0);
	while (1) {
		i2 += 1;
		convert_num(i2, buff2);
		writeScr(buff2, 11, 0);
	}
}


/*void schedule() {
	asm("pushad");
	asm("push ")
}*/
//Need to create a method to delay
//current process.

/*
Note that you will have to provide clearscr_box() and gets().  The function clearscr_box() clears a portion 
of the screen from r1, c1 until r2, c2 and has the prototype: void clearscr_box(int r1, int c1, int r2, int c2).
The function gets() reads a line of text.  In other words, it reads characters and concatenates them onto 
its first argument until it hits a new line, or has concatenated the number of letters specified by its 
second argument.  Its prototype is: int gets(char *s, int maxlen).
*/

int gets(char *s, int maxlen) {
	int size = 0;
	char c;
	while (size < maxlen) {
		c = k_getchar(&keyboard_buffer);
		if (c == '\n')
			return size;
		else {
			s[size++] = c;
		}
	}
}

void clearscr_box(int r1, int c1, int r2, int c2) {
	int row = r2-r1;
	int col = c2-c1;
	int size = row*80+col;
	char mt[2000];
	for (row = 0; row < size; row++) {
		mt[row] = ' ';
	}
	mt[row] = 0;
	writeScr(mt, r1, c1);

}

//Main Program
int main() {
	char charbuff[20];
	clearScr();
	color = 32;
	clearscr_box(0, 0, 2, 0);
	gets(&charbuff, 20);

	/*
	initializeQueue(&keyboard_buffer);
	clearScr();
	writeScr("Running ten processes", 0,0);
	clearScr();
	initIDT();
	setupPIC();
	initializeProcessQueue(&process_queue);
	asm("cli");
	uint32_t s = (uint32_t) allocStack();
	createProcess((uint32_t) DATA_SEL, (uint32_t) STACK_SEL, (uint32_t) (s + STACK_SIZE),
		(uint32_t) CODE_SEL, (uint32_t) p1);
	s = (uint32_t) allocStack();
	createProcess((uint32_t) DATA_SEL, (uint32_t) STACK_SEL, (uint32_t) (s + STACK_SIZE),
		(uint32_t) CODE_SEL, (uint32_t) p2);
	init_timer_device(50);
	//asm("sti");	
	go();
	*/
	while(1){};
}

