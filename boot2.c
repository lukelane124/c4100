//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
struct IDT_s {
	uint16_t base_low;
	uint8_t selector;
	uint8_t always0;
	uint8_t access;
	uint16_t base_high;
	
  // unsigned int base_high : 16;
  // unsigned int access : 8;
  // unsigned int selector : 8;
  // unsigned int base_low : 16;
} __attribute__((packed));
typedef struct IDT_s IDT;

void default_handler();
void kdb_enter();

struct gdt_r_s {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct gdt_r_s gdt_r_t;

IDT idt_entry_t[256];
int current_idt_entry_count = 0;

void initIDTEntry(IDT *entry, uint32_t base, uint16_t selector, uint8_t access){
	entry->base_low = (base & 0xffff);
	entry->access = access;
	entry->always0 = 0;
	entry->selector = selector;
	entry->base_high = ((base) >> 16) & 0xffff;
}

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
	gdt.limit= (uint16_t) sizeof(idt_entry_t);
	lidtr(gdt);
}

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
	outportb(0x21, 0xfd);		//Turn on the keyboard IRQ.
	outportb(0xa1, 0xff);		//Turn off all others.
}


void noShut();
void show_eax();
void clearScr();
void writeScr(char *string, int row, int col);
//void outportb(unsigned int, unsigned int);
void updateCursor(unsigned int loc) {
	//int loc = 5;
  outportb(0x3D4, 0x0f);
  outportb(0x3d5, (loc & 0xFF));
  outportb(0x3D4, 0x0e);
  outportb(0x3d5, ((loc >> 8) & 0xFF));
}


/*inline unsigned char inportb(unsigned int port)
{
   unsigned char ret;
   asm volatile ("inb %%dx,%%al":"=a" (ret):"d" (port));
   return ret;
}*/
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


int main() {
	//show_eax();
	clearScr();
	initIDT();
	setupPIC();
	

	while(1) {}
	
}
