#include "util.hpp"
#include "lcd_helper.hpp"
#include "rotary_helper.hpp"
#include "temp_sensor.hpp"
#include "os.hpp"

#define EEPROM_SIZE 30 // 2*2 bytes for temperature + 4*1 bytes for slopes + 2*2bytes for durations = 12 + margin (6) + starts at 10

void ISR_rot_sw_pressed(void);

void generate_home_page(void);
void generate_config_page(void);
void home_draw_text(void);

nb_delay_t delay_3s;
temperature_points_t temperature_points;
temperature_points_t tmp_temp_pts;
temperature_slopes_t temperature_slopes;
temperature_slopes_t tmp_temp_slopes;

OS_page *home_page;
OS_button *config_btn, *run_btn;
OS_curve *home_curve;
OS_separator *separator_1, *separator_2;
OS_callback *home_text;

OS_page *config_page;
OS_button *cancel_btn, *apply_btn;
OS_nb_input *temp_1_input, *temp_2_input, *slope_a_input, *slope_b_input, *slope_c_input;
OS_callback *config_text;

OS_page *current_page, *next_page;

bool sw_clicked;

void setup() {
  pinMode(DEBUG_LED_PIN, OUTPUT);
  digitalWrite(DEBUG_LED_PIN, LOW);

  Serial.begin(115200);
  Serial.println("Reflow oven booting...");

  // init libraries
  sw_clicked = false;
  EEPROM.begin(EEPROM_SIZE);
  thermo_init_pins(); // needs to be called before lcd_init, because SPI stuff creates problem other wise
  lcd_init();
  rotary_init();
  thermo_init_spi(&lcd_handle.getSPIinstance());
  temp_load_points(&temperature_points);
  temp_load_slopes(&temperature_slopes);
  copy_temp_points(temperature_points, &tmp_temp_pts);
  copy_temp_slopes(temperature_slopes, &tmp_temp_slopes);

  // setup interrupts
  attachInterrupt(digitalPinToInterrupt(ROTARY_SW_PIN), ISR_rot_sw_pressed, FALLING);

  // setup os' pages
  generate_home_page();
  generate_config_page();

  // enable home page
  next_page = current_page = home_page;
  current_page->enable();

  // init delays
  nb_delay_init(&delay_3s, 3000);
}

int32_t pos = 0;
bool test = false;
void loop() {
  if(nb_delay_check(&delay_3s)) {
    // digitalWrite(DEBUG_LED_PIN, !digitalRead(DEBUG_LED_PIN));
    thermo_print_temp();
  }

  // if(current_page != next_page) {
  //   current_page = next_page;
  //   current_page->enable();
  // }

  current_page->update_selection(&sw_clicked);
}

volatile bool in_sw_isr = false; // ISR variable should be volatile (TODO: why ?)
//TODO: do something to manage switch in "OS" for example just throw a flag, that could be
// managed in "update_selection" or another function
void ISR_rot_sw_pressed(void) {
  if(!in_sw_isr && !sw_clicked && !digitalRead(ROTARY_SW_PIN)) {
    in_sw_isr = true;

    sw_clicked = true;

    in_sw_isr = false;
  }
}

void generate_home_page(void) {
  home_page = new OS_page();

  config_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_DARKGREEN, 10, 10, 5, 5, "Config", go_to_config_page);
  home_page->add_item(config_btn);

  run_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_DARKGREEN, config_btn->get_width()+20, 10, 10, 5, "Run", NULL);
  home_page->add_item(run_btn);

  separator_1 = new OS_separator(TFT_LIGHTGREY, 31);
  home_page->add_item(separator_1);

  // TODO: add different color for text
  home_curve = new OS_curve(TFT_WHITE, TFT_LIGHTGREY, TFT_BLACK, 0, 35, false);
  home_page->add_item(home_curve);

  separator_2 = new OS_separator(TFT_LIGHTGREY, 103);
  home_page->add_item(separator_2);

  home_text = new OS_callback(home_draw_text);
  home_page->add_item(home_text);
}

void home_draw_text(void) {
  uint8_t start = 120;

  lcd_text(15, 106, "o", TFT_LIGHTGREY);
  lcd_text(10, 110, "( C)", TFT_LIGHTGREY);
      lcd_text(80, 106, "o", TFT_LIGHTGREY);
      lcd_text(75, 110, "( C/s)", TFT_LIGHTGREY);
  
  lcd_text(5, start, "0:", TFT_LIGHTGREY);lcd_text(16, start, "Room T", TFT_WHITE);
      lcd_text(70, start, "A: ", TFT_LIGHTGREY);lcd_text(81, start, String(temperature_slopes.tsA), TFT_WHITE);

  lcd_text(5, start+TXT_H, "1:", TFT_LIGHTGREY);lcd_text(16, start+TXT_H, String(temperature_points.tp1), TFT_WHITE);
      lcd_text(70, start+TXT_H, "B: ", TFT_LIGHTGREY);lcd_text(81, start+TXT_H, String(temperature_slopes.tsB), TFT_WHITE);

  lcd_text(5, start+2*TXT_H, "2:", TFT_LIGHTGREY);lcd_text(16, start+2*TXT_H, String(temperature_points.tp2), TFT_WHITE);
      lcd_text(70, start+2*TXT_H, "C: ", TFT_LIGHTGREY);lcd_text(81, start+2*TXT_H, String(temperature_slopes.tsC), TFT_WHITE);
} 

void generate_config_page(void) {
  config_page = new OS_page();

  apply_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_DARKGREEN, 5, 5, 3, 3, "Apply", apply_to_home_page);
  config_page->add_item(apply_btn);

  cancel_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_DARKGREEN, 70, 5, 3, 3, "Cancel", cancel_to_home_page);
  config_page->add_item(cancel_btn);

  temp_1_input = new OS_nb_input(50, 30, &tmp_temp_pts.tp1);
  config_page->add_item(temp_1_input);
  temp_2_input = new OS_nb_input(50, 46, &tmp_temp_pts.tp2);
  config_page->add_item(temp_2_input);

  slope_a_input = new OS_nb_input(55, 70, &tmp_temp_slopes.tsA);
  config_page->add_item(slope_a_input);
  slope_b_input = new OS_nb_input(55, 86, &tmp_temp_slopes.tsB);
  config_page->add_item(slope_b_input);
  slope_c_input = new OS_nb_input(55, 102, &tmp_temp_slopes.tsC);
  config_page->add_item(slope_c_input);

  config_text = new OS_callback(config_draw_text);
  config_page->add_item(config_text);
}

void config_draw_text(void) {
  lcd_text(5, 33, "Temp 1:", TFT_WHITE);
  lcd_text(5, 49, "Temp 2:", TFT_WHITE);

  lcd_text(5, 73, "Slope A:", TFT_WHITE);
  lcd_text(5, 89, "Slope B:", TFT_WHITE);
  lcd_text(5, 105, "Slope C:", TFT_WHITE);
}

// button callbacks
void go_to_config_page(void) {
  current_page = config_page;
  current_page->enable();
}

void apply_to_home_page(void) {
  copy_temp_points(tmp_temp_pts, &temperature_points);
  copy_temp_slopes(tmp_temp_slopes, &temperature_slopes);

  temp_save_points(temperature_points);
  temp_save_slopes(temperature_slopes);

  current_page = home_page;
  current_page->enable();
}

void cancel_to_home_page(void) {
  copy_temp_points(temperature_points, &tmp_temp_pts);
  copy_temp_slopes(temperature_slopes, &tmp_temp_slopes);
  
  current_page = home_page;
  current_page->enable();
}