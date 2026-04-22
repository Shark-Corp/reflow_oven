#ifndef _SSR_HELPER_H_
#define _SSR_HELPER_H_

#include "util.hpp"
#include <Arduino.h>

#define SSR_PIN   15

#define SSR_PWM_FREQ    25
#define SSR_MIN_DC      5

// TODO: select the "perfect" values for the PID
#define PID_P_FAC       3.0  
#define PID_I_FAC       0.11
#define PID_D_FAC       0.1

#define PID_I_MIN       -160.0
#define PID_I_MAX       160.0

#define PID_MAX         100.0
#define PID_MIN         0.0


void ssr_init(void);
void ssr_set_heat(uint8_t duty_cycle);
float ssr_get_pid_dc(float current_temp, float target);
void ssr_reset_pid(void);



#endif