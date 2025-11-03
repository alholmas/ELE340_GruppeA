#include <stdio.h>
#include <stdlib.h>
#define TIME_STEP 0.01 

// 
typedef struct {

    float m; // mass of the car
    float v; // current velocity of the car

} system_t;

// PID struct for storing gains, integrals, and previous error
typedef struct {
    float Kp;       // Propsjonjonal forsterkning
    float Ti;       // Integral tidskonstant
    float Td;       // Derivert tidskonstant
    float output;   // Kontroller utgang
} pid_t;

/* Extended PID state with integrator and previous error. Kept in this C
 * file for simplicity; other modules can call the functions below to use
 * the PID regulator.
 */
typedef struct {
    /* tuning */
    float Kp;
    float Ti; /* integral time (s) */
    float Td; /* derivative time (s) */
    float integrator_limit;
    float setpoint;

    /* internal computed gains */
    float Ki; /* = Kp / Ti */
    float Kd; /* = Kp * Td */

    /* state */
    float integrator;
    float prev_error;

    /* output limits */
    float out_min;
    float out_max;
} pid_state_t;

/* API: initialize, reset and compute */
void PID_Init(pid_state_t *s, float Kp, float Ti, float Td, float out_min, float out_max);
void PID_Reset(pid_state_t *s);
float PID_Compute(pid_state_t *s, float setpoint_mm, float measurement_mm);

/* Implementation */
void PID_Init(pid_state_t *s, float Kp, float Ti, float Td, float out_min, float out_max)
{
    if (s == NULL) return;
    s->Kp = Kp;
    s->Ti = Ti;
    s->Td = Td;
    s->Ki = (Ti != 0.0f) ? (Kp / Ti) : 0.0f;
    s->Kd = Kp * Td;
    s->integrator = 0.0f;
    s->prev_error = 0.0f;
    s->out_min = out_min;
    s->out_max = out_max;
}

void PID_Reset(pid_state_t *s)
{
    if (s == NULL) return;
    s->integrator = 0.0f;
    s->prev_error = 0.0f;
}

float PID_Compute(pid_state_t *s, float setpoint_mm, float measurement_mm)
{
    if (s == NULL) return 0.0f;

    float error = setpoint_mm - measurement_mm;

    /* Proportional term */
    float P = s->Kp * error;

    /* Derivative (backward difference) */
    float derivative = 0.0f;
    if (TIME_STEP > 0.0f) {
        derivative = (error - s->prev_error) / TIME_STEP;
    }
    float D = s->Kd * derivative;

    /* Integral (conditional anti-windup): update integrator only when
     * the controller is not saturated in the direction of error.
     */
    float I = s->Ki * s->integrator;
    float tentative = P + I + D;

    if (!((tentative > s->out_max && error > 0.0f) || (tentative < s->out_min && error < 0.0f))) {
        s->integrator += error * TIME_STEP;
        I = s->Ki * s->integrator;
    }

    float u = P + I + D;

    /* Clamp output */
    if (u > s->out_max) u = s->out_max;
    if (u < s->out_min) u = s->out_min;

    s->prev_error = error;
    return u;
}




