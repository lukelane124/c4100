//boot2.c





void noShut();
void show_eax();
void clearScr();
void writeScr(char *string, int row, int col);
void startup();

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
	startup();
	

	while(1) {}
	
}
