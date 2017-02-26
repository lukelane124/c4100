//boot2.c
//Declare a struct with 64 bits... Might work, might not.
#include<stdint.h>
typedef struct IDT {
	uint16_t base_high;
	uint8_t access;
	uint8_t selector;
	uint16_t base_low;
  // unsigned int base_high : 16;
  // unsigned int access : 8;
  // unsigned int selector : 8;
  // unsigned int base_low : 16;
} idt;

idt idt_entry_t[256];
void initIDTEntry(idt *entry, uint32_t base, uint16_t selector, uint8_t access);

void noShut();
void show_eax();
void clearScr();
void writeScr(char *string, int row, int col);
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


int main() {
	show_eax();
	
	

	while(1) {}
	
}
