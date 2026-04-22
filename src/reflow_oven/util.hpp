#ifndef _UTIL_H_
#define _UTIL_H_

#include <Arduino.h>
#include <EEPROM.h>

#define DEBUG_LED_PIN 2

#define SPI_CLK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 23

#define EEPROM_ADDR_TEMP_POINT_1    10 // 2 bytes
#define EEPROM_ADDR_TEMP_POINT_2    12 // 2 bytes
#define EEPROM_ADDR_TEMP_SLOPE_A    14 // 4 bytes
#define EEPROM_ADDR_TEMP_SLOPE_B    18 // 4 bytes
#define EEPROM_ADDR_TEMP_SLOPE_C    22 // 4 bytes
#define EEPROM_ADDR_TEMP_DURATION_A 26 // 2 bytes
#define EEPROM_ADDR_TEMP_DURATION_B 28 // 2 bytes

// 2+2 +4+4+4 +2+2 = 20 bytes


/*==========================================================
=============== Targeted Soldering Curve ===================
============================================================

T2. . . . . .| . . . . . . . . . ______
             |                  /!    !\
             |              SB / !    ! \
T1 . . . . . |. . . __________/  !    !  \  SC
             |     /!        !   !    !   \
             | SA / !        !   !    !    \
t0. . . . . .| . /  !        !   !    !     \
             +--+---+--------+---+----+------+----
                |d0 |   DA   |d1 | DB |  d2  |

t0 is room temperature (can't be set/changed)
d0, d1 and d2 are dependant to respective T and S
T1 and T2 are targeted temperatures
SA, SB and SC are slopes to reach targeted temperatures
(SC is not necesserally to be used, maybe can just use ambient cooling ?)
DA and DB are duration of the flat parts
===========================================================*/

// More info on internet, like : https://ww1.microchip.com/downloads/en/Appnotes/00233D.pdf

// TODO: update Slopes and temperatures structure according to the graph above
// TODO: add structure for delays
// TODO: update whats displayed on screen

// TEMPERATURES     // values for Pb free from TI's AN
typedef struct temperature_points_t {
    int16_t tp1; // ~150-200°C
    int16_t tp2; // ~245-260°C -> melting temp
} temperature_points_t;

typedef struct temperature_slopes_t {
    float tsA; // 3°C/s
    float tsB; // 2°C / s
    float tsC; // -4°C / s
} temperature_slopes_t;

typedef struct temperature_durations_t {
    int16_t tdA; // 60-180 s
    int16_t tdB; // 60-150 s
} temperature_durations_t;


void copy_temp_points(temperature_points_t source, temperature_points_t *target);
void copy_temp_slopes(temperature_slopes_t source, temperature_slopes_t *target);
void copy_temp_duration(temperature_durations_t source, temperature_durations_t *target);

void temp_load_points(temperature_points_t *temp_points);
void temp_load_slopes(temperature_slopes_t *temp_slopes);
void temp_load_duration(temperature_durations_t *temp_duration);

void temp_save_points(temperature_points_t temp_points);
void temp_save_slopes(temperature_slopes_t temp_slopes);
void temp_save_duration(temperature_durations_t temp_duration);

uint16_t reflow_duration(float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs);
uint16_t start_cooldown_time(float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs);
float get_current_target_temp(uint16_t current_time, float start_temp, temperature_points_t pts, temperature_slopes_t slps, temperature_durations_t durs);

// NON-BLOCKING DELAY
typedef struct nb_delay_t {
    uint32_t duration_ms;
    uint32_t last_check;
} nb_delay_t;

void nb_delay_init(nb_delay_t *delay, uint32_t duration_ms);
bool nb_delay_check(nb_delay_t *delay);


#endif