#ifndef EEPROM_H
#define EEPROM_H

#include "stdint.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define I2C_PORT_SDA_PIN 14
#define I2C_PORT_SDL_PIN 15
#define I2C_BAUD_RATE 100000
#define DEV_ADDR 0x50
#define DISPENSER_STATE_LEN 12 // TBD
#define DISPENSER_STATE_LAST_ADDR 32767
#define DISPENSER_STATE_ADDR (DISPENSER_STATE_LAST_ADDR - DISPENSER_STATE_LEN)
#define POSITION_ADDR (DISPENSER_STATE_LAST_ADDR - 2)

void eeprom_read_bytes(uint16_t address, uint8_t length, uint8_t *buffer);
void eeprom_write_bytes(uint16_t address, uint8_t length, uint8_t *data);
bool load_state_from_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining, uint8_t *current_step, uint16_t *steps_per_revolution, uint8_t *position);
void save_state_to_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining, uint8_t *current_step, uint16_t *steps_per_revolution);
void update_position(uint8_t position);
bool _validate_stored_value(uint8_t value, uint8_t not_value);
void i2c_setup(void);

#endif