#include "pico/stdlib.h"
#include "eeprom.h"

void eeprom_read_bytes(uint16_t address, uint8_t length, uint8_t *data)
{
    uint8_t buffer[2] = {address >> 8, address & 0xFF}; // High byte-, low byte of memory address

    i2c_write_blocking(I2C_PORT, DEVADDR, buffer, 2, true);
    i2c_read_blocking(I2C_PORT, DEVADDR, data, length, false);
}

void eeprom_write_bytes(uint16_t address, uint8_t length, uint8_t *data)
{
    uint8_t buffer[length + 2]; // High byte-, low byte of memory address, data
    buffer[0] = address >> 8;
    buffer[1] = address & 0xFF;

    for (size_t i = 0; i < length; i++)
    {
        buffer[i + 2] = data[i];
    }

    i2c_write_blocking(I2C_PORT, DEVADDR, buffer, length + 2, false);

    sleep_ms(5);
}