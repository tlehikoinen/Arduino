/*
 * Snake.c
 * Created: 14.10.2021
 * Author : Tommi
 */ 

#define F_CPU 16000000UL
#define BAUD 38400
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/portpins.h>
#include <avr/interrupt.h>
#include "st7735/st7735.h"
#include "snake.h"
#include "buttons.h"
#include "spi.h"

#define True 1
#define False 0

#define BUTTON_DELAY_MS 40		// How long button needs to be pressed

// Function prototypes
void changeRandomSeed(uint16_t newSeed);
int checkForButton(int direction);
void checkForDirectionChange();
int compare_ints(const void* a, const void* b);
void handleHighScores(uint16_t newScore);
void initialiseHead(point * head);
void speedUp();
void speedDown();
void switchColorTheme(uint8_t theme);
void wait(uint16_t ms);

// Counter(s) for snake and buttons
volatile int snakeMoveCounter = 0;	
volatile int pushButtonDownCounter = 0;
volatile int pushButtonUpCounter = 0;
volatile int pushButtonLeftCounter = 0;
volatile int pushButtonRightCounter = 0;

// Keeps track if button is released or not
volatile uint8_t buttonUpDown = 0;
volatile uint8_t buttonDownDown = 0;
volatile uint8_t buttonLeftDown = 0;
volatile uint8_t buttonRightDown = 0;

// Snake is allowed to change its direction only once in the same position
volatile uint8_t snakeMovedInPosition = False;
uint8_t snakeSpeed[3] = {80, 60, 40};

// GAME STATES
enum gameState {
	startScreen,
	highscoreScreen,
	game,
	endScreen,
	settingsScreen,
	speedScreen,
	colorScreen,
};
	
enum menuState {
	play,
	highscores,
	settings,
	menuItems,
};

enum settingsState {
	difficulty,
	color,
	settingsItems,	
};

enum speedMenuStage {
	slow, medium, fast, speedItems,
};
	
enum highscoreState {
	highestscore,
	reset,
	highscoreItems,
};

enum colorTheme {
	theme_white,
	theme_black,
	theme_color,
	theme_items
};

// STATE PREDEFINITIONS
enum gameState currentGameState = startScreen;
enum menuState currentMenuState = play;
enum highscoreState currentHighscoreState = highestscore;
enum settingsState currentSettingsState = difficulty;

// States with ('*inMenu') in name store arrow position in menu
// States with ('*Selected') in name store X position in menu (Which is displayed for the actual selected one)
enum speedMenuStage currentSpeedInMenu;
enum speedMenuStage currentSelectedSpeed;
uint8_t currentSnakeSpeed;

enum colorTheme currentThemeInMenu;
enum colorTheme currentSelectedTheme;

	// ** COLORS **
volatile uint16_t SNAKE_BACKGROUND;
volatile uint16_t SNAKE_BODY;
volatile uint16_t SNAKE_APPLE;
volatile uint16_t MENU_BACKGROUND;
volatile uint16_t MENU_TEXT;
volatile uint16_t MENU_SELECT;

int main(void) {
	
	// LCD 1 - init struct
	// ----------------------------------------------------------
	// Chip Select
	struct signal cs = { .ddr = &DDRB, .port = &PORTB, .pin = 0 };
	// Back Light
	struct signal bl = { .ddr = &DDRK, .port = &PORTK, .pin = 3 };
	// Data / Command
	struct signal dc = { .ddr = &DDRK, .port = &PORTK, .pin = 1 };
	// Reset
	struct signal rs = { .ddr = &DDRK, .port = &PORTK, .pin = 0 };
	// LCD struct
	struct st7735 lcd1 = { .cs = &cs, .bl = &bl, .dc = &dc, .rs = &rs };

	ST7735_Init (&lcd1);
	
	buttonsInit();
	spi_init();
	
	sei();	// Enable global interrupts
	
	// TIMER0 (8-bit) SETUP
	// Generate interrupt every 0.001 second
	// 64 prescaling, top value 250, clear on match
	// 1/(16M/64) * 250 = 0.001 => Refresh rate (1/0.001)/ 8(rows) = 125 Hz
	// Inside interrupt call changeRow function
    TCCR0A |= (1<<WGM01);					//CTC Mode
	TCCR0B |= (1<<CS01) | (1<<CS00);		//Prescaler 64
	OCR0A = 250;							//Compare match A
	TIMSK0 |= 1<<OCIE0A;					//Compare Match A interrupt enabled
	
	//TIMER1 (10ms) SETUP
	// 1/(16M/256) * 625 = 0.01
	TCCR1B |= (1<<WGM12);					//CTC Mode
	TCCR1B |= (1<<CS12);					//Prescaler 256
	OCR1A = 625;
	TIMSK1 |= 1<<OCIE1A;					//Compare match A interrupt enabled
	
	// TIMER2 (10ms)
	TCCR3A = 0x00;							// normal operation, no output
	OCR3A  = 156;							// compare A value (156+1)/15625 = 10,05 ms
	TCNT3  = 0x00;							// reset counter
	TIFR3 |= 1<<OCF3A;						// clear IRQ flag writing 1's to register
	TCCR3B |= 1<<CS30 | 1<<CS32;			// prescaler 1024 (15625Hz), start counter
	TCCR3B  |= 1<<WGM32;
	
	// Seed is randomized at beginning of each game by difference in time taken to press start button
	uint32_t randomSeed = 0;

	//Snakes body is linked list, if apple is not eaten last element is moved to front, and if apple is eaten new location for apple is randomly generated
	point * nextPoint = (point *) malloc(sizeof(point));
	point * applePoint = (point *) malloc(sizeof(point));
	point * head = (point *) malloc(sizeof(point));
	head->x = 0;
	head->y = 0;
	head->next = NULL;
	
	// Highscores are stored in the array
	uint16_t highscoresFromMemory [5];
	
// ** FUNCTIONS FOR THE GAME THAT REQUIRE ACCESS TO MAIN VARIABLES **

	// Snake is moved by first updating values in nextPoint with returnNextCell()
	// Then checked for possible body collision and moveSnakeByOne() if there wasn't any
	void moveSnake() {
		returnNextCell(head, nextPoint);
		if (bodyCollision(head, nextPoint) == 1) {
			continueGame = 0;
		}
		moveSnakeByOne(lcd1, &head, nextPoint, applePoint, SNAKE_BACKGROUND, SNAKE_BODY, SNAKE_APPLE);
		snakeMovedInPosition = False;
	}
	
	void startGame() {
		ST7735_ClearScreen(&lcd1, SNAKE_BACKGROUND);
		changeRandomSeed(randomSeed);
		setNewAppleLocation(head, applePoint);
		appleToGrid(lcd1, applePoint, SNAKE_APPLE);
		currentGameState = game;
		snakeMoveCounter=0;
	}
	
	void showEndScreen() {
		uint8_t newHighscore = 0;
		char value[32];
		itoa(snakeLength,value,10);
		
		if (snakeLength > highscoresFromMemory[0]) 
			newHighscore = True;
			
		drawEndScreen(&lcd1, value, newHighscore, MENU_BACKGROUND, MENU_TEXT);
	}
	
	void playGame() {
		checkForDirectionChange();
		if (snakeMoveCounter > currentSnakeSpeed) {
			moveSnake();
			snakeMoveCounter = 0;
		}
	}
	
	void readHighScoresFromMemory() {
		for (uint8_t i=0; i<5; i++) {
			highscoresFromMemory[i] = eeprom_readWord(SNAKE_ADDRESSES[i]);
		}
		qsort(highscoresFromMemory, 5, 2, compare_ints);
	}
	
	void showHighscores() {
		readHighScoresFromMemory();
		currentGameState = highscoreScreen;
		currentHighscoreState = highestscore;
		drawHighscoreMenu(&lcd1, highscoresFromMemory, 5, MENU_BACKGROUND, MENU_TEXT, MENU_SELECT);
	}
	
	void showSettings() {
		drawSettingsMenu(&lcd1, MENU_BACKGROUND, MENU_TEXT);
		setArrow(&lcd1, currentSettingsState+1, MENU_SELECT);
		currentGameState = settingsScreen;
	}
	
	void showSpeedMenu() {
		drawSpeedMenu(&lcd1, MENU_BACKGROUND, MENU_TEXT);
		setArrow(&lcd1, currentSpeedInMenu+1, MENU_SELECT);
		drawSelectedSpeed(&lcd1, currentSelectedSpeed, MENU_BACKGROUND, MENU_SELECT);
		currentGameState = speedScreen;
	}
	
	void showColorMenu() {
		drawColorMenu(&lcd1, MENU_BACKGROUND, MENU_TEXT);
		setArrow(&lcd1, currentThemeInMenu+1, MENU_SELECT);
		drawSelectedSpeed(&lcd1, currentSelectedTheme, MENU_BACKGROUND, MENU_SELECT);
		currentGameState = colorScreen;
		
	}
	void showStartScreen(){
		drawSnakeMenu(&lcd1, currentMenuState, MENU_BACKGROUND, MENU_TEXT, MENU_SELECT);
		currentGameState = startScreen;
	}
	
	void handleHighScores() {
		uint8_t i = 0;
		// Reads and stores highest scores to an array
		readHighScoresFromMemory();
		
		// Increment i until value [0] or [<snakeLength] occurs
		for (; i<5; i++) {
			if (highscoresFromMemory[i] == 0 || highscoresFromMemory[i] < snakeLength)
				break;
		}
		// Return if smallest item is bigger than snakeLength
		if (highscoresFromMemory[i] > snakeLength)
			return;
		
		// snakeLength is inserted into highscores and smaller values are moved one position to right dropping the smallest (last) element from the list
		uint16_t temp = highscoresFromMemory[i];
		highscoresFromMemory[i] = snakeLength;
		i++;
		for (; i<5; i++) {
			uint16_t temp2 = highscoresFromMemory[i];
			highscoresFromMemory[i] = temp;
			temp = temp2;
		}
		eeprom_saveSnakeScores(highscoresFromMemory, 5);
	}
	
	void endGame() {
		showEndScreen();
		freeMemory(head);
		initialiseHead(head);
		handleHighScores();
		// changeDirection(direction) checks if direction change is valid, forcing start direction to be right
		snake_direction = right;
		snakeLength = 1;
		wait(2000);
		showStartScreen();
	}
	
	void resetHighScore() {
		eeprom_clearSnakeAddresses();
		showHighscores();
	}

	void changeSpeed() {
		currentSelectedSpeed = currentSpeedInMenu;
		currentSnakeSpeed = snakeSpeed[currentSelectedSpeed];
		drawSelectedSpeed(&lcd1, currentSelectedSpeed, MENU_BACKGROUND, MENU_SELECT);
		eeprom_writeByte(SNAKE_SPEED_ADDRESS, currentSelectedSpeed);
	}
	
	void checkForMenuButtons() {
		uint8_t previousRow;
		if (checkForButton(up) == 1) {
			switch(currentGameState) {
				case startScreen: {
					previousRow = currentMenuState;
					if(currentMenuState == 0) {return;} 
						else {currentMenuState = currentMenuState-1;}
					moveArrow(&lcd1, previousRow, currentMenuState, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case highscoreScreen: {
					previousRow = currentHighscoreState;
					if(currentHighscoreState == 0) {return;} 
						else {currentHighscoreState = currentHighscoreState-1;}
					moveArrow(&lcd1, previousRow, currentHighscoreState, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case settingsScreen: {
					previousRow = currentSettingsState;
					if(currentSettingsState == 0) {return;} 
						else {currentSettingsState = currentSettingsState-1;}
					moveArrow(&lcd1, previousRow+1, currentSettingsState+1, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case speedScreen: {
					previousRow = currentSpeedInMenu;
					if(currentSpeedInMenu == 0) {return;}
					else {currentSpeedInMenu = currentSpeedInMenu-1;}
					moveArrow(&lcd1, previousRow+1, currentSpeedInMenu+1, MENU_BACKGROUND, MENU_SELECT);
					break;
					
				}
				case colorScreen: {
					previousRow = currentThemeInMenu;
					if (currentThemeInMenu == 0) {return;}
					else {currentThemeInMenu = currentThemeInMenu-1;}
					moveArrow(&lcd1, previousRow+1, currentThemeInMenu+1, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				default:
					return;
			}
			wait(400);
		}
		
		if (checkForButton(down) == 1) {
			switch(currentGameState) {
				case startScreen: {
					previousRow = currentMenuState;
					if(currentMenuState == (menuItems-1)) {return;} 
						else {currentMenuState = currentMenuState+1;}
					moveArrow(&lcd1, previousRow, currentMenuState, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case highscoreScreen: {
					previousRow = currentHighscoreState;
					if(currentHighscoreState == (highscoreItems - 1)) {return;} 
						else {currentHighscoreState = currentHighscoreState + 1;}
					moveArrow(&lcd1, previousRow, currentHighscoreState, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case settingsScreen: {
					previousRow = currentSettingsState;
					if(currentSettingsState == (settingsItems -1)) {return;} 
						else {currentSettingsState = currentSettingsState+1;}
					moveArrow(&lcd1, previousRow+1, currentSettingsState+1, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				case speedScreen: {
					previousRow = currentSpeedInMenu;
					if(currentSpeedInMenu == (speedItems -1)) {return;}
					else {currentSpeedInMenu = currentSpeedInMenu+1;}
					moveArrow(&lcd1, previousRow+1, currentSpeedInMenu+1, MENU_BACKGROUND, MENU_SELECT);
					break;
					
				}
				case colorScreen: {
					previousRow = currentThemeInMenu;
					if (currentThemeInMenu == (theme_items -1)) {return;}
					else {currentThemeInMenu = currentThemeInMenu+1;}
					moveArrow(&lcd1, previousRow+1, currentThemeInMenu+1, MENU_BACKGROUND, MENU_SELECT);
					break;
				}
				default:
					return;
			}
			wait(400);
		}
	}
	
	uint8_t readSnakeSpeed() {
		uint8_t speed = eeprom_readByte(SNAKE_SPEED_ADDRESS);
		if (speed > 2 || speed < 0) {
			speed = 1;
		}
		return speed;
	}
	
	uint8_t readColorTheme() {
		uint8_t theme = eeprom_readByte(SNAKE_THEME_ADDRESS);
		if (theme > 2 || theme < 0) {
			theme = 0;
		}
		currentThemeInMenu = theme;
		switchColorTheme(theme);
		return theme;
	}
	
	void changeColor() {
		if (currentThemeInMenu == currentSelectedTheme) {return;}
		switchColorTheme(currentThemeInMenu);
		eeprom_writeByte(SNAKE_THEME_ADDRESS, currentThemeInMenu);
		showColorMenu();
	}
	
	// At beginning of the game, highscores are read and placed to integer array as sorted
	readHighScoresFromMemory();
	readColorTheme();
	// Read speed and color stages from the memory
	currentSelectedSpeed = readSnakeSpeed();
	currentSpeedInMenu = currentSelectedSpeed;
	
	currentSnakeSpeed = snakeSpeed[currentSelectedSpeed];
	showStartScreen();

	// ** GAME LOOP STARTS **
	while(1) {
		switch (currentGameState) {
			case startScreen: {
				randomSeed++;
				checkForMenuButtons();
				if (checkForButton(right) == 1) {
					switch (currentMenuState) {
						case play: {
							continueGame = True;
							startGame();
							break;
						}
						case highscores: {
							showHighscores();
							break;
						}
						case settings: {
							showSettings();
							break;
						}
						default:
							continue;
					}
				}
				break;
			}
			case highscoreScreen: {
				checkForMenuButtons();
				if (checkForButton(left) == 1) {
					showStartScreen();
				}
				if (checkForButton(right) == 1) {
					switch (currentHighscoreState) {
						case highestscore: {
							break;
						}
						case reset: {
							resetHighScore();
							break;
						}
						default:
							continue;
					}
				}
				break;
			}
			case game: {
				while (continueGame) {
					playGame();		
				}
				currentGameState = endScreen;
				break;	
			}
			case endScreen: {
				endGame();
				break;
			}
			case settingsScreen: {
				checkForMenuButtons();
				if (checkForButton(left) == 1) {
					showStartScreen();
				}
				if (checkForButton(right)) {
					switch(currentSettingsState) {
						case difficulty: {
							showSpeedMenu();
							break;
						}
						case color: {
							showColorMenu();
							break;
						}
						default: {
							continue;
						}
					}
				}
				break;
			}
			case speedScreen: {
				checkForMenuButtons();
				if (checkForButton(left) == 1) {
					showSettings();
				}
				if (checkForButton(right)) {
					changeSpeed();
				}
				break;
			}
			case colorScreen: {
				checkForMenuButtons();
				if (checkForButton(left) == 1) {
					showSettings();
				}
				if (checkForButton(right)) {
					changeColor();
				}
				break;
			}
		}
	}
} /* END main() */

void changeRandomSeed(uint16_t newSeed) {
	srand(newSeed);
}

int checkForButton(int direction) {
	switch (direction) {
		case left: {
			if (pushButtonLeftCounter > BUTTON_DELAY_MS) {
				return 1;
			}
			break;
		}
		case right: {
			if (pushButtonRightCounter > BUTTON_DELAY_MS) {
				return 1;
			}
			break;
		}
		case up: {
			if (pushButtonUpCounter > BUTTON_DELAY_MS) {
				return 1;
			}
			break;
		}
		case down: {
			if (pushButtonDownCounter > BUTTON_DELAY_MS) {
				return 1;
			}
			break;
		}
	}
	return 0;
}

void checkForDirectionChange() {
	// direction with value 5 doesn't call changeDirection() function
	uint8_t direction = 5;
	if (pushButtonUpCounter > BUTTON_DELAY_MS) {
		if (!buttonUpDown) {
			direction = up;
			pushButtonUpCounter = 0;
			buttonUpDown = 1;
		}
	}
	if (pushButtonDownCounter > BUTTON_DELAY_MS) {
		if (!buttonDownDown) {
			direction = down;
			pushButtonDownCounter = 0;
			buttonDownDown = 1;
		}
	}
	if (pushButtonLeftCounter > BUTTON_DELAY_MS) {
		if (!buttonLeftDown) {
			direction = left;
			pushButtonLeftCounter = 0;
			buttonLeftDown = 1;
		}
	}
	if (pushButtonRightCounter > BUTTON_DELAY_MS) {
		if (!buttonRightDown) {
			direction = right;
			pushButtonRightCounter = 0;
			buttonRightDown = 1;
		}
	}
	if (direction == 5) {return;}
		else {
			if (snakeMovedInPosition == False) {
				changeDirection(direction);
				snakeMovedInPosition = True;
			}
		}
}

int compare_ints(const void* a, const void* b) {
	uint16_t arg1 = *(const uint16_t*)a;
	uint16_t arg2 = *(const uint16_t*)b;
	
	if (arg1 < arg2) return +1;
	if (arg1 > arg2) return -1;
	return 0;
}

void initialiseHead(point * head){ 
	head->next = NULL;
	head->x = 0;
	head->y = 0;
}

void switchColorTheme(uint8_t theme) {
	switch(theme) {
		case theme_white: {
			currentSelectedTheme = theme_white;
			MENU_BACKGROUND = WHITE;
			MENU_SELECT = BLACK;
			MENU_TEXT = BLACK;
			SNAKE_APPLE = RED_DARK;
			SNAKE_BACKGROUND = WHITE;
			SNAKE_BODY = BLACK;
			break;
		}
		case theme_black: {
			currentSelectedTheme = theme_black;
			MENU_BACKGROUND = BLACK;
			MENU_SELECT = RED_ORANGE;
			MENU_TEXT = MAGENTA;
			SNAKE_APPLE = RED_DARK;
			SNAKE_BACKGROUND = BLACK;
			SNAKE_BODY = MAGENTA;
			break;
		}
		case theme_color: {
			currentSelectedTheme = theme_color;
			MENU_BACKGROUND = ORANGE;
			MENU_SELECT = RED_ORANGE;
			MENU_TEXT = YELLOW;
			SNAKE_APPLE = RED_ORANGE;
			SNAKE_BACKGROUND = ORANGE;
			SNAKE_BODY = YELLOW;
			break;
		}
		default: {
			break;
		}
	}
}
void wait(uint16_t ms) {
	uint16_t uCycles, i;
	
	// Timer3 OCF3A flag in TIFR3 is set every 10 ms
	uCycles = ms/10;					// count number of cycles needed

	TCNT3 = 0x00;						// reset counter
	TIFR3 |= (1<<OCF3A);				// clear flag
	
	for (i = 0; i < uCycles; i++) {
		while(!(TIFR3 & (1<<OCF3A)));	// wait until IRQ flag is set
		TIFR3 |= (1<<OCF3A);			// Must write 1 to clear flag!
		//TCNT3 = 0x00;					// CTC mode no need to clear
	}
}

// ** Interrupt vectors **
// TIMER0 1ms
ISR(TIMER0_COMPA_vect) {
	snakeMoveCounter++;	
}

// TIMER1 10ms
ISR(TIMER1_COMPA_vect) {
	if (BUTTON_PIN &= (1<<BUTTON_DOWN)) {
		pushButtonDownCounter+=10;
		} else {
		buttonDownDown = 0;
		pushButtonDownCounter = 0;
	}		
	if (BUTTON_PIN &= (1<<BUTTON_UP)) {
		pushButtonUpCounter+=10;
		} else {
		buttonUpDown = 0;
		pushButtonUpCounter = 0;
	}
	if (BUTTON_PIN &= (1<<BUTTON_LEFT)) {
		pushButtonLeftCounter+=10;
		} else {
		buttonLeftDown = 0;
		pushButtonLeftCounter = 0;
	}
	if (BUTTON_PIN &= (1<<BUTTON_RIGHT)) {
		pushButtonRightCounter+=10;
		} else {
		buttonRightDown = 0;
		pushButtonRightCounter = 0;
	}
}

