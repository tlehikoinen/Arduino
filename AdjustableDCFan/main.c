
/*
 * FanController.c
 *
 * Created: 21.4.2021 
 * Author: Tommi
 * This is my Final Project solution for Temperature Controlled DC Fan.
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/portpins.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "7SegmentDisplay.h"
#include "SR595.h"
#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD
#define BUTTON_PIN PIND
#define BUTTON PD2
#define BUTTON_HIGH (BUTTON_PIN & 1<<BUTTON)


	// Function predefinition
	void waitMS(uint16_t delayMS);
	int readADC();
	void toggleState();						


	volatile int buttonCounter = 0;						// Keeps track if button has been pressed for 100 flag raises (10ms timer)

	enum {
		MODE_AUTO,
		MODE_USER
		};
		
	volatile int state = MODE_USER;						// User mode is default

	int main(void){
		
		// ADC
		ADCSRA |= _BV(ADEN);												// Enable ADC
		ADMUX  |= (_BV(REFS0));												// Vref = 5V
		ADCSRA |= (_BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));					// prescaler 128: fadc=16Mhz/128=125kHz
		ADCSRA |= 1<<ADSC;		
		
		// Timer0 (10ms)	
		TCCR0A = 1<<WGM01;													// Clear timer on compare match WGM02:0 = 2
		OCR0A = 157;														// Compare value for 10ms
		TCNT0  = 0x00;														// reset counter
		TIMSK0 |= (1<<OCIE0A);
		TIFR0 |= OCF0A;														// clear IRQ flag writing 1's to register
		TCCR0B = (1<<CS00) | (1<<CS02);										// prescaler 1024 
		
		// PWM Timer2 (controls PD3 aka OC2B) is 8 bit timer
		DDRD |= (1<<PD3);													// DDR out (0C2B) connected to MOSFET gate 
		TCCR2B |= (1<<CS21);												// Prescaling 8
		TCCR2A |= (1<<WGM20);												// Phase correct & OCR2A TOP
		TCCR2B |= (1<<WGM22);												// WGM22 bit (phase correct) is in different reg
		TCCR2A |= (1<<COM2B1);												// Clear 0C2B on compare match when up counting, set when down counting
		OCR2B = 0;															// Compared with TCNT2			
		OCR2A = 40;															// TOP value -- 2MHz/25000(desiredFreq) = 80/2 = 40
		//TIMSK2 |= (1<<OCIE2B);											// Output compare match B interrupt enabled (not set)
		
		sei();																// Global interrupts enabled
		
		// Port definitions
		BUTTON_DDR  &= ~(1<<BUTTON);										// Button as input, no internal pull up resistor is used
		SEGMENT_AF_DDR |= 0xFF;												// DDRB OUTPUT FOR SEGMENTS A-F
		SEGMENT_G_DDR |= (1 << SEGMENT_G);									// DDRD OUTPUT FOR SEGMENT G
		SR595_DDR |= (1<<SR595_SER) | (1<<SR595_SRCLK) | (1<<SR595_RCLK);	// SER, RCLK, SRCLK outputs
		
		// -- ADC COMPARE VALUE CALCULATIONS --
		double UNO_VOLTAGE = 4850;											// My Uno gave V reading of 4.85 V
		double maxADC = 1024;
		double TMPSensorVoltage[] = {710, 720, 730, 740, 750, 760, 770, 780};				// Readings for TMP36, system doesn't calibrate automatically so change accordingly!
		double potentiometerVoltage[] = {300, 800, 1400, 1900, 2500, 3000, 3500, 4000};		// Readings for Potentiometer
		double adcValueHolder;
		int TMP36Values[8];
		int potentiometerValues[8];
																						
		for (int i = 0; i<8; i++){												//Calculate ADC values for temperature sensor
			adcValueHolder = ((TMPSensorVoltage[i]/UNO_VOLTAGE)*maxADC);
			TMP36Values[i] = (int)adcValueHolder;
		}
		
		for (int i = 0; i<8; i++){												// Calculate ADC values for potentiometer
			adcValueHolder = ((potentiometerVoltage[i]/UNO_VOLTAGE)*maxADC);
			potentiometerValues[i] = (int)adcValueHolder;
		}
		// -- ADC CALCULATIONS END --
		
		int fanDutycycles[] = {0, 5, 9, 14, 19, 24, 28, 33, 40};				
		int SRLeds [] = {0, 128, 192, 224, 240, 248, 252, 254, 255};			// Leds light depending on fan speed
			
		SevenSegTurnLetterOn('u');												// User mode (default);
		
		while(1){									
														
			uint8_t overCounter = 0;															
			uint32_t adcReading = 0;
			
			if (buttonCounter > 100){											// buttonCounter increments (1) when 10ms timer raises flag and BUTTON is pressed
				toggleState();
				buttonCounter = 0;
			}
			
			for (uint8_t i=0;i<100;i++){										// Takes 100 ADC readings
				adcReading += readADC();
			}
			
			adcReading = adcReading/100;
																		
			switch (state) {						// Depending on the state, compares adcReading value to either precalculated potentiometer or tmp36 values
				case MODE_USER : {
					for (int i = 0; i<8; i++){
						if (adcReading>potentiometerValues[i]){
							overCounter++;
						}
					}
				break;	
				}
				case MODE_AUTO : {
					for (int i = 0; i<8; i++){
						if (adcReading>TMP36Values[i]){
							overCounter++;
						}
					}
				break;
				}
			}
			
			OCR2B = fanDutycycles[overCounter];						// Set PWM
			SR595AddMultipleBits(SRLeds[overCounter]);				// Light leds

		} // END WHILE

	}	// END MAIN
	
	
	ISR(TIMER0_COMPA_vect){
		if(BUTTON_HIGH){
			buttonCounter++;
		}else {
			buttonCounter = 0;
		}
	}
			
	void toggleState(){
		switch (state){
			case MODE_USER : {
				ADMUX  |= (1<<MUX0);				// ADC 1
				SevenSegTurnLetterOn('a');
				state = MODE_AUTO;
				break;
			}
			case MODE_AUTO : {
				ADMUX  &= ~(1<<MUX0);				// ADC 0
				SevenSegTurnLetterOn('u');
				state = MODE_USER;
				break;
			}
		}
	}
	
	int readADC(){				
		uint16_t uADCval = 0;
		
		ADCSRA |= _BV(ADEN);						// enable ADC
		ADCSRA |= _BV(ADSC);						// start conversion 

		while (ADCSRA & _BV(ADSC));					// wait until ADC ready (ADSC bit is reset)

		uADCval = ADC;								// read result
		ADCSRA &= ~(_BV(ADEN));						// stop ADC, no need to keep it up running
				
		return uADCval;
	}