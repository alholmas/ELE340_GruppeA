#ifndef __PID_H__
#define __PID_H__
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "main.h"


typedef struct {
  // PID parmetere
  int32_t Kp; // Proposjonal forsterkning
  int32_t Ki; // Integral forsterkning
  int32_t Kd; // Derivasjons forsterkning

  int32_t kaw; // Anti-windup forsterkning

  int32_t setpoint;       // Refranse/setpunkt y_r(k)
  
  int32_t proportional;   // Proportional del U_p(k)
  int32_t integral;       // Integral del U_i(k)
  int32_t integral_prev;  // Forrige integral del U_i(k-1)
  int32_t derivative;     // Derivative del U_d(k)

  int32_t err;            // Nåværende avvik e(k)
  int32_t err_prev;       // Forrige avvik e(k-1)

  int32_t filt_measurement_prev; // Forrige filtered måling y_fm(k-1)

  int32_t output;             // Pid pådrag U(k)
  int32_t anti_windup;        // Anti-windup verdi AW(k)

} pid_t;


void pid_init(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td, uint16_t kaw, uint16_t setpoint);
void update_pid_parameters(pid_t *pid, uint16_t Kp, uint16_t Ti, uint16_t Td,  uint16_t kaw, uint16_t setpoint);
void reset_pid(pid_t *pid);
void compute_PID_Output(pid_t *pid, uint16_t measured_value);




#ifdef __cplusplus
}
#endif
#endif /*__ PID_H__ */
