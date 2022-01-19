
/*
 * buttons.h
 * Author: Tommi
 */ 

#define BUTTON_DDR DDRA
#define BUTTON_PORT PORTA
#define BUTTON_PIN PINA
#define BUTTON_DOWN PA0
#define BUTTON_UP PA2
#define BUTTON_LEFT PA3
#define BUTTON_RIGHT PA1

void buttonsInit() {
	BUTTON_DDR |= ~(1<<BUTTON_LEFT) | ~(1<<BUTTON_RIGHT) | ~(1<<BUTTON_UP) |~(1<<PA0);
}


