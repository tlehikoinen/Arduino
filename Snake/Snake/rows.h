/*
* Functions for multiplexing rows on led matrix
*/

#define ROW_0 PB5
#define ROW_1 PB4
#define ROW_2 PD2
#define ROW_3 PD3
#define ROW_4 PD4
#define ROW_5 PD5
#define ROW_6 PD6
#define ROW_7 PD7

#define ROW_0_ON (PORTB |= (1<<PB5))
#define ROW_1_ON (PORTB |= (1<<PB4))
#define ROW_2_ON (PORTD |= (1<<ROW_2))
#define ROW_3_ON (PORTD |= (1<<ROW_3))
#define ROW_4_ON (PORTD |= (1<<ROW_4))
#define ROW_5_ON (PORTD |= (1<<ROW_5))
#define ROW_6_ON (PORTD |= (1<<ROW_6))
#define ROW_7_ON (PORTD |= (1<<ROW_7))

#define ROW_0_OFF (PORTB &= ~(1<<PB5))
#define ROW_1_OFF (PORTB &= ~(1<<PB4))
#define ROW_2_OFF (PORTD &= ~(1<<ROW_2))
#define ROW_3_OFF (PORTD &= ~(1<<ROW_3))
#define ROW_4_OFF (PORTD &= ~(1<<ROW_4))
#define ROW_5_OFF (PORTD &= ~(1<<ROW_5))
#define ROW_6_OFF (PORTD &= ~(1<<ROW_6))
#define ROW_7_OFF (PORTD &= ~(1<<ROW_7))

enum Rows {
	row0,
	row1,
	row2,
	row3,
	row4,
	row5,
	row6,
	row7
};

volatile int current_row = row0;

void changeCurrentRow() {
	if (current_row != row7) {
		current_row = current_row + 1;
		} else {
		current_row = row0;
	}
}

void displayNextRow() {
	switch (current_row) {
		case row0: {
			ROW_7_OFF;
			ROW_0_ON;
			break;
		}
		case row1: {
			ROW_0_OFF;
			ROW_1_ON;
			break;
		}
		case row2: {
			ROW_1_OFF;
			ROW_2_ON;
			break;
		}
		case row3: {
			ROW_2_OFF;
			ROW_3_ON;
			break;
		}
		case row4: {
			ROW_3_OFF;
			ROW_4_ON;
			break;
		}
		case row5: {
			ROW_4_OFF;
			ROW_5_ON;
			break;
		}
		case row6: {
			ROW_5_OFF;
			ROW_6_ON;
			break;
		}
		case row7: {
			ROW_6_OFF;
			ROW_7_ON;
			break;
		}
	}
}
