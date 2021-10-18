# Snake with 8x8 LED Matrix

`<demo>` : <https://www.youtube.com/watch?v=oUopZZrUELU>

<img src="https://github.com/yellowpasta/Arduino/blob/master/Snake/build_picture.png" height=300px, width=420px>

### Implementation of legendary snake game

- Control snake with push buttons (left, right, up, down)  
- Eat randomly appearing objects and avoid hitting your tail  
- If snake collides with its own body and game ends, you can play again after feedback  
- Snake can go through wall to other side of grid  

## Connection and components

##### Components required:
- Arduino Uno
- SN74HC595 Shift Register
- 4 push buttons
- 8x 560 Ohm resistors
- 4x 10k Ohm resistors
- Breadboard, jump wires

##### Push buttons:
- Connect push buttons to breadboard with 10k Ohm pull down resistors
- Uno connections
    - PC0 - Right
    - PC1 - Up
    - PC2 - Left
    - PC3 - Down

##### 1588BS 8x8 LED Matrix
- Common anode
- Pinout (Numbers for rows, letters for columns)  
H G 1 A 3 F D 0  
4 6 B C 7 E 5 2

##### SN74HC595
- Shift register controls grounding for columns and is used to control which LED's are on for a row
- Uno to SN74HC595 connections
    - PB0 - SER
    - PB1 - RCLK
    - PB2 - SRCLK
    - PB3 - OE
- SN74HC595 to LED Matrix (column) connections
    - Qa - A
    - Qb - B
    - Qc - C
    - Qd - D
    - Qe - E
    - Qf - F
    - Qg - G
    - Qh - H

##### LED Matrix Rows
- Each row is on at different time and Uno is used for multiplexing
- Human eye can't tell difference and sees every row to be on at the same time
- Use 560 Ohm resistor to limit current, SN74HC595 is not suited for sinking high amounts of current
- Uno to LED Matrix (row) connections
    - PB5 - 0
    - PB4 - 1
    - PD2 - 2
    - PD3 - 3
    - PD4 - 4 
    - PD5 - 5
    - PD6 - 6
    - PD7 - 7
