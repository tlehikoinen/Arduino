
/*
 * IncrementDecrementCounter.c
 *
 * Created: 21.4.2021 
 * Author: Tommi
 * Demonstrating, how I can display numbers 1-255 in binary using leds which are connected to SIPO shift register
 * Seven segment display is also connected and it increments when current number divided by 16 is even
 * 7SegmentDisplay.h and SR595.h contain information about connections to Uno
*/

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/portpins.h>
#include "7SegmentDisplay.h"
#include "SR595.h"

#define BUTTONS_DDR DDRD
#define BUTTONS_PORT PORTD
#define BUTTONS_PIN PIND
#define BUTTON_DECREMENT PD3
#define BUTTON_INCREMENT PD2

#define BUTTON_DECREMENT_ON (BUTTONS_PIN & 1<<BUTTON_DECREMENT)
#define BUTTON_INCREMENT_ON (BUTTONS_PIN & 1<<BUTTON_INCREMENT)
#define DELAY 30											// Delay for increment/decrement cycle
#define BUTTON_BOUNCE_DELAY 20

// Function predefinition
void waitMS(uint16_t delayMS);

	int main(void){
		
		/* 
		-- TIMER0 CALCULATIONS --
			Timer0 is 8 bit timer - counts from 0-255
				10ms / 255 = 39us -> one cycle must take more than 39us so it has time to reach 10ms
				1/(16M/x) < 39us -> solve for x (prescaler)
				x / 16 > 39
				x > 624
				-> prescaler must be 1024
			
			 With prescaler selected, we can count compare value for our counter (10ms)
				1/(16M/1024) = 64us
				10ms/64us = 156.25
				-> If compare value is set to 157 timer takes 10.048 ms
		*/
	
		TCCR0A = 1<<WGM01;							// Clear timer on compare match WGM02:0 = 2
		OCR0A = 157;								// Compare value for 10ms
		TCNT0  = 0x00;								// reset counter
		TIFR0 |= OCF0A;								// clear IRQ flag writing 1's to register
		TCCR0B = _BV(CS00) | _BV(CS02);				// prescaler 1024 
	
		/* DATA DIRECTIONS */
		SR595_DDR |= (1<<SR595_SER) | (1<<SR595_SRCLK) | (1<<SR595_RCLK);
		SEGMENT_AF_DDR |= 0xFF;								
		SEGMENT_G_DDR |= (1<<SEGMENT_G);
	
		uint8_t i = 0;								// 8 bit integer will flip to 0 when it goes over limit (255)
		
		while(1){
			
			if (BUTTON_INCREMENT_ON){
				waitMS(BUTTON_BOUNCE_DELAY);
					if(BUTTON_INCREMENT_ON) {
						i++;
						waitMS(DELAY);
					}
				
			}
			if (BUTTON_DECREMENT_ON){
				waitMS(BUTTON_BOUNCE_DELAY);
					if(BUTTON_DECREMENT_ON) {
						i--;
						waitMS(DELAY);
					}
			}
			
			SR595AddMultipleBits(i);
			SevenSegTurnNumberOn(i/16);

		} // END WHILE

	}	// END MAIN

	void waitMS (uint16_t delayMS) {
			
		TCNT0 = 0x00;								// Clear timer
		TIFR0 |= 1<<OCF0A;							// Clear flag by writing logic 1
		uint16_t i;
		uint16_t cycles = delayMS / 10;
	
		for (i = 0; i<cycles; i++)
		{
			while (!(TIFR0 & (1<<OCF0A)));			// While flag is not set stay in this loop
			TIFR0 |= 1<<OCF0A;						// Clear flag
		}

}