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
 */ 

#define F_CPU 16000000UL // 16 MHz
#include <avr/io.h>
#include <avr/portpins.h>
							
#define SR595_PORT PORTD	
#define SR595_DDR DDRD
#define SR595_SER PD6																// SER				PD6
#define SR595_SRCLK PD4																// SRCLK			PD4 
#define SR595_RCLK PD5																// RCLK				PD5

	// Function prototypes
	void SR595AddBit(int bit);														// Add single bit (0 or 1) into shift register
	void SR595PulseSR();															// Read SER and store the bit
	void SR595PulseLatch();															// Copy SR to latch
	void SR595AddMultipleBits(int number);											// Reads 8 first bits of a number and places them into latch register
		

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

