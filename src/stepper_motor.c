// Stepper motor stuff
#include <stdio.h>
#include "pico/stdlib.h"
#include "stepper_motor.h"

void motor_step(const uint8_t *step)
{
    gpio_put(STEPPER_PIN_A, step[0]);
    gpio_put(STEPPER_PIN_B, step[1]);
    gpio_put(STEPPER_PIN_C, step[2]);
    gpio_put(STEPPER_PIN_D, step[3]);
}

uint8_t adjust_current_step(uint8_t current_step, bool reverse)
{
    if (reverse == false)
    {
        if (current_step + 1 < STEPS)
            return current_step + 1;
        else
            return 0;
    }

    if (reverse == true)
    {
        if (current_step - 1 >= 0)
            return current_step - 1;
        else
            return STEPS - 1;
    }

    return 0;
}

void rotate_motor(MotorSteps *motor_steps, bool reverse)
{
    motor_steps->current_step = adjust_current_step(motor_steps->current_step, reverse);
    motor_step(motor_steps->steps[motor_steps->current_step]);
    sleep_ms(DELAY_MS);
}

void calibrate(MotorSteps *motor_steps, const int runs)
{
    printf("Calibrating...\n");

    // Perform calibration by running the motor and counting steps to the next falling edge
    int total_steps = 0;
    int total_low_steps = 0;

    for (int i = 0; i < runs; i++)
    {
        int steps = 0;
        int low_steps = 0;

        // Get to the starting position
        if (gpio_get(OPTO_FORK_PIN))
        {
            while (gpio_get(OPTO_FORK_PIN))
            {
                rotate_motor(motor_steps, false);
            }
        }

        while (!gpio_get(OPTO_FORK_PIN))
        {
            rotate_motor(motor_steps, false);
        }

        // Start counting steps
        while (gpio_get(OPTO_FORK_PIN))
        {
            rotate_motor(motor_steps, false);
            steps++;
        }

        while (!gpio_get(OPTO_FORK_PIN))
        {
            rotate_motor(motor_steps, false);
            steps++;
            low_steps++;
        }

        total_steps += steps;
        total_low_steps += low_steps;

        // Reverse back to center after the last run
        if (i + 1 == runs)
        {
            for (int j = 0; j < (total_low_steps / runs) / 2; j++)
            {
                rotate_motor(motor_steps, true);
            }
        }

        printf("Calibration run %d: %d steps\n", i + 1, steps);
    }

    // Take the average of the values obtained during calibration runs
    motor_steps->steps_per_revolution = total_steps / runs;

    printf("Calibration complete. Steps per revolution: %d\n", motor_steps->steps_per_revolution);
}

void turn_dispenser(MotorSteps *motor_steps, int turns, bool *pill_dispensed)
{
    int run_steps;

    printf("Running...\n");

    if (!motor_steps->steps_per_revolution)
    {
        printf("Warning! Device not calibrated. Defaulting to steps per revolution: %d\n", DEFAULT_STEPS_PER_REV);
        run_steps = turns * (DEFAULT_STEPS_PER_REV / 8);
    }
    else
        run_steps = turns * (motor_steps->steps_per_revolution / 8);

    *pill_dispensed = false;    
    for (int i = 0; i < run_steps; i++)
    {
        rotate_motor(motor_steps, false);
        if (!gpio_get(PIEZO_SENSOR_PIN))
        {
            *pill_dispensed = true;
        }
        
    }

    printf("Motor turned dispenser %d time(s)\n", turns);
}

void motor_setup(void)
{
    gpio_init(STEPPER_PIN_A);
    gpio_init(STEPPER_PIN_B);
    gpio_init(STEPPER_PIN_C);
    gpio_init(STEPPER_PIN_D);

    gpio_set_dir(STEPPER_PIN_A, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_B, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_C, GPIO_OUT);
    gpio_set_dir(STEPPER_PIN_D, GPIO_OUT);
}

void opto_fork_setup(void)
{
    gpio_init(OPTO_FORK_PIN);
    gpio_set_dir(OPTO_FORK_PIN, GPIO_IN);
    gpio_pull_up(OPTO_FORK_PIN);
}

void piezo_sensor_setup(void)
{
    gpio_init(PIEZO_SENSOR_PIN);
    gpio_set_dir(PIEZO_SENSOR_PIN, GPIO_IN);
    gpio_pull_up(PIEZO_SENSOR_PIN);
}