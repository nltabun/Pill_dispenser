#include <stdio.h>
#include "pico/stdlib.h"

#define LED_0 20
#define LED_1 21
#define LED_2 22
#define BUTTON_SW0 9
#define BUTTON_SW1 8
#define BUTTON_SW2 7
#define UART_NR 1 
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define UART_BAUD_RATE 9600
#define I2C_PORT i2c1
#define I2C_PORT_SDA_PIN 14
#define I2C_PORT_SDL_PIN 15
#define I2C_BAUD_RATE 100000
#define STEPPER_PIN_A 2
#define STEPPER_PIN_B 3
#define STEPPER_PIN_C 6
#define STEPPER_PIN_D 13
#define OPTO_FORK_PIN 28

enum State {
    WAIT = 0,
    CALIBRATE = 1,
    READY = 2,
    DISPENSING = 3
} DispenserState;

int main() {
    init_all();

    while (true) {

    }
}

void init_all()
{
    stdio_init_all();

    gpio_init(STEPPER_PIN_A);
    gpio_init(STEPPER_PIN_B);
    gpio_init(STEPPER_PIN_C);
    gpio_init(STEPPER_PIN_D);
    gpio_init(OPTO_FORK_PIN);
    gpio_set_dir(STEPPER_PIN_A, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_B, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_C, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_D, GPIO_OUT);
    gpio_set_dir(OPTO_FORK_PIN, GPIO_IN);
    gpio_pull_up(OPTO_FORK_PIN);
}