#include "pid.h"


#define TIME_STEP 0.01
#define INVERSE_TIME_STEP 100
#define OUTPUT_LIMIT 2500


void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint16_t integral_limit) 
{
  
  pid->Kp = Kp;
  pid->Ki = (Ti != 0) ? (Kp / Ti) : 0;;
  pid->Kd = Kp * Td;

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


void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint16_t integral_limit) {

  pid->Kp = Kp;
  pid->Ki = (Ti != 0) ? (Kp / Ti) : 0;
  pid->Kd = Kp * Td;

  pid->setpoint = setpoint;
  pid->integral_limit = integral_limit;
}

void reset_pid(pid_t *pid) {
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


void compute_PID_Output(pid_t *pid, uint16_t measured_value) {
  // Beregn error
  pid->error = measured_value - pid->setpoint;

  // Beregn p-delen
  pid->proportional = pid->Kp * pid->error;

  // Beregn i-delen
  pid->integral = pid->integral_prev + (pid->Ki * INVERSE_TIME_STEP) * (pid->error + pid->error_prev)/2; 
  // pid->integral = pid->integral_prev + (pid->Ki/TIME_STEP) * (pid->error + pid->error_prev)/2;
  
  if (pid->integral > pid->integral_limit) {
      pid->integral = pid->integral_limit;
  } else if (pid->integral < -pid->integral_limit) {
      pid->integral = -pid->integral_limit;
  }

  // Filter måleverdi for d-delen (lavpass FIR filter)
  // pid->filtered_measurement = (941 * pid->filtered_measurement_prev + 59 * measured_value)/1000;
  pid->filtered_measurement = (964 * pid->filtered_measurement_prev + 60 * measured_value)/1024;

  // Beregn d-delen
  pid->derivative = -(pid->Kd * INVERSE_TIME_STEP)*(pid->filtered_measurement - pid->filtered_measurement_prev);
  // pid->derivative = -(pid->Kd/TIME_STEP)*(pid->filtered_measurement - pid->filtered_measurement_prev);

  // Beregn total PID output
  pid->output = pid->proportional + pid->integral + pid->derivative;
  if (pid->output > pid->output_limit) {
      pid->output = pid->output_limit;
  } else if (pid->output < -pid->output_limit) {
      pid->output = -pid->output_limit;
  }

  pid->error_prev = pid->error;
  pid->filtered_measurement_prev = pid->filtered_measurement;
}




