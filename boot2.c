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
	writeScr("Process 1, thanks...", 0,0);
	while (1) {
		i += 1;
		convert_num(i, buff);
		writeScr(buff, 1, 0);
	}
}

void p2() {
	int i = 0;
	char buff[20];
	while (1) {
		writeScr("Process 2, thanks...", 10,0);
		i += 1;
		convert_num(i, buff);
		writeScr(buff, 11, 0);
	}
}


//Main Program
int main() {
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
	init_timer_device();
	asm("sti");	
	go();
}

