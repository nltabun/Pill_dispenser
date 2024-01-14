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
#define CYCLE_DURATION 30000000 // us = (30s) -- 86400000000 for 24h cycle
#define PIEZO_SENSOR_PIN 27


enum DispenserState
{
    WAIT_FOR_CALIBRATION = 1,
    CALIBRATING,
    READY_TO_START,
    DISPENSING
};

void init_all(void);
void led_setup(void);
void button_setup(void);
void piezo_sensor_setup(void);
static void gpio_handler(uint gpio, uint32_t event_mask);

//Global variables
static bool pillDispensed = false;

int main(void)
{
    enum DispenserState state;
    bool led = false;
    uint8_t led_timer = 0;
    uint64_t start_time;
    uint64_t time;
    uint8_t cycles_remaining;
    uint8_t position = 0;
    uint8_t msg[MSG_MAX_LEN] = {0};
    bool skip_wait = false;

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

    lora_connect();
    lora_msg("AT+MSG=\"Booting up...\"\r\n");

    state = load_state_from_eeprom(&cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution, &position);
    if (state)
    {
        printf("Loaded state: %d\n", state);
        lora_msg("AT+MSG=\"Dispenser state successfully loaded\"\r\n");

        if (state == DISPENSING)
        {
            recalibrate_after_poweroff(&MOTOR_STEPS, position);
            lora_msg("AT+MSG=\"Recalibrated\"\r\n");
            printf("Recalibrated. Cycles remaining: %d\n", cycles_remaining);

            if (position > 0)
                skip_wait = true;
        }
    }
    else
    {
        printf("No valid dispenser state found in memory\n");
        lora_msg("AT+MSG=\"No valid dispenser state found\"\r\n");
        state = WAIT_FOR_CALIBRATION; // Set state to '1'
    }

    start_time = time_us_64();
    time = start_time;
    

    while (true)
    {
        switch (state)
        {
        case WAIT_FOR_CALIBRATION:
            // Wait for button press and blink led 0
            while (gpio_get(BUTTON_SW1) && gpio_get(BUTTON_SW2))
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
            
            if (!gpio_get(BUTTON_SW2))
            {
                while (!gpio_get(BUTTON_SW2)) // Wait for button release
                    sleep_ms(10);

                read_log();
                break;
            }
            else
            {
                while (!gpio_get(BUTTON_SW1)) // Wait for button release
                    sleep_ms(10);
                
                erase_log(); // Erase previous runs log
                // sleep_ms(100); // Do we need this??

                state = CALIBRATING;
                save_state_to_eeprom((uint8_t)state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
            }
        case CALIBRATING:
            calibrate(&MOTOR_STEPS, 1);
            lora_msg("AT+MSG=\"Calibrated\"\r\n");

            state = READY_TO_START;
            save_state_to_eeprom((uint8_t)state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
        case READY_TO_START:
            gpio_put(LED_1, (led = true));
            while (gpio_get(BUTTON_SW1)) // Wait for button press
                sleep_ms(10);

            while (!gpio_get(BUTTON_SW1)) // Wait for button release
                sleep_ms(10);

            gpio_put(LED_1, (led = false));

            state = DISPENSING;
            cycles_remaining = 7;
            save_state_to_eeprom((uint8_t)state, &cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
        case DISPENSING:
            time = time_us_64();
            while (cycles_remaining > 0)
            {
                if (!skip_wait) // If the dispenser was in the middle of a turn on poweroff then we don't want to wait
                {
                    while ((time_us_64() - time) < CYCLE_DURATION)
                    {
                        sleep_ms(10);
                    }
                }

                time = time_us_64();
                turn_dispenser(&MOTOR_STEPS, 1, &pillDispensed);
                cycles_remaining--;
                save_state_to_eeprom((uint8_t)state, &cycles_remaining, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
                update_position(0);

                if (pillDispensed)
                {
                    snprintf((char *)msg, MSG_MAX_LEN, "Pill dispensed. %hu cycles remaining", cycles_remaining);
                    printf("%s\n", msg);
                    add_message_to_log(msg);
                    lora_msg("AT+MSG=\"Pill dispensed\"\r\n");
                }
                else
                {
                    snprintf((char *)msg, MSG_MAX_LEN, "Pill not dispensed. %hu cycles remaining", cycles_remaining);
                    printf("%s\n", msg);
                    add_message_to_log(msg);
                    for (int i = 0; i < 5; ++i) {
                        gpio_put(LED_2, (led = true));
                        sleep_ms(750);
                        gpio_put(LED_2, (led = false));
                        sleep_ms(750);
                    }
                    lora_msg("AT+MSG=\"Pill not dispensed\"\r\n");
                }
                pillDispensed = false;
                skip_wait = false;
            }
            state = WAIT_FOR_CALIBRATION;
            save_state_to_eeprom((uint8_t)state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
            break;
        default:
            state = WAIT_FOR_CALIBRATION;
            save_state_to_eeprom((uint8_t)state, 0, &MOTOR_STEPS.current_step, &MOTOR_STEPS.steps_per_revolution);
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

static void gpio_handler(uint gpio, uint32_t event_mask)
{
    pillDispensed = true;
}

void piezo_sensor_setup(void)
{
    gpio_init(PIEZO_SENSOR_PIN);
    gpio_set_dir(PIEZO_SENSOR_PIN, GPIO_IN);
    gpio_pull_up(PIEZO_SENSOR_PIN);
    gpio_set_irq_enabled_with_callback(PIEZO_SENSOR_PIN, GPIO_IRQ_EDGE_FALL, true, gpio_handler);
}
