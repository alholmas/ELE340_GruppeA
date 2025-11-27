#include "pid.h"
#include <stdint.h>
#include <sys/_intsup.h>

#define INVERSE_TIME_STEP 100
#define OUTPUT_LIMIT 250000
#define INTEGRAL_LIMIT 250000



void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t kb, uint16_t setpoint)
{
  pid->Kp = (int32_t)Kp;
  pid->Ki = (int32_t)Ki;
  pid->Kd = (int32_t)Kd;
  pid->kb = (int32_t)kb;

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

  pid->err_deadzone = (uint16_t)pid->setpoint / 100;


}

void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ki, uint16_t Kd, uint16_t kb, uint16_t setpoint)
{
  pid->Kp = (int32_t)Kp;
  pid->Ki = (int32_t)Ki;
  pid->Kd = (int32_t)Kd;
  pid->kb = (int32_t)kb;

  uint16_t deadzone = (uint16_t)pid->setpoint / 100;
  
  pid->err_deadzone = deadzone;

  pid->setpoint = (int32_t)setpoint;
  if (pid->Ki == 0) {
    pid->integral = 0;
    pid->integral_prev = 0;
  }
}

void pid_default_init(pid_t *pid)
{
  pid_init(pid, 5210, 6512, 0, 5, 300);
}


void pid_deactivate_deadzone(pid_t *pid)
{
  pid->err_deadzone = 0;
}

void pid_activate_deadzone(pid_t *pid)
{
  pid->err_deadzone = (uint16_t)pid->setpoint / 100;
}

// Resetter pid
void reset_pid(pid_t *pid)
{
  pid->proportional = 0;
  pid->integral = 0;
  pid->integral_prev = 0;
  pid->derivative = 0;

  pid->err = 0;
  pid->err_prev = 0;
  pid->err_deadzone = 0;
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
  if (pid->Ki != 0 && (pid->err > (int32_t)pid->err_deadzone || pid->err < -(int32_t)pid->err_deadzone)) {
      int32_t sum_err = pid->err + pid->err_prev;
      int32_t incr = (pid->Ki * sum_err) / (2LL * (int32_t)INVERSE_TIME_STEP);
      pid->integral = pid->integral_prev + incr;  
    }
  else {
    pid->integral = pid->integral_prev;
  }
  // Setter integral begrensning
  
  if (pid->integral > INTEGRAL_LIMIT) pid->integral = INTEGRAL_LIMIT;
  if (pid->integral < -INTEGRAL_LIMIT) pid->integral = -INTEGRAL_LIMIT;
  
  // Bergn D-delen kun visst kd er satt
  int32_t filt_meas = (int32_t)((964LL * (int32_t)pid->filt_measurement_prev + 60LL * (int32_t)measured_value) / 1024LL);
  int32_t delta_filt = filt_meas - pid->filt_measurement_prev;
  if(pid->Kd != 0) {
    pid->derivative = - (pid->Kd * (int32_t)INVERSE_TIME_STEP * delta_filt );
  }
  // Total output
  int32_t unsat = pid->proportional + pid->integral + pid->derivative;
  int32_t sat = unsat;
  // Outgangsbegrensning
  if(sat > OUTPUT_LIMIT) sat = OUTPUT_LIMIT;
  if(sat < -OUTPUT_LIMIT) sat = -OUTPUT_LIMIT;
  // Back-calculation for anti-windup
  if (sat != unsat) {
    if(pid->kb != 0) { 
      int32_t diff = sat - unsat;
      int32_t correction = (diff * pid->kb) / (int32_t)INVERSE_TIME_STEP;
      pid->integral += correction;
      if (pid->integral > INTEGRAL_LIMIT) pid->integral = INTEGRAL_LIMIT;
      if (pid->integral < -INTEGRAL_LIMIT) pid->integral = -INTEGRAL_LIMIT;
    }
  }

  pid->output = sat;
  pid->err_prev = pid->err;
  pid->integral_prev = pid->integral;
  pid->filt_measurement_prev = filt_meas;
}



