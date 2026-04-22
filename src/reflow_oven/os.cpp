#include "os.hpp"

// OS PAGE
OS_page* OS_page::current_page;
std::vector<OS_page*> OS_page::pages;

OS_page::OS_page():
is_active(false), selection_pos(0), current_rot_case(OS_ROT_SELECT) {
    OS_page::pages.push_back(this);
}

void OS_page::enable(void) {
    for (uint8_t i = 0; i < OS_page::pages.size(); i++) {
        if(OS_page::pages[i]->is_active) {
            OS_page::pages[i]->disable();
        }
    }
    this->is_active = true;
    OS_page::current_page = this;

    lcd_clear(); // clear screen before displaying items

    bool has_selected = false;
    uint8_t sel_item = 0;
    for(uint8_t i = 0; i < this->items.size(); i++) {
        if(this->items[i]->is_selectable() && !has_selected) {
            this->items[i]->set_selected(true);
            sel_item = i;
            has_selected = true;
        } else {
            // this->items[i]->set_selected(false);
            this->items[i]->draw();
        }
    }
    
    rotary_set_pos(sel_item); // set pos to selected item
}
void OS_page::disable(void) {
    this->is_active = false;
}

void OS_page::add_item(OS_item *item) {
    // TODO: check if item not already in here
    this->items.push_back(item);
    if(item->is_selectable()) {
        this->selectable_items.push_back(item);
    }
}

void OS_page::set_disable_item(OS_item *item, bool disable) {
    item->set_disabled(disable);

    auto it = std::find(this->selectable_items.begin(), this->selectable_items.end(), item);

    if(it != this->selectable_items.end() && disable) {
        this->selectable_items.erase(it);
    } else if(it == this->selectable_items.end() && !disable) {
        this->selectable_items.push_back(item);
    }

    //TODO: problem is that it is placed back, it doesn't keep the initial placement order

    return;
}


void OS_page::update_selection(bool *clicked) {
    switch (this->current_rot_case) {
        case OS_ROT_SCROLL:
            // TODO: manage scroll ?
            break;
        case OS_ROT_NUMBER:
            this->manage_rot_number();
            break;
        case OS_ROT_SELECT:
        default:
            this->manage_rot_selection();
            break;
    }

    if(*clicked) {
        OS_item *selected_item = this->selectable_items[this->get_selection_pos()];
        if(selected_item->get_callback() != NULL) {
            selected_item->run_callback();
        }
        *clicked = false;
    }
}

void OS_page::manage_rot_number(void) {
    int32_t curr_pos = rotary_get_pos();
    OS_nb_input* input = ((OS_nb_input*) this->get_selected_item());

    if(input->get_is_float()) {
        if(*(input->get_nb_pointer_f()) != (float) rotary_get_pos()/10.0f ) {
            input->clear();
            input->set_nb( (float) rotary_get_pos()/10.0f );
            input->draw();
        }
    } else {
        if(*(input->get_nb_pointer()) != (int16_t) rotary_get_pos() ) {
            input->clear();
            input->set_nb( (int16_t) rotary_get_pos() );
            input->draw();
        }
    }
}

void OS_page::manage_rot_selection(void) {
    int32_t curr_pos = rotary_get_pos();

    if(curr_pos < 0) {
        // TODO: maybe go to the last item
        curr_pos = 0;
        rotary_set_pos(curr_pos);
    } else if(curr_pos >= this->selectable_items.size()) {
        curr_pos = this->selectable_items.size()-1;
        rotary_set_pos(curr_pos);
    }

    if(curr_pos > this->get_selection_pos()) { // going upper
        if(this->selectable_items[curr_pos]->is_selectable()) {
            this->set_selection_pos(curr_pos); // selectable, select it
        } else { // not selectable, try to find next
            uint8_t selectable_pos = curr_pos-1; // get previous position
            for(int8_t i = curr_pos+1; i < this->selectable_items.size(); i++){
                if(this->selectable_items[i]->is_selectable()) {
                    selectable_pos = i;
                    break;
                }    
            }
            this->set_selection_pos(selectable_pos);
            rotary_set_pos(this->selection_pos);
        }
    } else if(curr_pos < this->get_selection_pos()) { // going lower
        if(this->selectable_items[curr_pos]->is_selectable()) {
            this->set_selection_pos(curr_pos); // selectable, select it
        } else { // not selectable, try to find previous
            uint8_t selectable_pos = curr_pos+1; // get previous position
            for(int8_t i = curr_pos-1; i > 0; i--){
                if(this->selectable_items[i]->is_selectable()) {
                    selectable_pos = i;
                    break;
                }    
            }
            this->set_selection_pos(selectable_pos);
            rotary_set_pos(this->selection_pos);
        }
    }    
}

OS_page* OS_page::get_current_page(void) {
    return OS_page::current_page;
}

OS_item* OS_page::get_selected_item(void) {
    return this->selectable_items[selection_pos];
}

int32_t OS_page::get_selection_pos(void) {
    return this->selection_pos;
}
void OS_page::set_selection_pos(int32_t new_pos) {
    if(this->selection_pos != new_pos) {
        this->selectable_items[this->selection_pos]->set_selected(false);
        this->selection_pos = new_pos;
        this->selectable_items[this->selection_pos]->set_selected(true);
    }
}

void OS_page::set_rot_case(OS_rot_case current_rot_case) {
    this->current_rot_case = current_rot_case;
}







// OS ITEM
OS_item::OS_item(uint32_t x, uint32_t y, bool selectable):
x(x), y(y), selectable(selectable), is_selected(false), disabled(false) {
    std::function<void(void)> func = NULL;
}

bool OS_item::is_selectable(void) {
    return (this->selectable && !this->disabled);
}
void OS_item::set_selected(bool is_selected) {
    this->is_selected = is_selected;
    this->draw();
}

void OS_item::set_callback(std::function<void(void)> func) {
    this->callback_function = func;
}
std::function<void(void)> OS_item::get_callback(void) {
    return this->callback_function;
}

void OS_item::run_callback(void) {
    this->callback_function();
}

void OS_item::set_disabled(bool disable) {
    this->disabled = disable;
}

uint32_t OS_item::get_x(void) {
    return this->x;
}

uint32_t OS_item::get_y(void) {
    return this->y;
}






// OS BUTTON 
OS_button::OS_button(uint32_t fg_color, uint32_t bg_color, uint32_t bg_color_selected, uint32_t x, uint32_t y, uint32_t padding_x, uint32_t padding_y, String text, std::function<void(void)> func):
OS_item(x, y, true), fg_color(fg_color), bg_color(bg_color), bg_color_selected(bg_color_selected), size(1), padding_x(padding_x), padding_y(padding_y), text(text) {
    this->set_callback(func);
}

void OS_button::draw(void) {
    uint32_t f_width = lcd_get_text_width(this->text) + 2 * padding_x;
    uint32_t f_height = 8 * this->size + 2 * padding_y;

    if(this->disabled) {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_LIGHTGREY);
        lcd_text(this->x+padding_x, this->y+padding_y, this->text, TFT_BLACK);
    } else {
        if(this->is_selected) {
            lcd_rect(this->x, this->y, f_width, f_height, this->bg_color);
            lcd_rect(this->x, this->y, f_width, f_height, this->bg_color_selected, false); // border
            lcd_rect(this->x+1, this->y+1, f_width-2, f_height-2, this->bg_color_selected, false); // border
        } else {
            lcd_rect(this->x, this->y, f_width, f_height, this->bg_color);
        }
        lcd_text(this->x+padding_x, this->y+padding_y, this->text, this->fg_color);
    }

    return;
}

void OS_button::clear(void) {
    uint32_t f_width = lcd_get_text_width(this->text) + 2 * padding_x;
    uint32_t f_height = 8 * this->size + 2 * padding_y;

    lcd_rect(this->x, this->y, f_width, f_height, TFT_BLACK, true);
}

uint16_t OS_button::get_width(void) {
    return lcd_get_text_width(this->text) + 2 * padding_x;
}

uint16_t OS_button::get_height(void) {
    return 8 * this->size + 2 * padding_y;
}

void OS_button::set_text(String new_text) {
    this->text = new_text;
}

void OS_button::update_text(String new_text) {
    this->clear();
    this->text = new_text;
    this->draw();
}




//OS integer INPUT
OS_nb_input::OS_nb_input(uint32_t x, uint32_t y, int16_t *nb_pointer):
OS_item(x, y, true), nb_pointer(nb_pointer), nb_pointer_f(NULL), is_active(false), is_float(false) {
    this->set_callback(OS_nb_input::save_selection);
}
OS_nb_input::OS_nb_input(uint32_t x, uint32_t y, float *nb_pointer):
OS_item(x, y, true), nb_pointer(NULL), nb_pointer_f(nb_pointer), is_active(false), is_float(true) {
    this->set_callback(OS_nb_input::save_selection);
}

void OS_nb_input::clear(void) {
    char nb_text[NB_INPUT_BUFF_SIZE];
    if(this->nb_pointer == NULL)
        snprintf(nb_text, NB_INPUT_BUFF_SIZE, "%.1f", this->nb_pointer_f);
    else
        snprintf(nb_text, NB_INPUT_BUFF_SIZE, "%d", this->nb_pointer);

    uint32_t f_width = lcd_get_text_width(nb_text) + 2*3;
    uint32_t f_height = 8 + 2*3;
    lcd_rect(this->x, this->y, f_width, f_height, TFT_BLACK);
}

void OS_nb_input::draw(void) {
    char nb_text[NB_INPUT_BUFF_SIZE];
    if(this->nb_pointer == NULL)
        snprintf(nb_text, NB_INPUT_BUFF_SIZE, "%.1f", *this->nb_pointer_f);
    else
        snprintf(nb_text, NB_INPUT_BUFF_SIZE, "%d", *this->nb_pointer);

    uint32_t f_width = lcd_get_text_width(nb_text) + 2*3;
    uint32_t f_height = 8 + 2*3;

    if(this->is_active) {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_YELLOW);
    } else if(this->is_selected) {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_LIGHTGREY);
        lcd_rect(this->x, this->y, f_width, f_height, TFT_RED, false); //border
    } else {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_WHITE);
    }

    lcd_text(this->x+3, this->y+3, nb_text, TFT_BLACK);

    return;
}

void OS_nb_input::save_selection(void) {
    OS_nb_input* input_pt = (OS_nb_input*) OS_page::get_current_page()->get_selected_item();

    if( ! input_pt->get_active_state() ) {
        input_pt->set_active(true);
        rotary_push_pos(); // saving select position
        OS_page::get_current_page()->set_rot_case(OS_ROT_NUMBER); // changing rot encoder mode
        
        //TODO: useless ?
        if(input_pt->nb_pointer == NULL)
            rotary_set_pos( (uint32_t) (*(input_pt->get_nb_pointer_f()) * 10.0f) );
        else
            rotary_set_pos( *(input_pt->get_nb_pointer()) );
    } else {
        input_pt->set_active(false);

        if(input_pt->nb_pointer == NULL)
            input_pt->set_nb( (float) rotary_get_pos()/10.0f ); // TODO: useless ?
        else
            input_pt->set_nb( (int16_t) rotary_get_pos() ); // TODO: useless ?

        //TODO: save in EEPROM 
        rotary_pull_pos(); // getting back select position
        OS_page::get_current_page()->set_rot_case(OS_ROT_SELECT);  // changing rot encoder mode
    }

    input_pt->draw();

    return;
}

int16_t* OS_nb_input::get_nb_pointer(void) {
    return this->nb_pointer;
}
float* OS_nb_input::get_nb_pointer_f(void) {
    return this->nb_pointer_f;
}
void OS_nb_input::set_nb(int16_t value) {
    *(this->nb_pointer) = value;
}
void OS_nb_input::set_nb(float value) {
    *(this->nb_pointer_f) = value;
}

bool OS_nb_input::get_active_state(void) {
    return this->is_active;
}
void OS_nb_input::set_active(bool is_active) {
    this->is_active = is_active;
}
bool OS_nb_input::get_is_float(void) {
    return this->is_float;
}




// OS CURVE
OS_curve::OS_curve(uint32_t fg_color, uint32_t txt_color, uint32_t bg_color, uint32_t x, uint32_t y, bool show_grid, bool use_params):
OS_item(x, y, false), height(60), width(120), fg_color(fg_color), txt_color(txt_color), bg_color(bg_color), show_grid(show_grid), use_params(use_params) {
    points = NULL;
    slopes = NULL;
    durations = NULL;
    this->start_temperature = UNSET_TEMPERATURE;
}

void OS_curve::draw(void) {
    bool take_params = (this->use_params && this->points != NULL && this->slopes != NULL && this->durations != NULL)? true : false;

    // width 120px (4px y padding)
    // height 60px (0px x padding)
    lcd_rect(this->x, this->y, this->width+8, this->height, this->bg_color); //clear

    // calculate line color from bg (if dark, ligher; if light, darker)
    uint8_t bg_r = (this->bg_color & 0xF800) >> 11;
    uint8_t bg_g = (this->bg_color & 0x07E0) >> 5;
    uint8_t bg_b = (this->bg_color & 0x001F);
    uint8_t shift = ((bg_r + bg_g + bg_b) / 3 > 20)? -10 : 10;
    uint8_t shift_g = (shift < 0)? -20 : 20; // get something more intense for the green, cause it has more bits
    //TODO: check that it's not < 0 nor > 255
    uint16_t grid_colour  = ((uint16_t)(bg_r + shift) << 11) + ((uint16_t)(bg_g + shift_g) << 5) + (bg_b + shift);

    uint8_t xstart = 12;
    uint8_t x_end_padding = 3;
    uint8_t sec_per_pixel = 1, deg_per_pixel = 1;
    float current_temp;
    if(this->start_temperature == UNSET_TEMPERATURE) {
        current_temp = 70;// TODO: verify right usage, change to config maybe, thermo_read_temp_c();
    } else {
        current_temp = (float) this->start_temperature;
    }

    uint16_t delta_temp = CURVE_MAX_TEMP - current_temp; //TODO: check if there is a better way to have max temp (take in account overshoot)

    if(take_params) {
        uint16_t ref_dur = reflow_duration(current_temp, *this->points, *this->slopes, *this->durations);
        sec_per_pixel = ref_dur / (this->width+8-xstart-x_end_padding); //TODO: this-> width is not the real curve full width ? (change also live curve)
        if(sec_per_pixel * this->width < ref_dur)
            sec_per_pixel++;
        
        deg_per_pixel = delta_temp/this->height;
        if(deg_per_pixel * this->height < delta_temp)
            deg_per_pixel++;
    }

    uint8_t topline_shift = (take_params)? this->height-((this->points->tp2-current_temp)/deg_per_pixel) : 0; // move midline to proportional position
    uint8_t midline_shift = (take_params)? this->height-((this->points->tp1-current_temp)/deg_per_pixel) : this->height/2; // move midline to proportional position
    lcd_dashed_h_line(13, this->y+this->height, 110, 3, 3, grid_colour); // bottom line
    lcd_dashed_h_line(13, this->y+midline_shift, 110, 3, 3, grid_colour);
    lcd_dashed_h_line(13, this->y+topline_shift, 110, 3, 3, grid_colour);
    lcd_dashed_h_line(13, this->y, 110, 3, 3, grid_colour); // 250°C line

    uint8_t s1_xend = xstart + ((take_params)? ((float)(this->points->tp1 - current_temp)/this->slopes->tsA)/sec_per_pixel : 20);
    uint8_t dA_xend = s1_xend + ((take_params)? (this->durations->tdA/sec_per_pixel) : 35);
    uint8_t s2_xend = dA_xend + ((take_params)? ((float)(this->points->tp2 - this->points->tp1)/this->slopes->tsB)/sec_per_pixel : 20);
    uint8_t dB_xend = s2_xend + ((take_params)? (this->durations->tdB/sec_per_pixel) : 15);
    if(take_params) {
        lcd_dashed_line(xstart,  this->y+this->height, s1_xend, this->y+midline_shift, this->fg_color);// 1st rising slope
        lcd_dashed_line(s1_xend, this->y+midline_shift, dA_xend, this->y+midline_shift, this->fg_color); // 1st flat
        lcd_dashed_line(dA_xend, this->y+midline_shift, s2_xend, this->y+topline_shift, this->fg_color); // 2nd rising slope
        lcd_dashed_line(s2_xend, this->y+topline_shift, dB_xend, this->y+topline_shift, this->fg_color); // 2nd flat
        lcd_dashed_line(dB_xend, this->y+topline_shift, 128-x_end_padding, this->y+this->height, this->fg_color); // last falling slope
    } else {
        lcd_line(xstart,  this->y+this->height, s1_xend, this->y+midline_shift, this->fg_color); // 1st rising slope
        lcd_line(s1_xend, this->y+midline_shift, dA_xend, this->y+midline_shift, this->fg_color); // 1st flat
        lcd_line(dA_xend, this->y+midline_shift, s2_xend, this->y+topline_shift, this->fg_color); // 2nd rising slope
        lcd_line(s2_xend, this->y+topline_shift, dB_xend, this->y, this->fg_color); // 2nd flat
        lcd_line(dB_xend, this->y+topline_shift, 128-x_end_padding, this->y+this->height, this->fg_color); // last falling slope
    }
    

    String pt0_txt, pt1_txt, pt2_txt;
    uint8_t pt_space;
    String slA_txt, slB_txt, slC_txt;
    String da_txt, db_txt;
    uint8_t da_xshift, db_xshift;
    if(take_params) {
        pt0_txt = (this->start_temperature == UNSET_TEMPERATURE)? "ST" : String(this->start_temperature);
        pt1_txt = String(this->points->tp1);
        pt2_txt = String(this->points->tp2);
        pt_space = 1;
        slA_txt = String(this->slopes->tsA, 1);
        slB_txt = String(this->slopes->tsB, 1);
        slC_txt = String(this->slopes->tsC, 1);
        da_txt = String(this->durations->tdA);
        db_txt = String(this->durations->tdB);
        da_xshift = 10;
        db_xshift = 4;
    } else {
        pt0_txt = "0";
        pt1_txt = "1";
        pt2_txt = "2";
        pt_space = 5;
        slA_txt = "A";
        slB_txt = "B";
        slC_txt = "C";
        da_txt = "a";
        db_txt = "b";
        da_xshift = 0;
        db_xshift = 0;
    }

    // Left Axis temperatures
    lcd_text(pt_space, this->y+this->height-4, pt0_txt, this->txt_color); // tp1
    lcd_text(pt_space, this->y+midline_shift-4, pt1_txt, this->txt_color); // tp2
    lcd_text(pt_space, this->y+topline_shift-2, pt2_txt, this->txt_color); // tp3

    // Slopes
    lcd_text(xstart, this->y+this->height-20, slA_txt, this->txt_color); // sA
    lcd_text(dA_xend-5, this->y+10, slB_txt, this->txt_color); // sB
    lcd_text(dB_xend+13, this->y+(this->height/2)-16, slC_txt, this->txt_color); // sC

    // Durations
    lcd_text(s1_xend+5, this->y+midline_shift+3, da_txt, this->txt_color); // dA
    lcd_text(s2_xend+3, this->y+topline_shift+3, db_txt, this->txt_color); // dB
}

void OS_curve::set_params(temperature_points_t *points, temperature_slopes_t *slopes, temperature_durations_t *durations) {
    this->points = points;
    this->slopes = slopes;
    this->durations = durations;
}

uint32_t OS_curve::get_h(void) {
    return this->height;
}

uint32_t OS_curve::get_w(void) {
    return this->width;
}

void OS_curve::set_starting_temp(int16_t start_temp) {
    this->start_temperature = start_temp;
}
int16_t OS_curve::get_starting_temp(void) {
    return this->start_temperature;
}

temperature_points_t OS_curve::get_points(void) {
    return *this->points;
}
temperature_slopes_t OS_curve::get_slopes(void) {
    return *this->slopes;
}
temperature_durations_t OS_curve::get_durations(void) {
    return *this->durations;
}




// OS live curve
OS_live_curve::OS_live_curve(uint32_t fg_color, OS_curve *source_curve, int16_t *temperatures, uint16_t *running_time):
OS_item(source_curve->get_x(), source_curve->get_y(), false), height(source_curve->get_h()), width(source_curve->get_w()), source_curve(source_curve), fg_color(fg_color), temperatures(temperatures), running_time(running_time), last_time_drawn(0) {

}

void OS_live_curve::draw(void) {
    // uint8_t xstart = 12; // left margin
    // uint8_t x_end_padding = 3;

    if(this->source_curve->get_starting_temp() == UNSET_TEMPERATURE)
        // this->start_temp = thermo_read_temp_c();
        this->start_temp = 70;
    else
        this->start_temp = (float) this->source_curve->get_starting_temp();

    this->ref_dur = reflow_duration(this->start_temp, this->source_curve->get_points(), this->source_curve->get_slopes(), this->source_curve->get_durations());
    this->sec_per_pixel = (float) this->ref_dur / (this->width+8-12-3);
    //this->delta_temp = this->source_curve->get_points().tp2 - this->start_temp; // lowest to highest temperature delta
    this->delta_temp = CURVE_MAX_TEMP - this->start_temp; // lowest to highest temperature delta
    this->deg_per_pixel = (float) this->delta_temp / this->height;

    this->y_start_temp_delta = this->start_temp / this->deg_per_pixel;

    this->draw_curve(0, *this->running_time);
    
    this->last_time_drawn = *this->running_time;
}


void OS_live_curve::update(void) {
    this->draw_curve(this->last_time_drawn, *this->running_time);

    this->last_time_drawn = *this->running_time;
}

void OS_live_curve::draw_curve(uint16_t start_pt, uint16_t end_pt) {
    // uint8_t xstart = 12; // left margin
    // uint8_t x_end_padding = 3;

    for(uint16_t i = start_pt; i < end_pt; i += this->sec_per_pixel) {
        float pixel_x = i / this->sec_per_pixel;
        float pixel_y = this->temperatures[i] / this->deg_per_pixel;
        lcd_pixel(
            this->x + 12 + (uint8_t)pixel_x,
            this->y + this->y_start_temp_delta + this->height - (uint8_t)pixel_y,
            this->fg_color
        );
    }

}



// OS SEPARATOR
OS_separator::OS_separator(uint32_t colour, uint32_t y):
OS_item(3, y, false), colour(colour) {
}
void OS_separator::draw(void) {
    lcd_line(this->x, this->y, 127-this->x, this->y, this->colour);
}




// OS CALLBACK
OS_callback::OS_callback(std::function<void(void)> func):
OS_item(0, 0, false) {
    this->callback_function = func;
}

void OS_callback::draw(void) {
    this->callback_function();
}

