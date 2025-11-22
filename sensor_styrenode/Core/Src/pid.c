/* pid.c - simple integer PID with conditional integral update
 * This implementation uses 64-bit intermediates to avoid overflow
 * and only accumulates integral when |error| > INTEGRAL_DEADZONE.
 */

#include "pid.h"
#include <stdint.h>

#define TIME_STEP 0.01
#define INVERSE_TIME_STEP 100
#define OUTPUT_LIMIT 250000
#define INTEGRAL_LIMIT 250000
#define INTEGRAL_DEADZONE 5 

void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t kaw, uint16_t setpoint)
{
  pid->Kp = (int32_t)Kp;
  pid->Ki = (int32_t)Ki;
  pid->Kd = (int32_t)Kd;
  pid->kaw = (int32_t)kaw;

  pid->setpoint = (int32_t)setpoint;
  
  pid->proportional = 0;
  pid->integral = 0;
  pid->integral_prev = 0;
  pid->derivative = 0;
  
  pid->err = 0;
  pid->err_prev = 0;

  pid->filt_measurement_prev = 0;

  pid->output = 0;
  pid->anti_windup = 0;
}

void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t kaw, uint16_t setpoint)
{
  pid->Kp = (int32_t)Kp;
  pid->Ki = (int32_t)Ki;
  pid->Kd = (int32_t)Kd;
  pid->kaw = (int32_t)kaw;

  pid->setpoint = (int32_t)setpoint;
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

  pid->err_prev = 0;

  pid->filt_measurement_prev = 0;

  pid->output = 0;
  pid->anti_windup = 0;
}

void compute_PID_Output(pid_t *pid, uint16_t measured_value)
{ 
  // Beregn error
  pid->err = pid->setpoint - (int32_t)measured_value;

  // Bergn P-delen
  pid->proportional = pid->Kp * pid->err;

  // Bergn I-delen kun visst ki er satt
  if (pid->Ki != 0) {
    // Stopper integrasjon ved litten feil, samme som pådragsfunksjon
    if (pid->err > INTEGRAL_DEADZONE || pid->err < -INTEGRAL_DEADZONE) {
      int32_t sum_err = pid->err + pid->err_prev;
      int32_t incr = 0;
      // Beregn inkrement med anti-windup kun visst kaw er satt
      if (pid->kaw != 0) {
        incr = ((pid->Ki + pid->anti_windup) * sum_err) / (2LL * (int32_t)INVERSE_TIME_STEP);
      }
      else {
        incr = (pid->Ki * sum_err) / (2LL * (int32_t)INVERSE_TIME_STEP);
      }
      pid->integral = pid->integral_prev + incr;  
    }
    else {
      pid->integral = pid->integral_prev;
    }
  }
  // Setter integral begrensning
  if (pid->integral > INTEGRAL_LIMIT) pid->integral = INTEGRAL_LIMIT;
  if (pid->integral < -INTEGRAL_LIMIT) pid->integral = -INTEGRAL_LIMIT;

  // Bergn D-delen kun visst kd er satt
  if(pid->Kd != 0) {
    int32_t filt_meas = (int32_t)((964LL * (int32_t)pid->filt_measurement_prev + 60LL * (int32_t)measured_value) / 1024LL);
    int32_t delta_filt = filt_meas - pid->filt_measurement_prev;
    pid->filt_measurement_prev = filt_meas;
    pid->derivative = - (pid->Kd * (int32_t)INVERSE_TIME_STEP * delta_filt );
  }

  // Total output
  pid->output = pid->proportional + pid->integral + pid->derivative;
  // Mellomlagring for anti-windup
  int32_t out_usat = pid->output;
  
  // Sett utgangsbegrensning
  if(pid->output > OUTPUT_LIMIT) pid->output = OUTPUT_LIMIT;
  if(pid->output < -OUTPUT_LIMIT) pid->output = -OUTPUT_LIMIT;
  
  pid->anti_windup = (out_usat - pid->output) * pid->kaw;
  pid->err_prev = pid->err;
  pid->integral_prev = pid->integral;
}



