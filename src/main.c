#include <stdio.h>
#include "pico/stdlib.h"
#include "../src/stepper_motor.h"
#include "../src/lora.h"

#define LED_0 20
#define LED_1 21
#define LED_2 22
#define BUTTON_SW0 9
#define BUTTON_SW1 8
#define BUTTON_SW2 7
#define I2C_PORT i2c1
#define I2C_PORT_SDA_PIN 14
#define I2C_PORT_SDL_PIN 15
#define I2C_BAUD_RATE 100000

void init_all();

MotorSteps MOTOR_STEPS = {
        {{1, 0, 0, 0}, // 0
         {1, 1, 0, 0}, // 1
         {0, 1, 0, 0}, // 2
         {0, 1, 1, 0}, // 3
         {0, 0, 1, 0}, // 4
         {0, 0, 1, 1}, // 5
         {0, 0, 0, 1}, // 6
         {1, 0, 0, 1}},  // 7
        {0},
        {0}
};

int main() {
    init_all();
    while (true)
    {

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