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

void temp_load_points(temperature_points_t *temp_points) {
    uint16_t tp1_msb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_1);
    uint8_t tp1_lsb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_1 + 1);
    temp_points->tp1 = (tp1_msb << 8) | tp1_lsb;

    uint16_t tp2_msb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_2);
    uint8_t tp2_lsb = EEPROM.read(EEPROM_ADDR_TEMP_POINT_2 + 1);
    temp_points->tp2 = (tp2_msb << 8) | tp2_lsb;
}
void temp_load_slopes(temperature_slopes_t *temp_slopes) {
    temp_slopes->tsA = EEPROM.read(EEPROM_ADDR_TEMP_SLOPE_A);
    temp_slopes->tsB = EEPROM.read(EEPROM_ADDR_TEMP_SLOPE_B);
    temp_slopes->tsC = EEPROM.read(EEPROM_ADDR_TEMP_SLOPE_C);
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
    EEPROM.write(EEPROM_ADDR_TEMP_SLOPE_A, temp_slopes.tsA);
    EEPROM.write(EEPROM_ADDR_TEMP_SLOPE_B, temp_slopes.tsB);
    EEPROM.write(EEPROM_ADDR_TEMP_SLOPE_C, temp_slopes.tsC);

    EEPROM.commit();
}

