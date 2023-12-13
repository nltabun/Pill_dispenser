#include <stdio.h>
#include "pico/stdlib.h"
#include "eeprom.h"

void eeprom_read_bytes(uint16_t address, uint8_t length, uint8_t *data)
{
    uint8_t buffer[2] = {address >> 8, address & 0xFF}; // High byte-, low byte of memory address

    i2c_write_blocking(I2C_PORT, DEV_ADDR, buffer, 2, true);
    i2c_read_blocking(I2C_PORT, DEV_ADDR, data, length, false);
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

    i2c_write_blocking(I2C_PORT, DEV_ADDR, buffer, length + 2, false);

    sleep_ms(5);
}

bool load_state_from_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining, uint8_t *current_step, uint16_t *steps_per_revolution, uint8_t *position)
{
    uint8_t read_buffer[DISPENSER_STATE_LEN] = {0};

    eeprom_read_bytes(DISPENSER_STATE_ADDR, DISPENSER_STATE_LEN, read_buffer);

    // Check if valid dispenser state is stored in the EEPROM
    if (_validate_stored_value(read_buffer[0], read_buffer[1]))
    {
        *dispenser_state = read_buffer[0];
    }
    else
    {
        *dispenser_state = 0;
        return false;
    }

    // Check if valid number of cycles remaining is stored in the EEPROM
    // This is present only if dispenser was shutdown in Dispensing state
    if (*dispenser_state == 3)
    {
        if (_validate_stored_value(read_buffer[2], read_buffer[3]))
            *cycles_remaining = read_buffer[2];
        else
            *cycles_remaining = 0;
    }

    if (_validate_stored_value(read_buffer[4], read_buffer[5]))
    {
        *current_step = read_buffer[4];
    }
    else
    {
        *current_step = 0;
    }

    if (_validate_stored_value(read_buffer[6], read_buffer[8]) && _validate_stored_value(read_buffer[7], read_buffer[9]))
    {
        *steps_per_revolution = (read_buffer[6] << 8) | read_buffer[7];
    }
    else
    {
        *steps_per_revolution = 0;
    }

    if (_validate_stored_value(read_buffer[10], read_buffer[11]))
    {
        *position = read_buffer[10];
    }
    else
    {
        *position = 0;
    }

    // printf("Loaded state: %hhu\nCycles remaining: %hhu\nCurrent step: %hhu\nSteps per revolution: %hu\nPosition: %hhu\n", *dispenser_state, *cycles_remaining, *current_step, *steps_per_revolution, *position);

    return true;
}

void save_state_to_eeprom(uint8_t *dispenser_state, uint8_t *cycles_remaining, uint8_t *current_step, uint16_t *steps_per_revolution)
{
    uint8_t write_buffer[DISPENSER_STATE_LEN];

    write_buffer[0] = *dispenser_state;
    write_buffer[1] = ~*dispenser_state;

    write_buffer[2] = *cycles_remaining;
    write_buffer[3] = ~*cycles_remaining;

    write_buffer[4] = *current_step;
    write_buffer[5] = ~*current_step;

    write_buffer[6] = *steps_per_revolution >> 8;
    write_buffer[7] = *steps_per_revolution & 0xFF;
    write_buffer[8] = ~(*steps_per_revolution >> 8);
    write_buffer[9] = ~(*steps_per_revolution & 0xFF);

    // printf("Writing state to EEPROM: %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu\n", write_buffer[0], write_buffer[1], write_buffer[2], write_buffer[3], write_buffer[4], write_buffer[5], write_buffer[6], write_buffer[7], write_buffer[8], write_buffer[9]);

    eeprom_write_bytes(DISPENSER_STATE_ADDR, DISPENSER_STATE_LEN-2, write_buffer);
}

void update_position(uint8_t position)
{
    uint8_t write_buffer[2] = {position, ~position};
    eeprom_write_bytes(POSITION_ADDR, 2, write_buffer);
}

bool _validate_stored_value(uint8_t value, uint8_t not_value)
{
    return value == (uint8_t)~not_value;
}

void i2c_setup(void)
{
    i2c_init(I2C_PORT, I2C_BAUD_RATE);
    gpio_set_function(I2C_PORT_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_PORT_SDL_PIN, GPIO_FUNC_I2C);
}