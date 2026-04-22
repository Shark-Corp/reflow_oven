#include "ssr_helper.hpp"

void ssr_init(void) {
    pinMode(SSR_PIN, OUTPUT); 
    analogWriteFrequency(SSR_PWM_FREQ);
    digitalWrite(SSR_PIN, LOW);
} 

// duty cycle in percentage 0-100
void ssr_set_heat(uint8_t duty_cycle) {
    if(duty_cycle < SSR_MIN_DC) { // off if DC < 5%
        analogWrite(SSR_PIN, 0);
        digitalWrite(SSR_PIN, LOW);
    } else {
        duty_cycle = (duty_cycle >100)? 100 : duty_cycle;

        analogWrite(SSR_PIN, map(duty_cycle, 0, 100, 0, 255));
    }
}

float error_integral = 0.0;
float last_error = 0.0;
float ssr_get_pid_dc(float current_temp, float target) {
    float error = target - current_temp;
    error_integral += error;
    if(error_integral > PID_I_MAX) error_integral = PID_I_MAX;
    else if(error_integral < PID_I_MIN) error_integral = PID_I_MIN;

    float p_term = PID_P_FAC * error;
    float i_term = PID_I_FAC * error_integral;
    float d_term = PID_D_FAC * (last_error - error);

    float result = p_term + i_term + d_term;

    if(result > PID_MAX) result = PID_MAX;
    else if(result < PID_MIN) result = PID_MIN;

    return result;
} 

void ssr_reset_pid(void) {
    last_error = 0.0;
    error_integral = 0.0;
}