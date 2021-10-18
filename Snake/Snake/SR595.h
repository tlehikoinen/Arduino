/*
 * Created: 17/04/2021 
 *  Author: Tommi
 This code was made to control Shift register SN74HC595 (SR595) with Arduino Uno
 SR595 is SIPO, which means it takes data in serial and outputs it in parallel form
 Needs 3 output pins from microcontroller to control SER, SRCLK and RCLK pins
 
 SER is the data bit (0 or 1), when SRCLK is on rising edge it reads SER and pushes it into register bit place 0, moving other bits by one
 -- NOTE: When data is pushed from bit 7 it can be fed into another shift register by using serial out pin,
		  this code only supports first 8 bits and daisy chaining wasn't taken into account -- 
		  
RCLK on the rising edge copies bits from shift register and sends them to outputs (if Output enable (OE(active low))) is negative.
SRCLR clears shift register
 */ 

/*
#define F_CPU 16000000UL // 16 MHz
#include <avr/io.h>
#include <avr/portpins.h>*/
							
#define SR595_PORT PORTB	
#define SR595_DDR DDRB
#define SR595_SER PB0																// SER				PB0
#define SR595_SRCLK PB2																// SRCLK			PB2
#define SR595_RCLK PB1																// RCLK				PB1
#define SR595_SRCLR 0																// SRCLR			NULL
#define SR595_OE PB3																// OE				PB3 						

	// Function prototypes
    void SR595Initialise();
	void SR595AddBit(int bit);														// Add single bit (0 or 1) into shift register
	void SR595PulseSR();															// Read SER and store the bit
	void SR595PulseLatch();															// Copy SR to latch
	void SR595AddMultipleBits(int number);											// Reads 8 first bits of a number and places them into latch register
	void SR595ClearSR();	
	void SR595OutputEnabled();
	void SR595OutputDisabled();

    void SR595Initialise() {
        SR595_DDR |= (1<<SR595_SER) | (1<<SR595_SRCLK) | (1<<SR595_RCLK) | (1<<SR595_OE);
        SR595ClearSR();
    }

	void SR595AddMultipleBits(int number){											// Reads first 8 bits of integer number starting from MSB
		int bitHold;																// Holds the bit
		int i = 7;																	// Sends bits from MSB(7) to 0
	
		while (i >= 0){
			bitHold = (number & (1 << i)) ? 1 : 0;									// Compares numbers bits and bit at place i, if they match returns 1 otherwise returns 0
			SR595AddBit(bitHold);	
			i--;
		}
		SR595PulseLatch();															// 8 bits have been added to shift register before copying it to latch
	}
    
    void SR595AddArray(int array[]) {
       for (int i = 7; i>=0; i--) {
           SR595AddBit(array[i]);
       }
       SR595PulseLatch();
    }
	
	void SR595AddArrayReversedColor(int array[]) {
		int bitHold;
		for (int i = 7; i>=0; i--) {
			bitHold = array[i] == 0 ? 1 : 0;
			SR595AddBit(bitHold);
		}
		SR595PulseLatch();
	}

	void SR595PulseSR(){															// Add SER's bit to shift register
		SR595_PORT |= (1<<SR595_SRCLK);				
		SR595_PORT &= ~(1<<SR595_SRCLK);
	}

	void SR595PulseLatch(){															// Copy bits from shift register to latch register
		SR595_PORT |= (1<<SR595_RCLK);				
		SR595_PORT &= ~(1<<SR595_RCLK);
	}

	void SR595AddBit(int bit){														// Only bit is added doesn't copy to latch			
		if (bit == 0){
			SR595_PORT &= ~(1<<SR595_SER);
		}
		if (bit == 1) {
			SR595_PORT |= (1<<SR595_SER);
		}
		SR595PulseSR();
	}

    void SR595ClearSR() {
        SR595_PORT &= ~(1<<SR595_SRCLR);
        SR595_PORT |= (1<<SR595_SRCLR);
    }
	
	void SR595OutputEnabled() {
		SR595_PORT &= ~(1<<SR595_OE);
	}
	void SR595OutputDisabled() {
		SR595_PORT |= (1<<SR595_OE);
	}
	

