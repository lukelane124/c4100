//DataStructs.h
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
static char *cset_4_nums ="123456789";

#define CSET_ZERO 0x0B

#define CSET_NL 0x1c
#define CSET_SPC 0x39
#define CSET_RET 0xE
#define CSET_POINT_PRESSED 0x34
#define CSET_SLASH_PRESSED 0x35