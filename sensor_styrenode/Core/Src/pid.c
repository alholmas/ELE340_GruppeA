/* pid.c - simple integer PID with conditional integral update
 * This implementation uses 64-bit intermediates to avoid overflow
 * and only accumulates integral when |error| > INTEGRAL_DEADZONE.
 */

#include "pid.h"
#include <stdint.h>

#define TIME_STEP 0.01
#define INVERSE_TIME_STEP 100
#define OUTPUT_LIMIT 250000
#define INTEGRAL_DEADZONE 5 

void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t setpoint, uint32_t integral_limit)
{
  pid->Kp = Kp;
  pid->Ki = Ki;
  pid->Kd = Kd;

  pid->setpoint = setpoint;
  pid->integral_limit = integral_limit;

  pid->proportional = 0;
  pid->integral = 0;
  pid->integral_prev = 0;
  pid->derivative = 0;

  pid->error = 0;
  pid->error_prev = 0;

  pid->filtered_measurement = 0;
  pid->filtered_measurement_prev = 0;

  pid->output = 0;
  pid->output_limit = OUTPUT_LIMIT;
}

void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t setpoint, uint32_t integral_limit)
{
  pid->Kp = Kp;
  pid->Ki = Ki;
  pid->Kd = Kd;

  pid->setpoint = setpoint;
  pid->integral_limit = integral_limit;
  if (pid->Ki == 0) {
    pid->integral = 0;
    pid->integral_prev = 0;
  }
}

void reset_pid(pid_t *pid)
{
  pid->proportional = 0;
  pid->integral = 0;
  pid->integral_prev = 0;
  pid->derivative = 0;

  pid->error = 0;
  pid->error_prev = 0;

  pid->filtered_measurement = 0;
  pid->filtered_measurement_prev = 0;

  pid->output = 0;
}

void compute_PID_Output(pid_t *pid, uint16_t measured_value)
{
  // Beregn error (signed)
  int32_t error = (int32_t)measured_value - (int32_t)pid->setpoint;
  pid->error = error;

  // P-delen (64-bit temp)
  int64_t P = (int64_t)pid->Kp * (int64_t)error;
  pid->proportional = (int32_t)P;

  // I-delen: trapezoidal, kun når |error| > INTEGRAL_DEADZONE
  int64_t I = (int64_t)pid->integral_prev;
  if (error > INTEGRAL_DEADZONE || error < -INTEGRAL_DEADZONE) {
    int64_t sum_err = (int64_t)error + (int64_t)pid->error_prev;
    int64_t incr = 0;
    if (pid->Ki != 0) {
      incr = ((int64_t)pid->Ki * sum_err) / (2LL * (int64_t)INVERSE_TIME_STEP);
    }
    I = (int64_t)pid->integral_prev + incr;
  }

  // Clamp integral
  if (I > (int64_t)pid->integral_limit) I = pid->integral_limit;
  if (I < -(int64_t)pid->integral_limit) I = -(int64_t)pid->integral_limit;

  // Lowpass filter for derivative measurement
  int32_t filt = (int32_t)((964LL * (int64_t)pid->filtered_measurement_prev + 60LL * (int64_t)measured_value) / 1024LL);

  // D-delen
  int64_t delta_filt = (int64_t)filt - (int64_t)pid->filtered_measurement_prev;
  int64_t D = 0;
  if (pid->Kd != 0) {
    D = - ( (int64_t)pid->Kd * (int64_t)INVERSE_TIME_STEP * delta_filt );
  }

  // Total output
  int64_t out64 = P + I + D;
  if (out64 > (int64_t)pid->output_limit) out64 = pid->output_limit;
  if (out64 < -(int64_t)pid->output_limit) out64 = - (int64_t)pid->output_limit;

  pid->output = (int32_t)out64;

  // Update history
  pid->integral = (int32_t)I;
  pid->derivative = (int32_t)D;

  pid->error_prev = error;
  pid->filtered_measurement_prev = filt;
  pid->integral_prev = (int32_t)I;
}


