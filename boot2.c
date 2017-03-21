//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
#define true 1
#define false 0
//Structure declaration section
struct IDT_s {
	uint16_t base_low;
	uint16_t selector;
	uint8_t always0;
	uint8_t access;
	uint16_t base_high;
} __attribute__((packed));
typedef struct IDT_s IDT;

struct gdt_r_s {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed));
typedef struct gdt_r_s gdt_r_t;

//CircularQueue, and associated Functions.
#define MAX_ITEMS 128
struct keyboardBuffer_s {
    short top;
    short tail;
    short numItems;
    short mxItems;
    char data[MAX_ITEMS];
} __attribute__((packed));
typedef struct keyboardBuffer_s keyboardBuffer_t;
void initializeQueue(keyboardBuffer_t *q) {
	int i;
	q->top = 0;
	q->tail = 0;
	q->numItems = 0;
	q->mxItems = MAX_ITEMS ;
	for (i = 0; i < MAX_ITEMS; i++) {
		q->data[i] = 0;
	}
	return;
}

char isEmpty(keyboardBuffer_t *q) {
	if ( q->numItems == 0)
		return true;
	else
		return false;
}

char putItem(keyboardBuffer_t *q, unsigned char theItemValue) {
	if (q->numItems >= MAX_ITEMS)
		return false;
	else {
		q->data[q->tail++] = theItemValue;
		if (q->tail > MAX_ITEMS)
			q->tail = 0;
		q->numItems++;
		return true;
	}
}

unsigned char getItem(keyboardBuffer_t *q) {
	char ret;
	if (isEmpty(q))
		return 0;
	else{
		ret = q->data[q->top++];
		if (q->top > MAX_ITEMS)
			q->top = 0;
		q->numItems--;

	}
	return ret;	
}



enum CSET_1 {
Q_PRESSED = 0x10, W_PRESSED = 0x11, E_PRESSED = 0x12, R_PRESSED = 0x13,
T_PRESSED = 0x14, Y_PRESSED = 0x15, U_PRESSED = 0x16, I_PRESSED = 0x17,
O_PRESSED = 0x18, P_PRESSED = 0x19
};
static char* cset_1_chars = "qwertyuiop";

enum CSET_2 {
A_PRESSED = 0x1E, S_PRESSED = 0x1F, D_PRESSED = 0x20, F_PRESSED = 0x21,
G_PRESSED = 0x22, H_PRESSED = 0x23, J_PRESSED = 0x24, K_PRESSED = 0x25,
L_PRESSED = 0x26
};
static char *cset_2_chars = "asdfghjkl";

enum CSET_3 {
Z_PRESSED = 0x2C, X_PRESSED = 0x2D, C_PRESSED = 0x2E, V_PRESSED = 0x2F,
B_PRESSED = 0x30, N_PRESSED = 0x31, M_PRESSED = 0x32,
};
static char *cset_3_chars = "zxcvbnm";

enum CSET_NUMBERS {
ONE_PRESSED = 0x2, TWO_PRESSED = 0x3, THREE_PRESSED = 0x4,
FOUR_PRESSED = 0x5, FIVE_PRESSED = 0x6, SIX_PRESSED = 0x7,
SEVEN_PRESSED = 0x8, EIGHT_PRESSED = 0x9, NINE_PRESSED = 0xA
};

#define CSET_ZERO 0x0B

#define CSET_NL 0x1c
#define CSET_SPC 0x39
#define CSET_RET 0xE
#define CSET_POINT_PRESSED 0x34
#define CSET_SLASH_PRESSED 0x35

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
char translate_scancode(int code) {
	//return 'k'
	if (code > 0x9 && code < 0x1a)
		return cset_1_chars[code - 0x10];
	if (code > 0x1d && code < 0x27)
		return cset_2_chars[code - 0x1e];
	if (code > 0x2b && code < 0x33)
		return cset_3_chars[code - 0x2c];
	if (code > 0x1 && code < 0xb)
		return code - 1;
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
char k_getchar() {
	if (isEmpty(&keyboard_buffer))
		return 0;
	else {
		return getItem(&keyboard_buffer);
	}
}

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
	gdt.limit= (uint16_t) sizeof(idt_entry_t) - 1;
	lidtr(&gdt);
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


//void outportb(unsigned int, unsigned int);
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
		ch = k_getchar();

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

