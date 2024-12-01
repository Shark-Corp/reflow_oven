#include "rotary_helper.hpp"



RotaryEncoder *rot_encoder = nullptr;
int32_t pushed_pos = 0;

void rotary_init(void) {
    pinMode(ROTARY_SW_PIN, INPUT_PULLUP); // External pull up on PCB, but still activated here
    pinMode(ROTARY_DT_PIN, INPUT); 
    pinMode(ROTARY_CLK_PIN, INPUT); 

    rot_encoder = new RotaryEncoder(ROTARY_CLK_PIN, ROTARY_DT_PIN, RotaryEncoder::LatchMode::TWO03);

    attachInterrupt(digitalPinToInterrupt(ROTARY_CLK_PIN), rotary_check_pos, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_DT_PIN), rotary_check_pos, CHANGE);
}

void rotary_check_pos(void) {
  rot_encoder->tick(); // just call tick() to check the state.
}

int32_t rotary_get_pos(void) {
    return (rot_encoder->getPosition() / ROT_STEP_SIZE);
}
void rotary_set_pos(int32_t pos) {
    rot_encoder->setPosition(pos * ROT_STEP_SIZE);
    return;
}

void rotary_push_pos(void) {
    pushed_pos = rot_encoder->getPosition();
}
void rotary_pull_pos(void) {
    rot_encoder->setPosition(pushed_pos);
}
