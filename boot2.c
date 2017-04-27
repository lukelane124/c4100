//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
//#include"./DataStructs.h"
#include "./KernelFunctions.h"
#define true 1
#define false 0

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
void clearscr_box(int r1, int c1, int r2, int c2);
int gets(char *s, int maxlen);
void writeln(char *string);
uint32_t pushf_cli_fun();
void popf_fun(uint32_t eflags);
void vterm_block_if_background();
void sched_fun(PQ*, PCB_t*);

//Global variable section
extern uint8_t color;
extern PQ process_queue;
IDT idt_entry_t[256];						//IDT table.
int current_idt_entry_count = 0;			//Indes for filling table.
keyboardBuffer_t keyboard_buffer;			//Keyboard buffer.
uint32_t stacks [10][1024];					//Stack table for process allocation.
int next_stack = 0;								//Stack index.
//extern PCB_t pcbs [10][sizeof(PCB_t)];				//PCB table for BPC allocation.
PCB_t pcbs [10];
//[sizeof(PCB_t)];
int pcb_count = 0;								//Index for pcb table.
PQ process_queue;
PCB_t *currentProcess;						//Current running process.
int _row = 0;
int _col = 0;
int cur_vterm = 0;
PQ vterm_q;


uint32_t modulo(int a, int b) {
	return a%b;
}
void vterm_foreground_next() {
	uint32_t temp = pushf_cli_fun();
	cur_vterm = (cur_vterm + 1) % pcb_count;
	char buff[5];
	convert_num(cur_vterm, buff);
	writeScr(buff, 0, 77);
	if (!isProcessEmpty(&vterm_q)) {
		uint32_t temp  = (uint32_t) getProcess(&vterm_q);
		addProcess(&process_queue, temp);
	}
	popf_fun(temp);
	return;
}

void vterm_block_if_background() {
	while(cur_vterm != currentProcess->pid) {
		if (!isProcessEmpty(&vterm_q)) {
			uint32_t temp = getProcess(&vterm_q);
		}		
		sched_fun(&vterm_q, currentProcess);
		go();
	}
}


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

void set_cursor(int row, int col) {
	currentProcess->row = row;
	currentProcess->col = col;
}


void p1() {
	char *msg = "Process p1: ";
	writeln(msg);
	while (1) {
		clearscr_box(0,0, 9, 79);
		set_cursor(0,0);
		writeln("Please enter a string less than 20 characters:");
		//set_cursor(0, 0);
		char s[21];
		char t[21];
		gets(s, 20);
		writeln("You entered: ");
		writeln(s);
		writeln("Hit enter to continue.");
		while(k_getchar(&keyboard_buffer) != '\n'){}
		writeScr(s, 10, 0);
	}	
}

void p2() {
	int i2 = 0;
	char buff2[20];
	writeScr("Process 2, thanks...", 12,0);
	while (1) {
		i2 += 1;
		convert_num(i2, buff2);
		writeScr(buff2, 13, 0);
	}
}

void writeln(char *string) {
	if (currentProcess->col != 0)
		currentProcess->col = 0;
	writeScr(string, currentProcess->row, currentProcess->col);
	currentProcess->row++;
	if (currentProcess->row > 40)
		currentProcess->row = 0;


}

int gets(char *s, int maxlen) {
	int size = 0;
	char zero = 0;
	char c;
	char zero1 = 0;
	while (size < maxlen) {
		uint32_t eflags = pushf_cli_fun();
		vterm_block_if_background();
		c = k_getchar(&keyboard_buffer);
		popf_fun(eflags);
		if (c == '\n') 
			break;
		else if (c == 0)
			continue;
		else {
			s[size++] = c;
			writeScr(&c, currentProcess->row, currentProcess->col++);
			//_col++;
		}
	}
	s[size] = 0;
	return size;
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
	initializeQueue(&keyboard_buffer);
	initializeProcessQueue(&vterm_q);
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
	//addProcess(&vterm_q, getProcess(&process_queue));
	init_timer_device(50);
	//asm("sti");	
	go();
	
	while(1){};
}

