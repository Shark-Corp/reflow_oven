#include "util.hpp"
#include "lcd_helper.hpp"
#include "rotary_helper.hpp"
#include "ssr_helper.hpp"
#include "temp_sensor.hpp"
#include "os.hpp"

#define EEPROM_SIZE 40 // 2*2 bytes for temperature + 3*4 bytes for slopes + 2*2bytes for durations = 20 + margin (10) + starts at 10

#define TICK_MS       1000
#define OVERSAMPLING  5
#define FAST_TICK_MS  TICK_MS/OVERSAMPLING

void ISR_rot_sw_pressed(void);

void generate_home_page(void);
void generate_config_page(void);
void generate_run_page(void);
void home_draw_text(void);
void run_draw_text(void);
void run_print_time(uint16_t time_in_s);
void run_print_temp(float temperature);


nb_delay_t temp_tick;
nb_delay_t fast_tick;

temperature_points_t temperature_points;
temperature_points_t tmp_temp_pts;
temperature_slopes_t temperature_slopes;
temperature_slopes_t tmp_temp_slopes;
temperature_durations_t temperature_durations;
temperature_durations_t tmp_temp_dur;


OS_page *home_page;
OS_button *config_btn, *run_btn;
OS_curve *home_curve;
OS_separator *separator_1, *separator_2;
OS_callback *home_text;

OS_page *config_page;
OS_button *cancel_btn, *apply_btn;
OS_nb_input *temp_1_input, *temp_2_input;
OS_nb_input *slope_a_input, *slope_b_input, *slope_c_input;
OS_nb_input *duration_a_input, *duration_b_input;
OS_callback *config_text;

OS_page *run_page;
OS_button *run_back_btn, *run_start_stop_btn;
OS_curve *run_curve;
OS_live_curve *run_live_curve;
OS_callback *run_text;



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
  ssr_init();
  thermo_init_spi(&lcd_handle.getSPIinstance());
  temp_load_points(&temperature_points);
  temp_load_slopes(&temperature_slopes);
  temp_load_duration(&temperature_durations);
  copy_temp_points(temperature_points, &tmp_temp_pts);
  copy_temp_slopes(temperature_slopes, &tmp_temp_slopes);
  copy_temp_duration(temperature_durations, &tmp_temp_dur);

  // setup interrupts
  attachInterrupt(digitalPinToInterrupt(ROTARY_SW_PIN), ISR_rot_sw_pressed, FALLING);

  // setup os' pages
  generate_home_page();
  generate_config_page();
  generate_run_page();

  // enable home page
  next_page = current_page = home_page;
  current_page->enable();

  // init delays
  nb_delay_init(&fast_tick, FAST_TICK_MS);
  nb_delay_init(&temp_tick, TICK_MS);
}

int32_t pos = 0;
float current_temperature = 0, start_temperature = 0;
uint16_t running_time = 0, reflow_stop_time = 0;
int16_t temperatures_per_s[1500]; // TODO: Might do it dynamically
bool running = false;

float current_target_temp = 0.0;
int16_t start_temp_target = 70; // TODO: move to config.
bool enable_heat = false;
bool start_temp_reached = false;

float oversample_temp = 0.0;
uint8_t oversample_phase = 0;

uint16_t current_dc = 0;

void loop() {
  if(nb_delay_check(&fast_tick)) {
    oversample_temp += thermo_read_temp_c();
    oversample_phase++;

    if(oversample_phase >= OVERSAMPLING) {
      current_temperature = oversample_temp/(float)OVERSAMPLING;
      oversample_temp = 0.0;
      oversample_phase = 0;
    }
  }

  if(nb_delay_check(&temp_tick)) {
    if(running) { //TODO: for PID, maybe more regularly, like 2 or 4 HZ instead of 1Hz
      if(!start_temp_reached && (int16_t) current_temperature > start_temp_target) {
        start_temp_reached = true;
      }

      if(start_temp_reached) {
        running_time++;
        temperatures_per_s[running_time] = (int16_t) current_temperature; //save temperature by second

        // TODO: get intermediary temperatures during slopes ?
        current_target_temp = get_current_target_temp(running_time, start_temp_target, temperature_points, temperature_slopes, temperature_durations);
        
        current_dc = ssr_get_pid_dc(current_temperature, current_target_temp);
        ssr_set_heat(current_dc);
        //TODO: merge heater and running ? (like red outline for runing, empty for 0%, green for 25%, yellow for 50%, red for > 75%...)
        if(current_dc < SSR_MIN_DC) { // TODO: make something neater for this...
          enable_heat = false;
          lcd_circle(100, 5, 4, TFT_BLACK);// heating indicator // TODO: remove because it maybe a doublon with enable_heat ?
        } else {
          enable_heat = true;
          lcd_circle(100, 5, 3, TFT_YELLOW);// heating indicator
        }
      } else {
        // else -> wait for temperature to rise. //TODO: show "reaching start temperature..." message
        current_dc = ssr_get_pid_dc(current_temperature, start_temp_target);
        ssr_set_heat(current_dc);
        lcd_circle(100, 5, 3, TFT_YELLOW);// heating indicator
      } 

      // stop reflow when reach final time
      if(running_time >= reflow_stop_time) {
        end_reflow();
      }
    }
    if(current_page == run_page) {
      if(running) {
        run_print_time(running_time);
        run_live_curve->update();
      }
      run_print_temp(current_temperature);
      run_print_targ_temp(current_target_temp);
      run_print_dc(current_dc);
    }
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

  config_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, 10, 10, 5, 5, "Config", go_to_config_page);
  home_page->add_item(config_btn);

  // TODO: disable config if running

  run_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, config_btn->get_width()+20, 10, 10, 5, "Run", go_to_run_page);
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

  if(running)
    lcd_circle(120, 5, 3, TFT_RED);// running indicator
  if(enable_heat)
    lcd_circle(100, 5, 3, TFT_YELLOW);

  lcd_text(15, 106, "o", TFT_LIGHTGREY);
  lcd_text(10, 110, "( C):", TFT_LIGHTGREY);
      lcd_text(80, 106, "o", TFT_LIGHTGREY);
      lcd_text(75, 110, "( C/s):", TFT_LIGHTGREY);
  
  lcd_text(5, start, "0:", TFT_LIGHTGREY);lcd_text(16, start, "Curr T", TFT_WHITE);
      lcd_text(70, start, "A: ", TFT_LIGHTGREY);lcd_text(81, start, String(temperature_slopes.tsA, 1), TFT_WHITE);

  lcd_text(5, start+TXT_H, "1:", TFT_LIGHTGREY);lcd_text(16, start+TXT_H, String(temperature_points.tp1), TFT_WHITE);
      lcd_text(70, start+TXT_H, "B: ", TFT_LIGHTGREY);lcd_text(81, start+TXT_H, String(temperature_slopes.tsB, 1), TFT_WHITE);

  lcd_text(5, start+2*TXT_H, "2:", TFT_LIGHTGREY);lcd_text(16, start+2*TXT_H, String(temperature_points.tp2), TFT_WHITE);
      lcd_text(70, start+2*TXT_H, "C: ", TFT_LIGHTGREY);lcd_text(81, start+2*TXT_H, String(temperature_slopes.tsC, 1), TFT_WHITE);

    lcd_text(5, start+7+3*TXT_H, "(s):", TFT_LIGHTGREY);
      lcd_text(33, start+7+3*TXT_H, "a:", TFT_LIGHTGREY); lcd_text(44, start+6+3*TXT_H, String(temperature_durations.tdA), TFT_WHITE);
      lcd_text(68, start+7+3*TXT_H, "b:", TFT_LIGHTGREY); lcd_text(79, start+6+3*TXT_H, String(temperature_durations.tdB), TFT_WHITE);
} 

void generate_config_page(void) {
  config_page = new OS_page();

  apply_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, 5, 5, 3, 3, "Apply", apply_to_home_page);
  config_page->add_item(apply_btn);

  cancel_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, 60, 5, 3, 3, "Cancel", cancel_to_home_page);
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

  duration_a_input = new OS_nb_input(73, 126, &tmp_temp_dur.tdA);
  config_page->add_item(duration_a_input);
  duration_b_input = new OS_nb_input(73, 142, &tmp_temp_dur.tdB);
  config_page->add_item(duration_b_input);

  config_text = new OS_callback(config_draw_text);
  config_page->add_item(config_text);
}

void config_draw_text(void) {
  if(running)
    lcd_circle(120, 5, 3, TFT_RED); // running indicator
  if(enable_heat)
    lcd_circle(100, 5, 3, TFT_YELLOW);

  lcd_text(5, 33, "Temp 1:", TFT_WHITE);
  lcd_text(5, 49, "Temp 2:", TFT_WHITE);

  lcd_text(5, 73, "Slope A:", TFT_WHITE);
  lcd_text(5, 89, "Slope B:", TFT_WHITE);
  lcd_text(5, 105, "Slope C:", TFT_WHITE);

  lcd_text(5, 129, "Duration a:", TFT_WHITE);
  lcd_text(5, 145, "Duration b:", TFT_WHITE);
}

void generate_run_page(void) {
  run_page = new OS_page();

  run_back_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, 5, 5, 3, 3, "Back", go_to_home_page);
  run_page->add_item(run_back_btn);

  run_start_stop_btn = new OS_button(TFT_BLACK, TFT_GOLD, TFT_RED, 50, 5, 3, 3, "Start", start_stop_reflow);
  run_page->add_item(run_start_stop_btn);    

  run_curve = new OS_curve(TFT_LIGHTGREY, TFT_LIGHTGREY, TFT_BLACK, 0, 35, false, true);
  run_curve->set_params(&temperature_points, &temperature_slopes, &temperature_durations);
  run_page->add_item(run_curve);

  run_live_curve = new OS_live_curve(TFT_RED, run_curve, temperatures_per_s, &running_time);
  run_page->add_item(run_live_curve);

  run_text = new OS_callback(run_draw_text);
  run_page->add_item(run_text);
}

void run_draw_text(void) {
  if(running)
    lcd_circle(120, 5, 3, TFT_RED);// running indicator

  if(enable_heat)
    lcd_circle(100, 5, 3, TFT_YELLOW);// heating indicator
  else
    lcd_circle(100, 5, 4, TFT_BLACK);// heating indicator

  lcd_text(10, 100, "DC%:", TFT_WHITE);
  run_print_dc(current_dc);

  lcd_text(5, 113, "Targ. T:", TFT_WHITE);
  run_print_targ_temp(current_target_temp);

  lcd_text(5, 129, "Elaps. time:", TFT_WHITE);
  run_print_time(running_time);

  lcd_text(5, 145, "Curr. T :", TFT_WHITE);
    lcd_text(48, 141, "o", TFT_WHITE);
  run_print_temp(current_temperature);
}
void run_print_time(uint16_t time_in_s) {
  char seconds_str[4];
  sprintf(seconds_str, "%03u", time_in_s);

  char full_time_str[4]; // Shouldn't exceed 999s
  sprintf(full_time_str, "%03u", reflow_stop_time);

  // remove old text
  lcd_rect(77, 128, 120, 10, TFT_BLACK, true);

  lcd_text(78, 129, seconds_str, TFT_LIGHTGREY);
  lcd_text(96, 129, "/", TFT_LIGHTGREY);
  lcd_text(102, 129, full_time_str, TFT_LIGHTGREY);
  lcd_text(120, 129, "s", TFT_LIGHTGREY);
}
void run_print_temp(float temperature) {
  String str_temp = String(temperature);
  uint8_t text_width = lcd_get_text_width(str_temp);

  // remove old text
  lcd_rect(64, 144, 18+text_width, 10, TFT_BLACK, true);

  lcd_text(65, 145, str_temp, TFT_LIGHTGREY, TFT_BLACK);

  lcd_text(65+2+text_width, 141, "o", TFT_LIGHTGREY);
  lcd_text(65+6+2+text_width, 145, "C", TFT_LIGHTGREY);
}
void run_print_targ_temp(float temperature) {
  String str_temp = String(temperature);
  uint8_t text_width = lcd_get_text_width(str_temp);

  // remove old text
  lcd_rect(64, 112, 18+text_width, 10, TFT_BLACK, true);

  lcd_text(65, 113, str_temp, TFT_LIGHTGREY, TFT_BLACK);

  lcd_text(65+2+text_width, 109, "o", TFT_LIGHTGREY);
  lcd_text(65+6+2+text_width, 113, "C", TFT_LIGHTGREY);
}

void run_print_dc(uint16_t dc) {
  String str_dc = String(dc);
  uint8_t text_width = lcd_get_text_width(str_dc);

  // remove old text
  lcd_rect(64, 99, 18+text_width, 10, TFT_BLACK, true);

  lcd_text(65, 100, str_dc, TFT_LIGHTGREY, TFT_BLACK);
  lcd_text(65+2+text_width, 100, "%", TFT_LIGHTGREY);
}

// button callbacks
void go_to_config_page(void) {
  current_page = config_page;
  current_page->enable();
}

void apply_to_home_page(void) {
  copy_temp_points(tmp_temp_pts, &temperature_points);
  copy_temp_slopes(tmp_temp_slopes, &temperature_slopes);
  copy_temp_duration(tmp_temp_dur, &temperature_durations);

  temp_save_points(temperature_points);
  temp_save_slopes(temperature_slopes);
  temp_save_duration(temperature_durations);

  current_page = home_page;
  current_page->enable();
}

void cancel_to_home_page(void) {
  copy_temp_points(temperature_points, &tmp_temp_pts);
  copy_temp_slopes(temperature_slopes, &tmp_temp_slopes);
  copy_temp_duration(temperature_durations, &tmp_temp_dur);
  
  current_page = home_page;
  current_page->enable();
}

void go_to_home_page(void) {
  current_page = home_page;
  current_page->enable();
}


void go_to_run_page(void) {
  current_page = run_page;
  current_page->enable();
}

void start_stop_reflow(void) {
  if(running) {
    //Stopping...
    running = false;
    enable_heat = false;
    current_dc = 0;
    ssr_set_heat(0);
    ssr_reset_pid();
    lcd_circle(100, 5, 3, TFT_BLACK);// heating indicator

    if(current_page == run_page)
      run_print_time(running_time); // just to show last second
    // run_curve->set_starting_temp(UNSET_TEMPERATURE);
    run_curve->set_starting_temp(start_temp_target);

    run_start_stop_btn->set_text("Start");
    config_btn->set_disabled(false);
  } else {
    //Starting...
    // Setup variables
    start_temperature = (float) start_temp_target;//current_temperature;
    //TODO: confirm it's okay with target start and not current temperature
    //TODO: ensure int to float conversion is okay
    reflow_stop_time = reflow_duration(start_temperature, temperature_points, temperature_slopes, temperature_durations);
    running_time = 0;
    current_target_temp = start_temperature;
    temperatures_per_s[0] = (int16_t) start_temperature;
    run_curve->set_starting_temp(start_temperature);// TODO: (current_temperature); ?

    // update UI
    run_start_stop_btn->set_text("Stop");
    config_btn->set_disabled(true);

    enable_heat = true; // TODO: needed ?

    start_temp_reached = false;
    running = true;
  }

  if(current_page == run_page) {
    current_page->enable();
  }
}

// when reflowing is done
void end_reflow(void) {
  running = false;
  run_start_stop_btn->set_text("Start");
  config_btn->set_disabled(false);

  enable_heat = false;
  current_dc = 0;
  ssr_set_heat(0);
  ssr_reset_pid();

  if(current_page == run_page) {
    current_page->enable();
  } else {
    lcd_circle(120, 5, 4, TFT_BLACK);// running indicator
    lcd_circle(100, 5, 4, TFT_BLACK);// heating indicator
  }
}


