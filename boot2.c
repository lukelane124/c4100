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
	//uint32_t temp = pushf_cli_fun();
	cur_vterm = (cur_vterm + 1) % pcb_count;
	if (!isProcessEmpty(&vterm_q)) {
		uint32_t temp  = (uint32_t) getProcess(&vterm_q);
		addProcess(&process_queue, temp);
	}
	char buff[5];
	convert_num(cur_vterm, buff);
	writeScr(buff, 0, 77);
	//popf_fun(temp);
	return;
}

void vterm_block_if_background() {
	//uint32_t temp = pushf_cli_fun();
	while(cur_vterm != currentProcess->pid) {
		if (!isProcessEmpty(&vterm_q)) {
			uint32_t temp = getProcess(&vterm_q);
		}	
		sched_fun(&vterm_q, currentProcess);	
	}
	
	//popf_fun(temp);
		return;
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
		writeln("Hit any key to continue.");
		//while(k_getchar(&keyboard_buffer) != '\n'){}
		while(1){
			gets(t,1);
			if (t[0] = '\n')
				break;
		}
		writeScr(s, 10, 0);
	}	
}

int stoi(char *s) {
	int i = 0;
	while (*s) {
		if (*s >= '0' && *s <= '9') {
			i *= 10;
			i += (*s++ - '0');
		} else {
			i = 0;
			break;
		}
	}
	return i;
}

int is_prime(int n) {
  int i;
  if (n == 2) return 1;
  for (i = 2; i < n - 1; i++) {
    if (n % i == 0) return 0;
  }	
  return 1;
}


void p2() {
	while(1) {	
		char buff[11];
		int i;
		int primeCount;
		currentProcess->row = 15;
		writeln("Please enter a less than 11 digit number to calculate the number of primes between that number and zero.");
		gets(buff, 11);
		int number = stoi(buff);
		for (i = 0; i < number; i++) {
			if (is_prime(number - i)) 
				primeCount++;
		}
		clearscr_box(11, 0, 14, 79);
		writeln("The # of primes between zero and");
		writeln(buff);
		writeln(" is ");
		convert_num(primeCount, buff);
		writeln(buff);
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
	uint32_t temp = pushf_cli_fun();
	int rows = r2-r1;
	int cols = c2-c1;
	int size = rows*80+cols;
	char mt[2000];
	if (size > 2000) {
		//set_cursor(0,0);
		writeScr("There is an error in clearscr_box with the size...", 5, 0);
	}
	for (rows = 0; rows < size; rows++) {
		mt[rows] = ' ';
	}
	mt[rows] = 0;
	writeScr(mt, r1, c1);
	popf_fun(temp);

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

