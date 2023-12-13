#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "stepper_motor.h"
#include "lora.h"
#include "eeprom.h"

#define LED_0 20
#define LED_1 21
#define LED_2 22
#define BUTTON_SW0 9
#define BUTTON_SW1 8
#define BUTTON_SW2 7

enum DispenserState
{
    WAIT_FOR_CALIBRATION = 0,
    CALIBRATING,
    READY_TO_START,
    DISPENSING
};

void init_all(void);
void led_setup(void);
void button_setup(void);

int main(void)
{
    enum DispenserState state;
    bool led = false;
    uint8_t led_timer = 0;
    uint64_t start_time;
    uint64_t time;
    uint8_t cycles_remaining;
    bool pill_dispensed;
    uint8_t position = 0;

    MotorSteps MOTOR_STEPS = {
        {{1, 0, 0, 0},  // 0
         {1, 1, 0, 0},  // 1
         {0, 1, 0, 0},  // 2
         {0, 1, 1, 0},  // 3
         {0, 0, 1, 0},  // 4
         {0, 0, 1, 1},  // 5
         {0, 0, 0, 1},  // 6
         {1, 0, 0, 1}}, // 7
        0,
        0};

    init_all();

    // lora_msg("Booting up...");
    lora_connect();
    lora_msg("AT+MSG=\"Booting up...\"\r\n");
    char response[RESP_LEN];

    if (load_state_from_eeprom(&state, &cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution, &position))
    {
        printf("Loaded state: %d\n", state);
        // lora_msg("Dispenser state successfully loaded");
        lora_msg("AT+MSG=\"Dispenser state successfully loaded\"\r\n");

        if (state == DISPENSING)
        {
            recalibrate_after_poweroff(&MOTOR_STEPS, cycles_remaining, position);
            lora_msg("AT+MSG=\"Recalibrating\"\r\n");
            printf("Recalibrated. Cycles remaining: %d\n", cycles_remaining);
        }
    }
    else
    {
        printf("No valid dispenser state found\n");
        // lora_msg("No valid dispenser state found");
        lora_msg("AT+MSG=\"No valid dispenser state found\"\r\n");
    }

    start_time = time_us_64();
    time = start_time;

    while (true)
    {
        switch (state)
        {
        case WAIT_FOR_CALIBRATION:
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
            state = CALIBRATING;
            save_state_to_eeprom(&state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
        case CALIBRATING:
            calibrate(&MOTOR_STEPS, 1);
            // lora_msg("Calibrated");
            lora_msg("AT+MSG=\"Calibrated\"\r\n");
            state = READY_TO_START;
            save_state_to_eeprom(&state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
        case READY_TO_START:
            gpio_put(LED_1, (led = true));
            while (gpio_get(BUTTON_SW1))
                sleep_ms(10);

            while (!gpio_get(BUTTON_SW1))
                sleep_ms(10);

            gpio_put(LED_1, (led = false));

            state = DISPENSING;
            cycles_remaining = 7;
            save_state_to_eeprom(&state, &cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
        case DISPENSING:
            time = time_us_64();
            while (cycles_remaining > 0)
            {
                while ((time_us_64() - time) < 30000000) // TODO: Need a better way to do this
                {
                    sleep_ms(10);
                }

                turn_dispenser(&MOTOR_STEPS, 1, &pill_dispensed);
                cycles_remaining--;
                save_state_to_eeprom(&state, &cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
                update_position(0);
                time = time_us_64();

                if (pill_dispensed)
                {
                    printf("Pill dispensed!\n");
                    // lora_msg("Pill dispensed!");
                    lora_msg("AT+MSG=\"Pill dispensed\"\r\n");
                }
                else
                {
                    printf("Pill not dispensed!\n");
                    // lora_msg("Pill not dispensed!");
                    lora_msg("AT+MSG=\"Pill not dispensed\"\r\n");
                }
            }

            state = WAIT_FOR_CALIBRATION;
            save_state_to_eeprom(&state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
            break;
        default:
            state = WAIT_FOR_CALIBRATION;
            save_state_to_eeprom(&state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
            break;
        }
    }
}

void init_all(void)
{
    stdio_init_all();
    led_setup();
    button_setup();
    motor_setup();
    opto_fork_setup();
    piezo_sensor_setup();
    i2c_setup();
}

void led_setup(void)
{
    gpio_init(LED_0);
    gpio_init(LED_1);
    gpio_init(LED_2);

    gpio_set_dir(LED_0, GPIO_OUT);
    gpio_set_dir(LED_1, GPIO_OUT);
    gpio_set_dir(LED_2, GPIO_OUT);
}

void button_setup(void)
{
    gpio_init(BUTTON_SW0);
    gpio_init(BUTTON_SW1);
    gpio_init(BUTTON_SW2);

    gpio_set_dir(BUTTON_SW0, GPIO_IN);
    gpio_set_dir(BUTTON_SW1, GPIO_IN);
    gpio_set_dir(BUTTON_SW2, GPIO_IN);

    gpio_pull_up(BUTTON_SW0);
    gpio_pull_up(BUTTON_SW1);
    gpio_pull_up(BUTTON_SW2);
}
