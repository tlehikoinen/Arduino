/*
* Functions to move snake
* Author: Tommi
*/
#include "st7735/st7735.h"

typedef struct point_xy {
    int x;
	int y;
    struct point_xy* next;
} point;

enum direction {
    up,
    down,
    left,
    right
};

uint8_t snake_direction = right;
uint16_t snakeLength = 1;
uint8_t continueGame = 1;

void setPointWithNumber(int number, point * point) {
    number = number - 1;
    point->x = number / 40;
    point->y = number % 32;
}

void appleToGrid(struct st7735 lcd, point * applePoint, uint16_t color_apple) {
	uint16_t x = applePoint->x;
	uint16_t y = applePoint->y;
	ST7735_DrawRectangle(&lcd, y*4, (y*4)+3, x*4, (x*4)+3, color_apple);
}

int returnRandomNumber() {  
    return rand() % (1280) + 1;
}

int validAppleLocation(point * head, point * applePoint) {
    point * current = head;

    while (current != NULL) {
        if (current->x == applePoint->x && current->y == applePoint->y)
            return 0;
        current = current->next;
    }
    return 1;
}

void setNewAppleLocation(point * head, point * applePoint) {
    //Gets random number between 1 and 1280 and converts it into xy value
    do {
        setPointWithNumber(returnRandomNumber(), applePoint);
    } while (validAppleLocation(head, applePoint) == 0);
}

void pushToList(point ** head, int x, int y) {
    point * new_point = (point *) malloc(sizeof(point));
    new_point->x = x;
    new_point->y = y;

    new_point->next = (*head);
    (*head) = new_point;
}

void addToEnd(point * head, int x, int y) {
    point * new_point = (point *) malloc(sizeof(point));
    new_point->x = x;
    new_point->y = y;
    point * current = head;
    
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_point;
}

void moveLastToFrontUpdatePosition(struct st7735 lcd, point ** head, point * nextPoint, uint16_t color_bckgrn, uint16_t color_body) {
    if (*head == NULL)
        return;
		
	uint16_t prev_x = (*head)->x;
	uint16_t prev_y = (*head)->y;
	uint16_t next_x = nextPoint->x;
	uint16_t next_y = nextPoint->y;
    if ((*head)->next == NULL) {
		ST7735_DrawRectangle(&lcd, prev_y*4, (prev_y*4)+3, prev_x*4, (prev_x*4)+3, color_bckgrn);
		ST7735_DrawRectangle(&lcd, next_y*4, (next_y*4)+3, next_x*4, (next_x*4)+3, color_body);
        (*head)->x = nextPoint->x;
        (*head)->y = nextPoint->y;
        return;
    }

    point * secLast = NULL;
    point * last = *head;

    while (last->next != NULL) {
        secLast = last;
        last = last->next;
    }
	ST7735_DrawRectangle(&lcd, last->y*4, (last->y*4)+3, last->x*4, (last->x*4)+3, color_bckgrn);
	ST7735_DrawRectangle(&lcd, next_y*4, (next_y*4)+3, next_x*4, (next_x*4)+3, color_body);
    secLast->next = NULL;
    last->next = *head;
    last->x = nextPoint->x;
    last->y = nextPoint->y;
    *head = last;
}

int validDirectionChange(int direction) {
    int validDirection = 1;
    switch (direction) {
        case up: {
            if (snake_direction == down)
                validDirection =  0;
            break;
        }
        case down: {
            if (snake_direction == up)
                validDirection =  0;
            break;
        }
        case left: {
            if (snake_direction == right)
                validDirection = 0;
            break;
        }
        case right: {
            if (snake_direction == left)
                validDirection = 0;
            break;
        }
    }
        return validDirection;
}

void changeDirection(int direction){
    if (validDirectionChange(direction) == 1) {
        snake_direction = direction;
    }
}

void returnNextCell(point * head, point * nextPoint){
	// Finds out heads next position based on direction and returns it
	switch (snake_direction){
		case left: {
			nextPoint->x = head->x;

			if (head->y == 0) {
				nextPoint->y =  39; 
			} else {
				nextPoint->y = head->y -1;
			}
			break;
		}
		case right: {
			nextPoint->x = head->x;    
			if (head->y == 39) {
				nextPoint->y = 0;
			} else {
				nextPoint->y = head->y + 1;
			}
			break;
		}
		case up: {
			nextPoint->y = head->y;               
			if (head->x == 0) {
				nextPoint->x = 31;
			} else {
				nextPoint->x = head->x -1;
			}
			break;
		}
		case down: {
			nextPoint->y = head->y;                
			if (head->x == 31) {
				nextPoint->x = 0;
			} else {
			  nextPoint->x = head->x + 1;
			}
			break;
		}        
	}
}

int appleEaten(point * applePoint, point * nextPoint) {
    if (applePoint->x == nextPoint->x && applePoint->y == nextPoint->y)
        return 1;
    return 0;
}

int bodyCollision(point * head, point * nextPoint) {
    point * current = head;
    // Comparing nextPoint to second last value, snake moves tail as it moves, therefore collision with last element doesn't matter
    while (current->next != NULL) {
        if (current->x == nextPoint->x && current->y == nextPoint->y) {
            continueGame = 0;        
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void moveSnakeByOne(struct st7735 lcd, point ** head, point * nextPoint, point * applePoint, uint16_t color_bckgrn, uint16_t color_body, uint16_t color_apple) {
    if (appleEaten(applePoint, nextPoint) == 1) {
		snakeLength++;
		if(snakeLength == 1280) {
			continueGame = 0;
		} else {
			pushToList(head, nextPoint->x, nextPoint->y);
			setNewAppleLocation(*head, applePoint);
			appleToGrid(lcd, applePoint, color_apple);
		}
    } else {
        moveLastToFrontUpdatePosition(lcd, head, nextPoint, color_bckgrn, color_body);
    }
}

void freeMemory(point * head) {
	point * current = head->next;
	point * tmp;
	
	while (current != NULL) {
		tmp = current;
		current = current->next;
		free(tmp);
	}

}