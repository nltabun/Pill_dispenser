#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "stepper_motor.h"
#include "lora.h"

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

enum DispenserState
{
    WAIT_FOR_BUTTON = 0,
    CALIBRATE = 1,
    READY = 2,
    DISPENSING = 3
};

void init_all();

int main()
{
    enum DispenserState state;
    bool led = false;
    uint8_t led_timer = 0;
    uint64_t time;
    uint8_t cycles_remaining;

    MotorSteps MOTOR_STEPS = {
        {{1, 0, 0, 0}, // 0
         {1, 1, 0, 0}, // 1
         {0, 1, 0, 0}, // 2
         {0, 1, 1, 0}, // 3
         {0, 0, 1, 0}, // 4
         {0, 0, 1, 1}, // 5
         {0, 0, 0, 1}, // 6
         {1, 0, 0, 1}},  // 7
        0,
        0
    };

    init_all();

    time = time_us_64();
    state = WAIT_FOR_BUTTON;

    while (true)
    {
        switch (state)
        {
        case WAIT_FOR_BUTTON:
            while (gpio_get(BUTTON_SW1))
            {
                if (led_timer > 100)
                {
                    led = !led;
                    gpio_put(LED_0, led);
                    led_timer = 0;
                }
                led_timer++;
                sleep_ms(10);
            }
            gpio_put(LED_0, (led = false));

            while (!gpio_get(BUTTON_SW1))
                sleep_ms(10);
            sleep_ms(100);
            state = CALIBRATE;
            break;
        case CALIBRATE:
            calibrate(&MOTOR_STEPS, 1);
            state = READY;
            break;
        case READY:
            gpio_put(LED_1, (led = true));
            while (gpio_get(BUTTON_SW1))
                sleep_ms(10);   

            while (!gpio_get(BUTTON_SW1))
                sleep_ms(10);

            gpio_put(LED_1, (led = false));

            cycles_remaining = 7;

            state = DISPENSING;
            break;
        case DISPENSING:
            for (; cycles_remaining > 0; cycles_remaining--)
            {
                time = time_us_64();
                while ((time_us_64() - time) < 30000000)
                {
                    sleep_ms(10);
                }

                run_motor(&MOTOR_STEPS, 1);
                
            }
            
            state = WAIT_FOR_BUTTON;
            break;
        default:
            state = WAIT_FOR_BUTTON;
            break;
        }
    }
}

void init_all()
{
    stdio_init_all();

    gpio_init(LED_0);
    gpio_init(LED_1);
    gpio_init(LED_2);

    gpio_set_dir(LED_0, GPIO_OUT);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_set_dir(LED_2, GPIO_OUT);

    gpio_init(BUTTON_SW0);
    gpio_init(BUTTON_SW1);
    gpio_init(BUTTON_SW2);

    gpio_set_dir(BUTTON_SW0, GPIO_IN);
    gpio_set_dir(BUTTON_SW1, GPIO_IN);
    gpio_set_dir(BUTTON_SW2, GPIO_IN);

    gpio_pull_up(BUTTON_SW0);
    gpio_pull_up(BUTTON_SW1);
    gpio_pull_up(BUTTON_SW2);

    gpio_init(STEPPER_PIN_A);
    gpio_init(STEPPER_PIN_B);
    gpio_init(STEPPER_PIN_C);
    gpio_init(STEPPER_PIN_D);

    gpio_set_dir(STEPPER_PIN_A, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_B, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_C, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_D, GPIO_OUT);

    gpio_init(OPTO_FORK_PIN);
    gpio_set_dir(OPTO_FORK_PIN, GPIO_IN);
    gpio_pull_up(OPTO_FORK_PIN);
}