#ifndef EEPROM_H
#define EEPROM_H

#include "stdint.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define I2C_PORT_SDA_PIN 14
#define I2C_PORT_SDL_PIN 15
#define I2C_BAUD_RATE 100000
#define DEVADDR 0x50

void eeprom_read_bytes(uint16_t address, uint8_t length, uint8_t *buffer);
void eeprom_write_bytes(uint16_t address, uint8_t length, uint8_t *data);

#endif