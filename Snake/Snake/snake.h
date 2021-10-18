#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    int x;
	int y;
    struct node* next;
} point;

enum direction {
    up,
    down,
    left,
    right
};

int snake_direction = right;
int snakeLength = 1;
int continueGame = 1;

int grid[8][8] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0}};
		
void changeGrid (int newGrid[8][8]) {
	memcpy(grid,newGrid, sizeof(grid));
}

void setPointWithNumber(int number, point * point) {
    number = number - 1;
    point->x = number / 8;
    point->y = number % 8;
}

void clearGrid() {
    for (int i=0; i<8; i++) {
        for (int j=0; j<8; j++) {
            if (grid[i][j] == 2)
                continue;
            grid[i][j] = 0;
        }
    }
}

void gridPointToHigh(int x, int y) {
    grid[x][y] = 1;
}

void gridPointToLow(int x, int y) {
    grid[x][y] = 0;
}

void gridPointToApple(int x, int y) {
    grid[x][y] = 2;
}

void snakeToGrid(point * head) {
   point * current = head;
   clearGrid();

   while (current != NULL) {
       gridPointToHigh(current->x, current->y);
       current = current->next;
   }
}

void appleToGrid(point * applePoint) {
    gridPointToApple(applePoint->x, applePoint->y);
}

int returnRandomNumber() {  
    return rand() % (63 + 1 - 0) + 1;
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
    //Gets random number between 1 and 64 and converts it into xy value
    do {
        setPointWithNumber(returnRandomNumber(), applePoint);
    } while (validAppleLocation(head, applePoint) == 0);
}

void printList(point * head) {
    point * current = head;

    while (current != NULL) {
        printf("\nx=%d, y=%d", current->x, current->y);
        current = current->next;
    }
    printf("\n__\n");
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

void moveLastToFrontUpdatePosition(point ** head, point * nextPoint) {
    if (*head == NULL)
        return;
    if ((*head)->next == NULL) {
		gridPointToLow((*head)->x, (*head)->y);
		gridPointToHigh(nextPoint->x, nextPoint->y);
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

	gridPointToLow(last->x, last->y);
	gridPointToHigh(nextPoint->x, nextPoint->y);
    secLast->next = NULL;
    last->next = *head;
    last->x = nextPoint->x;
    last->y = nextPoint->y;
    *head = last;
}

void printGrid() {
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
				nextPoint->y =  7; 
			} else {
				nextPoint->y = head->y -1;
			}
			break;
		}
		case right: {
			nextPoint->x = head->x;    
			if (head->y == 7) {
				nextPoint->y = 0;
			} else {
				nextPoint->y = head->y + 1;
			}
			break;
		}
		case up: {
			nextPoint->y = head->y;               
			if (head->x == 0) {
				nextPoint->x = 7;
			} else {
				nextPoint->x = head->x -1;
			}
			break;
		}
		case down: {
			nextPoint->y = head->y;                
			if (head->x == 7) {
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
    // Comparing nextPoint to second last value, snake moves tail as it moves, collision therefore doesn't matter
    while (current->next != NULL) {
        if (current->x == nextPoint->x && current->y == nextPoint->y) {
            continueGame = 0;        
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void moveSnakeByOne(point ** head, point * nextPoint, point * applePoint) {
    if (appleEaten(applePoint, nextPoint) == 1) {
		snakeLength++;
		if(snakeLength == 63) {
			continueGame = 0;
		} else {
			pushToList(head, nextPoint->x, nextPoint->y);
			setNewAppleLocation(*head, applePoint);
			appleToGrid(applePoint);
		}
    } else {
        moveLastToFrontUpdatePosition(head, nextPoint);
    }
}

//void gameLogic(point * nextPoint, point * applePoint, point * head) {
    //while(continueGame) {
            //returnNextCell(head, nextPoint);
            //if (bodyCollision(head, nextPoint) == 1) {
                //continueGame = 0;
                //break;
            //}
            //moveSnakeByOne(&head, nextPoint, applePoint);
            ////snakeToGrid(head);
    //}
//}

void freeMemory(point * head) {
	point * current = head->next;
	point * tmp;
	
	while (current != NULL) {
		tmp = current;
		current = current->next;
		free(tmp);
	}

}


