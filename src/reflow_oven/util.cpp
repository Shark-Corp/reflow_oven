#include "util.hpp"

void nb_delay_init(nb_delay_t *delay, uint32_t duration_ms) {
    delay->duration_ms = duration_ms;
    delay->last_check = millis();
}
bool nb_delay_check(nb_delay_t *delay) {
    if(millis() - delay->last_check >= delay->duration_ms) {
        delay->last_check = millis();
        return true;
    } else {
        return false;
    }
}

void copy_temp_points(temperature_points_t source, temperature_points_t *target) {
    target->tp1 = source.tp1;
    target->tp2 = source.tp2;
}
void copy_temp_slopes(temperature_slopes_t source, temperature_slopes_t *target) {
    target->tsA = source.tsA;
    target->tsB = source.tsB;
    target->tsC = source.tsC;
}
void copy_temp_duration(temperature_durations_t source, temperature_durations_t *target) {
    target->tdA = source.tdA;
    target->tdB = source.tdB;
}


void temp_load_points(temperature_points_t *temp_points) {
    uint16_t tp1_msb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_1);
    uint8_t tp1_lsb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_1 + 1);
    temp_points->tp1 = (tp1_msb << 8) | tp1_lsb;

    uint16_t tp2_msb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_2);
    uint8_t tp2_lsb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_2 + 1);
    temp_points->tp2 = (tp2_msb << 8) | tp2_lsb;
}
void temp_load_slopes(temperature_slopes_t *temp_slopes) { 
    temp_slopes->tsA = EEPROM.get(EEPROM_ADDR_TEMP_SLOPE_A, temp_slopes->tsA);
    temp_slopes->tsB = EEPROM.get(EEPROM_ADDR_TEMP_SLOPE_B, temp_slopes->tsB);
    temp_slopes->tsC = EEPROM.get(EEPROM_ADDR_TEMP_SLOPE_C, temp_slopes->tsC);
}
void temp_load_duration(temperature_durations_t *temp_duration) {
    uint16_t tdA_msb = EEPROM.read(EEPROM_ADDR_TEMP_DURATION_A);
    uint8_t tdA_lsb = EEPROM.read(EEPROM_ADDR_TEMP_DURATION_A + 1);
    temp_duration->tdA = (tdA_msb << 8) | tdA_lsb;

    uint16_t tdB_msb = EEPROM.read(EEPROM_ADDR_TEMP_DURATION_B);
    uint8_t tdB_lsb = EEPROM.read(EEPROM_ADDR_TEMP_DURATION_B + 1);
    temp_duration->tdB = (tdB_msb << 8) | tdB_lsb;
}

void temp_save_points(temperature_points_t temp_points) {
    // Temp Point 1
    EEPROM.write(EEPROM_ADDR_TEMP_POINT_1,  (temp_points.tp1 >> 8) & 0xFF); // msb
    EEPROM.write(EEPROM_ADDR_TEMP_POINT_1+1, temp_points.tp1 & 0xFF); // lsb 

    // Temp Point 2
    EEPROM.write(EEPROM_ADDR_TEMP_POINT_2,  (temp_points.tp2 >> 8) & 0xFF); // msb
    EEPROM.write(EEPROM_ADDR_TEMP_POINT_2+1, temp_points.tp2 & 0xFF); // lsb

    EEPROM.commit();
}
void temp_save_slopes(temperature_slopes_t temp_slopes) {
    // Temp slopes A-C
    EEPROM.put(EEPROM_ADDR_TEMP_SLOPE_A, temp_slopes.tsA);
    EEPROM.put(EEPROM_ADDR_TEMP_SLOPE_B, temp_slopes.tsB);
    EEPROM.put(EEPROM_ADDR_TEMP_SLOPE_C, temp_slopes.tsC);

    EEPROM.commit();
}

void temp_save_duration(temperature_durations_t temp_duration) {
    // Temp Duration A
    EEPROM.write(EEPROM_ADDR_TEMP_DURATION_A,  (temp_duration.tdA >> 8) & 0xFF); // msb
    EEPROM.write(EEPROM_ADDR_TEMP_DURATION_A+1, temp_duration.tdA & 0xFF); // lsb 

    // Temp Duration B
    EEPROM.write(EEPROM_ADDR_TEMP_DURATION_B,  (temp_duration.tdB >> 8) & 0xFF); // msb
    EEPROM.write(EEPROM_ADDR_TEMP_DURATION_B+1, temp_duration.tdB & 0xFF); // lsb

    EEPROM.commit();
}

uint16_t reflow_duration(float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs) {
    uint16_t tot_duration = 0;
    
    tot_duration += start_cooldown_time(start_temp, pts, slps, durs);
    tot_duration += (50 - pts.tp2) / slps.tsC; // slopes C duration (considering 50°C is finished)

    return tot_duration;
}

uint16_t start_cooldown_time(float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs) {
    uint16_t tot_duration = 0;
    
    tot_duration += durs.tdA;
    tot_duration += durs.tdB;

    tot_duration += (pts.tp1 - (int16_t)start_temp) / slps.tsA; // slopes A duration
    tot_duration += (pts.tp2 - pts.tp1) / slps.tsB; // slopes B duration

    return tot_duration;
}

float get_current_target_temp(uint16_t current_time, float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs) {
    // TODO: improve for slope moment and cooldown moment
    float target_temp = 0.0;

    if(current_time < (uint16_t)(durs.tdA + (pts.tp1 - start_temp) / slps.tsA)) {
        target_temp = pts.tp1; 
    } else if(current_time < start_cooldown_time(start_temp, pts, slps, durs)) {
        target_temp = pts.tp2;
    } else {
        target_temp = 0.0;
    }

    return target_temp;
}
