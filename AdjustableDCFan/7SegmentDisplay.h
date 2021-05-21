/*
 * Created: 15/04/2021 
 * Author : Tommi
	This code adds support for 7 segment display, can be used to display numbers 0-15 (Dot is not coded)
 */ 

#define F_CPU 16000000UL // 16 MHz
#include <avr/io.h>
#include <avr/portpins.h>

#define SEGMENT_AF_DDR DDRB
#define SEGMENT_G_DDR DDRD	
#define SEGMENT_AF_PORT PORTB										
#define SEGMENT_G_PORT PORTD										
																	// 7 Segment Pin		Uno Pin
#define SEGMENT_A	PB0												// PIN7					PB0
#define SEGMENT_B	PB1												// PIN6					PB1
#define SEGMENT_C	PB2												// PIN4					PB2
#define SEGMENT_D	PB3												// PIN2					PB3
#define SEGMENT_E	PB4												// PIN1					PB4
#define SEGMENT_F	PB5												// PIN9					PB5
#define SEGMENT_G	PD7												// PIN10				PD7
// Turn segment on													
#define SEGMENT_A_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_A)			
#define SEGMENT_B_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_B)		
#define SEGMENT_C_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_C)			
#define SEGMENT_D_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_D)		
#define SEGMENT_E_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_E)			
#define SEGMENT_F_ON	SEGMENT_AF_PORT |= (1 << SEGMENT_F)			
#define SEGMENT_G_ON	SEGMENT_G_PORT  |= (1 << SEGMENT_G)			

	// Function prototypes
	void SevenSegDimSegments();										// Turn all segments off
	void SevenSegTurnNumberOn(uint8_t number);						// Display number 0-15 on screen (0-9, A-F)	
	void SevenSegTurnSegmentOn(char segment);						// Turn individual segment on A-G
	void SevenSegTurnLetterOn(char letter);
	
	// Dim every segment
	void SevenSegDimSegments(){
		SEGMENT_AF_PORT &= 0x00;									// Dims A-F
		SEGMENT_G_PORT &= ~(1 << SEGMENT_G);						// Dims G
	}
		
	// Turn individual segment on
	void SevenSegTurnSegmentOn(char segment){
			switch(segment) {
				case 'A' : {
					SEGMENT_AF_PORT |= (1 << PB0);
					break;
				}
				case 'B' : {
					SEGMENT_AF_PORT |= (1 << PB1);
					break;
				}
				case 'C' : {
					SEGMENT_AF_PORT |= (1 << PB2);
					break;
				}
				case 'D' : {
					SEGMENT_AF_PORT |= (1 << PB3);
					break;
				}
				case 'E' : {
					SEGMENT_AF_PORT |= (1 << PB4);
					break;
				}
				case 'F' : {
					SEGMENT_AF_PORT |= (1 << PB5);
					break;
				}
				case 'G' : {
					SEGMENT_G_PORT |= (1 << PD7);
					break;
				}
			}		// End switch
	}
	
	// Display number 0-9 + A-F
	void SevenSegTurnNumberOn(uint8_t number) {
			
		switch(number){
			if (number > 15 || number < 0)
				break;
				
			case 0 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON;
				break;
			}
			case 1 : {
				SevenSegDimSegments();
				SEGMENT_B_ON; SEGMENT_C_ON;
				break;
			}
			case 2 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_G_ON;
				break;
			}
			case 3 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_G_ON;
				break;
			}
			case 4 : {
				SevenSegDimSegments();
				SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 5: {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 6 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON; 
				break;
			}
			case 7 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON;
				break;
			}
			case 8 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 9 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 10 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 11 : {
				SevenSegDimSegments();
				SEGMENT_C_ON, SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 12 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON;
				break;
			}
			case 13 : {
				SevenSegDimSegments();
				SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_G_ON;
				break;
			}
			case 14 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 15 : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
		} // END SWITCH
	}
	
	void SevenSegTurnLetterOn(char letter){									
		
		switch (letter){
			if (letter != 'u' || letter != 'a')
				break;
					
			case 'a' : {
				SevenSegDimSegments();
				SEGMENT_A_ON; SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_E_ON; SEGMENT_F_ON; SEGMENT_G_ON;
				break;
			}
			case 'u' : {		
				SevenSegDimSegments();
				SEGMENT_B_ON; SEGMENT_C_ON; SEGMENT_D_ON; SEGMENT_E_ON; SEGMENT_F_ON;
				break;
			}
		}
	}
	


	
	    


