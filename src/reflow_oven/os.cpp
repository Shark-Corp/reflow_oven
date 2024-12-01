#include "os.hpp"

// OS PAGE
// OS_page* page_array[MAX_PAGE_NUMBER];
OS_page* OS_page::current_page;
std::vector<OS_page*> OS_page::pages;

OS_page::OS_page():
is_active(false), selection_pos(0), current_rot_case(OS_ROT_SELECT) {
    OS_page::pages.push_back(this);

    // this->items_array = new OS_item*[MAX_ITEMS_NUMBER];
    // this->items.setStorage(this->items_array, MAX_ITEMS_NUMBER, 0);

    // this->selectable_items_array = new OS_item*[MAX_ITEMS_NUMBER];
    // this->selectable_items.setStorage(this->selectable_items_array, MAX_ITEMS_NUMBER, 0);
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
    for(uint8_t i = 0; i < this->items.size(); i++) {
        if(this->items[i]->is_selectable() && !has_selected) {
            this->items[i]->set_selected(true);
            has_selected = true;
        } else {
            this->items[i]->draw();
        }
    }
    
    rotary_set_pos(0); // set pos to 0 when selecting first item
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

void OS_page::update_selection(bool *clicked) {
    switch (this->current_rot_case) {
        case OS_ROT_SCROLL:
            // TODO: manage scroll
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

    if(*(input->get_nb_pointer()) != (int16_t) rotary_get_pos() ) {
        input->clear();
        input->set_nb( (int16_t) rotary_get_pos() );
        input->draw();
    }
}

void OS_page::manage_rot_selection(void) {
    int32_t curr_pos = rotary_get_pos();

     if(curr_pos < 0) {
        // TODO: maybe go to the last
        curr_pos = 0;
        rotary_set_pos(curr_pos);
    } else if(curr_pos >= this->selectable_items.size()) {
        curr_pos = this->selectable_items.size()-1;
        rotary_set_pos(curr_pos);
    }

    if(curr_pos != this->get_selection_pos()) {
        this->set_selection_pos(curr_pos);
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
    this->selectable_items[this->selection_pos]->set_selected(false);
    this->selection_pos = new_pos;
    this->selectable_items[this->selection_pos]->set_selected(true);
}

void OS_page::set_rot_case(OS_rot_case current_rot_case) {
    this->current_rot_case = current_rot_case;
}







// OS ITEM
OS_item::OS_item(uint32_t x, uint32_t y, bool selectable):
x(x), y(y), selectable(selectable), is_selected(false) {
    std::function<void(void)> func = NULL;
}

bool OS_item::is_selectable(void) {
    return this->selectable;
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





// OS BUTTON 
OS_button::OS_button(uint32_t fg_color, uint32_t bg_color, uint32_t bg_color_selected, uint32_t x, uint32_t y, uint32_t padding_x, uint32_t padding_y, String text, std::function<void(void)> func):
OS_item(x, y, true), fg_color(fg_color), bg_color(bg_color), bg_color_selected(bg_color_selected), size(1), padding_x(padding_x), padding_y(padding_y), text(text) {
    this->set_callback(func);
}

void OS_button::draw(void) {
    uint32_t f_width = lcd_get_text_width(this->text) + 2 * padding_x;
    uint32_t f_height = 8 * this->size + 2 * padding_y;

    if(this->is_selected) {
        lcd_rect(this->x, this->y, f_width, f_height, this->bg_color_selected);
    } else {
        lcd_rect(this->x, this->y, f_width, f_height, this->bg_color);
    }

    lcd_text(this->x+padding_x, this->y+padding_y, this->text, this->fg_color);

    return;
}

uint16_t OS_button::get_width(void) {
    return lcd_get_text_width(this->text) + 2 * padding_x;
}

uint16_t OS_button::get_height(void) {
    return 8 * this->size + 2 * padding_y;
}




//OS integer INPUT
OS_nb_input::OS_nb_input(uint32_t x, uint32_t y, int16_t *nb_pointer):
OS_item(x, y, true), nb_pointer(nb_pointer), is_active(false) {
    this->set_callback(OS_nb_input::save_selection);
}

void OS_nb_input::clear(void) {
    char nb_text[7];
    itoa(*(this->nb_pointer), nb_text, 10);
    uint32_t f_width = lcd_get_text_width(nb_text) + 2*3;
    uint32_t f_height = 8 + 2*3;
    lcd_rect(this->x, this->y, f_width, f_height, TFT_BLACK);
}

void OS_nb_input::draw(void) {
    char nb_text[7];
    itoa(*(this->nb_pointer), nb_text, 10);
    uint32_t f_width = lcd_get_text_width(nb_text) + 2*3;
    uint32_t f_height = 8 + 2*3;

    if(this->is_active) {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_YELLOW);
    } else if(this->is_selected) {
        lcd_rect(this->x, this->y, f_width, f_height, TFT_LIGHTGREY);
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
        rotary_set_pos( *(input_pt->get_nb_pointer()) );
    } else {
        input_pt->set_active(false);
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
void OS_nb_input::set_nb(int16_t value) {
    *(this->nb_pointer) = value;
}

bool OS_nb_input::get_active_state(void) {
    return this->is_active;
}
void OS_nb_input::set_active(bool is_active) {
    this->is_active = is_active;
}



// OS CURVE
OS_curve::OS_curve(uint32_t fg_color, uint32_t txt_color, uint32_t bg_color, uint32_t x, uint32_t y, bool show_grid):
OS_item(x, y, false), height(60), width(120), fg_color(fg_color), txt_color(txt_color), bg_color(bg_color), show_grid(show_grid) {

}

void OS_curve::draw(void) {
    // width 120px (4px y padding)
    // height 60px (0px x padding)
    lcd_rect(this->x, this->y, this->width+8, this->height, this->bg_color);

    // TODO: manage grid :)
    // calculate line color from bg (if dark, ligher; if light, darker)
    uint8_t bg_r = (this->bg_color & 0xF800) >> 11;
    uint8_t bg_g = (this->bg_color & 0x07E0) >> 5;
    uint8_t bg_b = (this->bg_color & 0x001F);
    uint8_t shift = ((bg_r + bg_g + bg_b) / 3 > 20)? -10 : 10;
    uint8_t shift_g = (shift < 0)? -20 : 20; // get something more intense for the green, cause it has more bits
    //TODO: check that it's not < 0 nor > 255
    uint16_t grid_colour  = ((uint16_t)(bg_r + shift) << 11) + ((uint16_t)(bg_g + shift_g) << 5) + (bg_b + shift);
    
    lcd_dashed_h_line(13, this->y+this->height, 110, 3, 3, grid_colour);
    lcd_dashed_h_line(13, this->y+(this->height/2), 110, 3, 3, grid_colour);
    lcd_dashed_h_line(13, this->y, 110, 3, 3, grid_colour);

    lcd_line(15, this->y+this->height, 35, this->y+(this->height/2), this->fg_color); // 1st rising slope
    lcd_line(35, this->y+(this->height/2), 70, this->y+(this->height/2), this->fg_color); // 1st flat
    lcd_line(70, this->y+(this->height/2), 90, this->y, this->fg_color); // 2nd rising slope
    lcd_line(90, this->y, 105, this->y, this->fg_color); // 2nd flat
    lcd_line(105, this->y, 123, this->y+this->height, this->fg_color); // last falling slope

    lcd_text(5, this->y+this->height-4, "0", this->txt_color);
    lcd_text(5, this->y+(this->height/2)-4, "1", this->txt_color);
    lcd_text(5, this->y-2, "2", this->txt_color);

    lcd_text(18, this->y+this->height-20, "A", this->txt_color);
    lcd_text(70, this->y+10, "B", this->txt_color);
    lcd_text(113, this->y+(this->height/2)-16, "C", this->txt_color);
}
// void OS_curve::set_show_grid(bool show_grid);

// void OS_curve::set_temp_points(float a, float b, float c, float d) {
//     this->temp_points[0] = a;
//     this->temp_points[1] = b;
//     this->temp_points[2] = c;
//     this->temp_points[3] = d;
// }
// void OS_curve::set_temp_point(uint8_t id, float value) {
//     if(id >= 0 and id <= 3) {
//         this->temp_points[id] = value;
//     }
// }
// void OS_curve::set_slopes(float a, float b, float c, float d, float e) {
//     this->slopes[0] = a;
//     this->slopes[1] = b;
//     this->slopes[2] = c;
//     this->slopes[3] = d;
//     this->slopes[4] = e;
// }
// void OS_curve::set_slope(uint8_t id, float value) {
//     if(id >= 0 and id <= 3) {
//         this->temp_points[id] = value;
//     }
// }




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

