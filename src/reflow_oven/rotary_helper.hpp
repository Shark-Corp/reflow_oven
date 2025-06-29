#ifndef _ROTARY_HELPER_H_
#define _ROTARY_HELPER_H_

#include "util.hpp"
#include <Arduino.h>
#include <RotaryEncoder.h>


#define ROTARY_CLK_PIN  25
#define ROTARY_DT_PIN   26
#define ROTARY_SW_PIN   27

#define ROT_STEP_SIZE 2 // 1 step is actually 2 tick apparently; TODO: check if can be fixed at init


extern RotaryEncoder *rot_encoder;

void rotary_init(void);
IRAM_ATTR void rotary_check_pos(void);
int32_t rotary_get_pos(void);
void rotary_set_pos(int32_t pos);
void rotary_push_pos(void);
void rotary_pull_pos(void);


#endif