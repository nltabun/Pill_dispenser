#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "pico/stdlib.h"

#define STEPPER_PIN_A 2
#define STEPPER_PIN_B 3
#define STEPPER_PIN_C 6
#define STEPPER_PIN_D 13
#define OPTO_FORK_PIN 28
#define STEPS 8
#define COILS 4
#define DELAY_MS 2 
#define DEFAULT_STEPS_PER_REV 4096

typedef struct MotorSteps
{
    uint8_t steps[STEPS][COILS];
    uint8_t current_step;
    uint16_t steps_per_revolution;
} MotorSteps;

void motor_step(const uint8_t *step);
uint8_t adjust_current_step(uint8_t current_step, bool reverse);
void rotate_motor(MotorSteps *motor_steps, bool reverse);
void calibrate(MotorSteps *motor_steps, const int runs);
void recalibrate_after_poweroff(MotorSteps *motor_steps, uint8_t position);
void turn_dispenser(MotorSteps *motor_steps, int turns, bool *pill_dispensed);
void motor_setup(void);
void opto_fork_setup(void);

#endif
