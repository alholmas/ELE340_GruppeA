#ifndef __PID_H__
#define __PID_H__
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"


typedef struct {
  uint16_t Kp; // Proportional gain
  uint16_t Ki; // Integral forsterkning
  uint16_t Kd; // Derivative forsterkning

  uint16_t setpoint;        // Refranse/setpunkt y_r(k)
  uint16_t integral_limit;  // Begrensning for integral delen

  int32_t proportional;   // Proportional del U_p(k)
  int32_t integral;       // Integral del U_i(k)
  int32_t integral_prev;  // Forrige integral del U_i(k-1)
  int32_t derivative;     // Derivative del U_d(k)

  int32_t error;          // Avvik e(k)
  int32_t error_prev;     // Forrige avvik e(k-1)

  int32_t filtered_measurement;      // Filtered measurement y_fm(k)
  int32_t filtered_measurement_prev; // Forrige filtered measurement y_fm(k-1)


  int32_t output;          // Pid pådrag U(k)
  int32_t output_limit;    // Pid pådragsbegresning

} pid_t;


void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint16_t integral_limit);
void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t setpoint, uint16_t integral_limit);
void reset_pid(pid_t *pid);
void compute_PID_Output(pid_t *pid, uint16_t measured_value);




#ifdef __cplusplus
}
#endif
#endif /*__ PID_H__ */
