/*
 * Snake.c
 *
 * Created: 14.10.2021 17.56.20
 * Author : Opiskelu
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/portpins.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "rows.h"
#include "snake.h"
#include "SR595.h"

#define BUTTON_DOWN PC3
#define BUTTON_UP PC1
#define BUTTON_LEFT PC2
#define BUTTON_RIGHT  PC0

#define BUTTON_DELAY 10

int startGrid[8][8] = {
	{1,0,0,0,0,0,0,1},
 	{1,0,0,0,0,0,0,1},
 	{1,0,0,0,0,0,0,1},
 	{1,0,0,1,1,0,0,1},
 	{1,0,0,1,1,0,0,1},
 	{1,0,0,0,0,0,0,1},
 	{1,0,0,0,0,0,0,1},
 	{1,0,0,0,0,0,0,1}};
int happyGrid[8][8] = {
	{0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{1,0,0,0,0,0,0,1},
	{0,1,0,0,0,0,1,0},
	{0,0,1,1,1,1,0,0},
	{0,0,0,0,0,0,0,0}};
int sadGrid[8][8] = {
	{0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,1,1,0,0},
	{0,1,0,0,0,0,1,0},
	{1,0,0,0,0,0,0,1},
	{0,0,0,0,0,0,0,0}};		
int wowGrid[8][8] = {
	{0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,0},
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,1,1,0,0},
	{0,1,0,0,0,0,1,0},
	{0,1,0,0,0,0,1,0},
	{0,0,1,1,1,1,0,0},
	{0,0,0,0,0,0,0,0}}; 

volatile int snakeMoveCounter = 0;
volatile int pushButtonDownCounter = 0;
volatile int pushButtonUpCounter = 0;
volatile int pushButtonLeftCounter = 0;
volatile int pushButtonRightCounter = 0;

enum gameState {
	startScreen,
	game,
	endScreen,
	};

int main(void)
{
	sei();
	// TIMER0 (8-bit) SETUP
	// Generate interrupt every 0.00125 second
	// 64 prescaling, top value 20, clear on match
	// Inside interrupt call changeRow function
    TCCR0A |= (1<<WGM01);					//CTC Mode
	TCCR0B |= (1<<CS01) | (1<<CS00);		//Prescaler 64
	OCR0A = 20;								//Compare match A
	TIMSK0 |= 1<<OCIE0A;					//Compare Match A interrupt enabled
	
	//TIMER1 (10ms) SETUP
	TCCR1B |= (1<<WGM12);					//CTC Mode
	TCCR1B |= (1<<CS12);					//Prescaler 256
	OCR1AL = 0b01110001;					//Compare  match A
	OCR1AH = 0b00000010;
	TIMSK1 |= 1<<OCIE1A;					//Compare match A interrupt enabled
	
	//BUTTONS
	DDRC |= ~(1<<PC0) | ~(1<<PC1) | ~(1<<PC2) |~(1<<PC3);
	
	uint8_t randomSeed = 0;

	// Snakes body is linked list, if apple is not eaten last element is moved to front, and if apple is eaten new location for apple is randomly generated
	point * nextPoint = (point *) malloc(sizeof(point));
	point * applePoint = (point *) malloc(sizeof(point));
	point * head = (point *) malloc(sizeof(point));
	head->x = 0;
	head->y = 0;
	head->next = NULL;
	snakeToGrid(head);	

	SR595Initialise();
	DDRB |= (1<<PB5) | (1<<PB4);
	DDRD |= 0b11111100;
	PORTD &= 0x00;
	ROW_0_ON;
	SR595AddArrayReversedColor(grid[current_row]);
	
	int currentGameState = startScreen;
	
	void changeRandomSeed(uint16_t newSeed) {
		srand(newSeed);
	}
	
	void initialiseHead(point * head) {
		head->next = NULL;
		head->x = 0;
		head->y = 0;
	}	
	
	// Move snake by first updating values in nextPoint with returnNextCell()
	// Then check for possible body collision and moveSnakeByOne() if there wasn't any
	void moveSnake() {
		returnNextCell(head, nextPoint);
		if (bodyCollision(head, nextPoint) == 1) {
			continueGame = 0;
		}
		moveSnakeByOne(&head, nextPoint, applePoint);
	}
	
	void endGame() {
		if (snakeLength >= 63) {
			changeGrid(wowGrid);
		}
		else if (snakeLength > 30) {
			changeGrid(happyGrid);
		} else {
			changeGrid(sadGrid);
		}
		snakeLength = 0;
		// changeDirection function checks if direction change is valid, forcing start direction to be right
		snake_direction = right;
	}
	
	int checkForStartButton() {
		if (pushButtonRightCounter > BUTTON_DELAY) {
			return 1;
		}
		return 0;
	}
	
	void checkForDirectionChange() {
		if (pushButtonUpCounter > BUTTON_DELAY) {
			changeDirection(up);
			pushButtonUpCounter = 0;
		}
		if (pushButtonDownCounter > BUTTON_DELAY) {
			changeDirection(down);
			pushButtonDownCounter = 0;
		}
		if (pushButtonLeftCounter > BUTTON_DELAY) {
			changeDirection(left);
			pushButtonLeftCounter = 0;
		}
		if (pushButtonRightCounter > BUTTON_DELAY) {
			changeDirection(right);
			pushButtonRightCounter = 0;
		}
	}

	while(1) {
		switch (currentGameState) {
			case startScreen: {
				randomSeed++;
				if (checkForStartButton() == 1) {
					continueGame = 1;
					clearGrid();
					changeRandomSeed(randomSeed);
					setNewAppleLocation(head, applePoint);
					appleToGrid(applePoint);
					currentGameState = game;
				}
				break;
			}
			case game: {
				while (continueGame) {
				    checkForDirectionChange();
				    if (snakeMoveCounter > 2150) {
					    moveSnake();
					    snakeMoveCounter = 0;
				    }			
				}
				currentGameState = endScreen;
				break;	
			}
			case endScreen: {
				endGame();
				freeMemory(head);
				initialiseHead(head);
				_delay_ms(1000);
				currentGameState = startScreen;
			}
		}
	}
}

void changeRow() {
	changeCurrentRow();
	SR595AddArrayReversedColor(grid[current_row]);
	displayNextRow();
}

ISR(TIMER0_COMPA_vect) {
	SR595OutputDisabled();
	changeRow();
	SR595OutputEnabled();
	snakeMoveCounter++;
}

ISR(TIMER1_COMPA_vect) {
	if (PINC &= (1<<BUTTON_DOWN)) {
		pushButtonDownCounter++;
		} else {
		pushButtonDownCounter = 0;
	}		
	if (PINC &= (1<<BUTTON_UP)) {
		pushButtonUpCounter++;
		} else {
		pushButtonUpCounter = 0;
	}
	if (PINC &= (1<<BUTTON_LEFT)) {
		pushButtonLeftCounter++;
		} else {
		pushButtonLeftCounter = 0;
	}
	if (PINC &= (1<<BUTTON_RIGHT)) {
		pushButtonRightCounter++;
		} else {
		pushButtonRightCounter = 0;
	}
}
