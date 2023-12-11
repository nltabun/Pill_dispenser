#ifndef EEPROM_H
#define EEPROM_H

#include "stdint.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define I2C_PORT_SDA_PIN 14
#define I2C_PORT_SDL_PIN 15
#define I2C_BAUD_RATE 100000
#define DEV_ADDR 0x50
#define DISPENSER_STATE_LEN 4 // TBD
#define DISPENSER_STATE_LAST_ADDR 32767
#define DISPENSER_STATE_ADDR (DISPENSER_STATE_LAST_ADDR - DISPENSER_STATE_LEN)

void eeprom_read_bytes(uint16_t address, uint8_t length, uint8_t *buffer);
void eeprom_write_bytes(uint16_t address, uint8_t length, uint8_t *data);
bool load_state_from_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining);
void save_state_to_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining);
bool _validate_stored_value(uint8_t value, uint8_t not_value);
void i2c_setup(void);

#endif