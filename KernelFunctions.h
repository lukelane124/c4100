//KernelInit.h
#include<stdint.h>
#include"./DataStructs.h"

void setupPIC() {
	//set up cascading mode:
	outportb(0x20, 0x11);		//Start 8259 master initialization.
	outportb(0xa0, 0x11);		//Start 8259 slave initialization.
	outportb(0x21, 0x20);		//Set master base interrupt vector (idt 32-38)
	outportb(0xa1, 0x28);		//Set slave base interrupt vector (idt 39-45)
	//Tell the master that he was a slave:
	outportb(0x21, 0x04);		//Set cascade...
	outportb(0xa1, 0x02);		//On IRQ2
	//Enable 8086 mode:
	outportb(0x21, 0x01);		//Finish 8259 initialization
	outportb(0xa1, 0x02);
	//Reset IRQ masks:
	outportb(0x21, 0x0);
	outportb(0xa1, 0x0);
	//Now, enable the keyboard IRQ only:
	outportb(0x21, 0xfc);		//Turn on the keyboard IRQ, and the timer.
	outportb(0xa1, 0xff);		//Turn off all others.
}


void initIDTEntry(IDT *entry, uint32_t base, uint16_t selector, uint8_t access){
	entry->base_low = (base & 0xffff);
	entry->access = access;
	entry->always0 = 0;
	entry->selector = selector;
	entry->base_high = ((base) >> 16) & 0xffff;
}

char translate_scancode(int code) {
	//return 'k'
	if (code > 0xF && code < 0x1a)
		return cset_1_chars[code - 0x10];
	if (code > 0x1d && code < 0x27)
		return cset_2_chars[code - 0x1e];
	if (code > 0x2b && code < 0x33)
		return cset_3_chars[code - 0x2c];
	if (code > 0x1 && code < 0xb)
		return cset_4_nums[code-0x02];
	if (code == CSET_ZERO)
		return '0';
	if (code == CSET_NL)
		return '\n';
	if (code == CSET_SPC)
		return ' ';
	if (code == CSET_RET)
		return 0xe;
	if (code == CSET_POINT_PRESSED)
		return '.';
	if (CSET_SLASH_PRESSED == code)
		return '/';
	else 
		return 0;
}

void updateCursor(unsigned int loc) {
	//int loc = 5;
  outportb(0x3D4, 0x0f);
  outportb(0x3d5, (loc & 0xFF));
  outportb(0x3D4, 0x0e);
  outportb(0x3d5, ((loc >> 8) & 0xFF));
}


int convert_num_h(unsigned int num, char buf[]) {
  if (num == 0) {
    return 0;
  }
  int idx = convert_num_h(num / 10, buf);
  buf[idx] = num % 10 + '0';
  buf[idx+1] = '\0';
  return idx + 1;
}

void convert_num(unsigned int num, char buf[]) {
  if (num == 0) {
    buf[0] = '0';
    buf[1] = '\0';
  } else {
    convert_num_h(num, buf);
  }
}

char k_getchar(keyboardBuffer_t *kb) {
	if (isEmpty(kb))
		return 0;
	else {
		return getItem(kb);
	}
}
extern int pcb_count;
extern PCB_t pcbs [10][sizeof(PCB_t)];
PCB_t* allocatePCB() {
	return pcbs[pcb_count];
}

extern uint32_t stacks [10][1024];
extern int next_stack;
uint32_t allocStack() {
	return (uint32_t) stacks[next_stack++];
}

extern PQ process_queue;
uint32_t createProcess(uint32_t ds, uint32_t ss, uint32_t stackTop,
	uint32_t cs, uint32_t processEntry) {
	uint32_t *sp = (uint32_t*)stackTop;
	sp -= 1;
	*sp = 0x0200;
	sp -= 1;
	*sp = cs;
	sp -= 1;
	*sp = processEntry;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = 0;
	sp -= 1;
	*sp = ds;
	sp -= 1;
	*sp = ds;
	sp -= 1;
	*sp = ds;
	sp -= 1;
	*sp = ds;
	PCB_t *pcb = allocatePCB();
	pcb->sp = (uint32_t) sp;
	pcb->pid = pcb_count++;
	addProcess(&process_queue, (uint32_t) pcb);
	return (uint32_t) pcb;
}