#include "util.hpp"
#include "lcd_helper.hpp"
#include "rotary_helper.hpp"
#include "os.hpp"

void ISR_rot_sw_pressed(void);

void generate_home_page(void);
void generate_config_page(void);
void home_draw_text(void);

nb_delay_t led_delay;
temperature_points_t temperature_points;
temperature_slopes_t temperature_slopes;

OS_page *home_page;
OS_button *config_btn, *run_btn;
OS_curve *home_curve;
OS_separator *separator_1, *separator_2;
OS_callback *home_text;

OS_page *config_page;
OS_button *return_btn;
OS_nb_input *temp_1_input, *temp_2_input, *slope_a_input, *slope_b_input, *slope_c_input;
OS_callback *config_text;

OS_page *current_page, *next_page;

bool sw_clicked;

void setup() {
  pinMode(DEBUG_LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println("Reflow oven booting...");

  // init libraries
  sw_clicked = false;
  lcd_init();
  rotary_init();
  temp_load_points(temperature_points);
  temp_load_slopes(temperature_slopes);

  // setup interrupts
  attachInterrupt(digitalPinToInterrupt(ROTARY_SW_PIN), ISR_rot_sw_pressed, FALLING);

  // setup os' pages
  generate_home_page();
  generate_config_page();

  // enable home page
  next_page = current_page = home_page;
  current_page->enable();

  // init delays
  nb_delay_init(&led_delay, 3000);
}

int32_t pos = 0;
void loop() {
  // if(nb_delay_check(&led_delay)) {
  //   digitalWrite(DEBUG_LED_PIN, !digitalRead(DEBUG_LED_PIN));
  // }

  // if(current_page != next_page) {
  //   current_page = next_page;
  //   current_page->enable();
  // }

  current_page->update_selection(&sw_clicked);
}

volatile bool in_sw_isr = false; // ISR variable should be volatile (why ?)
//TODO: do something to manage switch in "OS" for example just throw a flag, that could be
// managed in "update_selection" or another function
void ISR_rot_sw_pressed(void) {
  if(!in_sw_isr && !sw_clicked) {
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

  return_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_DARKGREEN, 5, 5, 3, 3, "Return", go_to_home_page);
  config_page->add_item(return_btn);

  temp_1_input = new OS_nb_input(50, 30, &temperature_points.tp1);
  config_page->add_item(temp_1_input);
  temp_2_input = new OS_nb_input(50, 46, &temperature_points.tp2);
  config_page->add_item(temp_2_input);

  slope_a_input = new OS_nb_input(55, 70, &temperature_slopes.tsA);
  config_page->add_item(slope_a_input);
  slope_b_input = new OS_nb_input(55, 86, &temperature_slopes.tsB);
  config_page->add_item(slope_b_input);
  slope_c_input = new OS_nb_input(55, 102, &temperature_slopes.tsC);
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

void go_to_home_page(void) {
  current_page = home_page;
  current_page->enable();
}