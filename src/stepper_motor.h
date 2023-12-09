#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "pico/stdlib.h"

#define STEPS 8
#define COILS 4

typedef struct MotorSteps
{
    uint8_t steps[STEPS][COILS];
    uint8_t current_step;
    int steps_per_revolution;
} MotorSteps;

void motor_step(const uint8_t *step);
uint8_t adjust_current_step(uint8_t current_step, bool reverse);
void calibrate(const MotorSteps *motor_steps, uint8_t *current_step, const int runs, int *steps_per_revolution);
void run_motor(const MotorSteps *motor_steps, uint8_t *current_step, int runs, int *steps_per_revolution);

#endif