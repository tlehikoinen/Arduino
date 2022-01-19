
/*
 *  spi.h
 *  Created: 16.11.2021
 *  Author: Tommi with help from following repository:
 *  github.com/hexagon5un/AVR-Programming/tree/master/Chapter16_SPI/spiEEPROMDemo
 *  Functions to operate with 25C320 (32kbit) EEPROM via SPI interface
 */ 

/* NOTES FROM DATASHEET 
 * Data is sampled on the first rising edge of SCK after CS goes low
 * Before writing data, write enable latch must be set (CS low | WREN | CS high | CS low + write)
  
 
 NOTES END */
#include <avr/io.h>
#include <avr/portpins.h>

// Pins definitions
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_SS PK7
#define SPI_SCK PB1
#define SPI_MOSI PB2
#define SPI_MISO PB3


// EE refers to external EEPROM
#define EE_READ	0b00000011    // Read data from memory
#define EE_WRITE 0b00000010   // Write data to memory
#define EE_WRDI 0b00000100    // Disable write operations
#define EE_WREN 0b00000110    // Enable write operations
#define EE_RDSR 0b00000101    // Read STATUS register
#define EE_WRSR 0b00000001    // Write STATUS register

// EEPROM Status Register Bits -- from data sheet
// Use these to parse status register
#define EEPROM_WRITE_IN_PROGRESS    0
#define EEPROM_WRITE_ENABLE_LATCH   1
#define EEPROM_BLOCK_PROTECT_0      2
#define EEPROM_BLOCK_PROTECT_1      3
#define EEPROM_BYTES_PER_PAGE       64
#define EEPROM_BYTES_MAX            0x7FFF

#define HIGH 1
#define LOW  0

// SET SS PIN
#define SET_SS_PIN(state) ((state) ? (PORTK |= (1<<SPI_SS)) : (PORTK &= ~(1<<SPI_SS)))

#define SNAKE_LENGTH_ADDRESS 0x00
#define SNAKE_SPEED_ADDRESS 0x0A

uint16_t SNAKE_ADDRESSES[5] = {
	0x00, 0x02, 0x04, 0x06, 0x08
};
uint8_t NUM_HIGHSCORE_ADDRESSES = 5;

#define SNAKE_THEME_ADDRESS 0x0F

void spi_init() {
	
	DDRK |= (1 << SPI_SS);                        /* set SS output */
	PORTK |= (1 << SPI_SS);       /* start off not selected (high) */

    SPI_DDR |= (1 << SPI_MOSI);                   /* output on MOSI */
    SPI_PORT |= (1 << SPI_MISO);                  /* pullup on MISO */
    SPI_DDR |= (1 << SPI_SCK);                      /* output on SCK */

  /* Don't have to set phase, polarity b/c
   * default works with 25LCxxx chips */
    SPCR |= (1 << SPR1);                /* div 16, safer for breadboards */
    SPCR |= (1 << MSTR);                                  /* clockmaster */
    SPCR |= (1 << SPE);                                        /* enable */

}

void spi_transmitByte(char byte) {
	SPDR = byte;
	while (!(SPSR & (1<<SPIF)));  // wait until IRQ flag is set
}

void eeprom_send16BitAddress(uint16_t address) {
	spi_transmitByte((uint8_t) (address >> 8));    /* most significant byte */
	spi_transmitByte((uint8_t) address);          /* least significant byte */
}

uint8_t eeprom_readStatus(void) {
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_RDSR);
	spi_transmitByte(0);                            /* clock out eight bits */
	SET_SS_PIN(HIGH);
	return (SPDR);                                  /* return the result */
}

void eeprom_writeEnable(void) {
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_WREN);
	SET_SS_PIN(HIGH);
}

uint8_t eeprom_readByte(uint16_t address) {
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_READ);
	eeprom_send16BitAddress(address);
	spi_transmitByte(0);
	SET_SS_PIN(HIGH);
	return (SPDR);
}

uint16_t eeprom_readWord(uint16_t address) {
	uint16_t eepromWord;
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_READ);
	eeprom_send16BitAddress(address);
	spi_transmitByte(0);
	eepromWord = SPDR;
	eepromWord = (eepromWord << 8);                      /* most-sig bit */
	spi_transmitByte(0);
	eepromWord += SPDR;                                 /* least-sig bit */
	SET_SS_PIN(HIGH);
	return (eepromWord);
}

void eeprom_writeByte(uint16_t address, uint8_t byte) {
	eeprom_writeEnable();
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_WRITE);
	eeprom_send16BitAddress(address);
	spi_transmitByte(byte);
	SET_SS_PIN(HIGH);
	while (eeprom_readStatus() & (1<<EEPROM_WRITE_IN_PROGRESS)) {;
	}
}

void eeprom_writeWord(uint16_t address, uint16_t word) {
	eeprom_writeEnable();
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_WRITE);
	eeprom_send16BitAddress(address);
	spi_transmitByte((uint8_t) (word >> 8));
	spi_transmitByte((uint8_t) word);
	SET_SS_PIN(HIGH);
	while (eeprom_readStatus() & (1<<EEPROM_WRITE_IN_PROGRESS)) {;
	}
}

void eeprom_clearAll(void) {
	uint8_t i;
	uint16_t pageAddress = 0;
	while (pageAddress < EEPROM_BYTES_MAX) {
		eeprom_writeEnable();
		SET_SS_PIN(LOW);
		spi_transmitByte(EE_WRITE);
		eeprom_send16BitAddress(pageAddress);
		for (i = 0; i < EEPROM_BYTES_PER_PAGE; i++) {
			spi_transmitByte(0);
		}
		SET_SS_PIN(HIGH);
		pageAddress += EEPROM_BYTES_PER_PAGE;
		while (eeprom_readStatus() & (1<<EEPROM_WRITE_IN_PROGRESS)) {;
		}
	}
}

void eeprom_clearSnakeAddresses(void) {
	eeprom_writeEnable();
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_WRITE);
	eeprom_send16BitAddress(SNAKE_ADDRESSES[0]);
	for (uint8_t i=0; i<NUM_HIGHSCORE_ADDRESSES*2;i++) {
		spi_transmitByte(0);
	}
	SET_SS_PIN(HIGH);
	while (eeprom_readStatus() & (1<<EEPROM_WRITE_IN_PROGRESS)) {;
	}
}

void eeprom_saveSnakeScores(uint16_t * array, uint8_t length) {
	eeprom_writeEnable();
	SET_SS_PIN(LOW);
	spi_transmitByte(EE_WRITE);
	eeprom_send16BitAddress(SNAKE_ADDRESSES[0]);
	for (uint8_t i = 0; i<length;i++) {
		spi_transmitByte((uint8_t) (array[i] >> 8));
		spi_transmitByte((uint8_t) array[i]);
	}
	SET_SS_PIN(HIGH);
	while (eeprom_readStatus() & (1<<EEPROM_WRITE_IN_PROGRESS)) {;
	}
}