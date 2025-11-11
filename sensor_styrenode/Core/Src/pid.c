#include "pid.h"
#include <stdint.h>


#define TIME_STEP 0.01
#define INVERSE_TIME_STEP 100
#define OUTPUT_LIMIT 250000


void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint32_t integral_limit) 
{
  
  pid->Kp = Kp;
  pid->Ki = (Ti != 0) ? (Kp / Ti) : 0;
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

  if (pid->Ki == 0) {
    pid->integral = 0;
    pid->integral_prev = 0;
  }

}


void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint32_t integral_limit) 
{
  pid->Kp = Kp;
  pid->Ki = (Ti != 0) ? (Kp / Ti) : 0;
  pid->Kd = Kp * Td;

  pid->setpoint = setpoint;
  pid->integral_limit = integral_limit;
  if (pid->Ki == 0) {
    pid->integral = 0;
    pid->integral_prev = 0;
  }
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


// void compute_PID_Output(pid_t *pid, uint16_t measured_value) {
//   // Beregn error
//   pid->error = measured_value - pid->setpoint;

//   // Beregn p-delen
//   pid->proportional = pid->Kp * pid->error;

//   // Beregn i-delen
//   pid->integral = pid->integral_prev + ((uint32_t)(pid->Ki *((uint32_t)(pid->error + pid->error_prev)))) / (uint32_t)(200); 
//   // pid->integral = pid->integral_prev + (pid->Ki/TIME_STEP) * (pid->error + pid->error_prev)/2;
  
//   if (pid->integral > pid->integral_limit) {
//       pid->integral = pid->integral_limit;
//   } else if (pid->integral < -pid->integral_limit) {
//       pid->integral = -pid->integral_limit;
//   }

//   // Filter måleverdi for d-delen (lavpass FIR filter)
//   // pid->filtered_measurement = (941 * pid->filtered_measurement_prev + 59 * measured_value)/1000;
//   pid->filtered_measurement = (964 * pid->filtered_measurement_prev + 60 * measured_value)/1024;

//   // Beregn d-delen
//   pid->derivative = -(pid->Kd * (uint32_t)INVERSE_TIME_STEP)*(pid->filtered_measurement - pid->filtered_measurement_prev);
//   // pid->derivative = -(pid->Kd/TIME_STEP)*(pid->filtered_measurement - pid->filtered_measurement_prev);

//   // Beregn total PID output
//   pid->output = pid->proportional + pid->integral + pid->derivative;
//   if (pid->output > pid->output_limit) {
//       pid->output = pid->output_limit;
//   } else if (pid->output < -pid->output_limit) {
//       pid->output = -pid->output_limit;
//   }

//   pid->error_prev = pid->error;
//   pid->filtered_measurement_prev = pid->filtered_measurement;
//   pid->integral_prev = pid->integral;
// }


void compute_PID_Output(pid_t *pid, uint16_t measured_value) {
  // Beregn error (bruk signed)
  int32_t error = (int32_t)measured_value - (int32_t)pid->setpoint;
  pid->error = error;

  // P-delen (bruk 64-bit temp for sikkerhet)
  int64_t P = (int64_t)pid->Kp * (int64_t)error; // Kp som du leverer - kan være skalert av deg
  // Hvis du bruker en ekstern skala S, del med S her: P = P / S;

  // I-delen (trapezoidal)
  // Ki antas å være (Kp / Ti) slik koden tidligere gjorde. Vi gjør:
  // incr = Ki * (error + error_prev) / (2 * INVERSE_TIME_STEP)
  int64_t sum_err = (int64_t)error + (int64_t)pid->error_prev;
  int64_t incr = 0;
  if (pid->Ki != 0) {
    // bruk 64-bit og del sist for å holde presisjon
    incr = ((int64_t)pid->Ki * sum_err) / (2LL * (int64_t)INVERSE_TIME_STEP);
  }
  int64_t I = (int64_t)pid->integral_prev + incr;

  // limiter integral
  if (I > (int64_t)pid->integral_limit) I = pid->integral_limit;
  if (I < -(int64_t)pid->integral_limit) I = -(int64_t)pid->integral_limit;

  // Filter måleverdi for d-delen (lowpass)
  int32_t filt = (int32_t)((964LL * (int64_t)pid->filtered_measurement_prev + 60LL * (int64_t)measured_value) / 1024LL);

  // D-delen: Kd * delta / dt    -> delta * INVERSE_TIME_STEP
  // Kd i koden er satt som Kp * Td
  int64_t delta_filt = (int64_t)filt - (int64_t)pid->filtered_measurement_prev;
  int64_t D = 0;
  if (pid->Kd != 0) {
    D = - ( (int64_t)pid->Kd * (int64_t)INVERSE_TIME_STEP * delta_filt );
    // del med eventuell skala hvis du bruker en gain-skala S: D = D / S;
  }

  // Total output (bruk 64-bit før clamp)
  int64_t out64 = P + I + D;

  // Clamp til output_limit
  if (out64 > (int64_t)pid->output_limit) out64 = pid->output_limit;
  if (out64 < -(int64_t)pid->output_limit) out64 = - (int64_t)pid->output_limit;

  // Lagre tilbake i pid struct (bruk int32 for output)
  pid->output = (int32_t)out64;

  // Oppdater historikk
  pid->proportional = (int32_t)P;
  pid->integral = (int32_t)I;
  pid->derivative = (int32_t)D;

  pid->error_prev = error;
  pid->filtered_measurement_prev = filt;
  pid->integral_prev = (int32_t)I;
  pid->integral = (int32_t)I;
}

