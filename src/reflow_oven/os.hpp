#ifndef _OS_H_
#define _OS_H_

#include "util.hpp"
#include "lcd_helper.hpp"
#include "rotary_helper.hpp"
#include "temp_sensor.hpp"
#include <Arduino.h>
#include <vector>


#define MAX_PAGE_NUMBER     5
#define MAX_ITEMS_NUMBER    10

#define UNSET_TEMPERATURE   -500
#define CURVE_MAX_TEMP      250

#define NB_INPUT_BUFF_SIZE  10

typedef enum OS_rot_case {
    OS_ROT_SELECT, OS_ROT_NUMBER, OS_ROT_SCROLL
};




class OS_item {
    protected:
        uint32_t x;
        uint32_t y;
        bool selectable;
        bool is_selected;
        bool disabled;
        std::function<void(void)> callback_function;

    public:
        OS_item(uint32_t x, uint32_t y, bool selectable);
        bool is_selectable(void);
        virtual void draw(void) = 0;
        void set_selected(bool is_selected);
        void set_callback(std::function<void(void)> func);
        std::function<void(void)> get_callback(void);
        void run_callback(void);
        void set_disabled(bool disable);

        uint32_t get_x(void);
        uint32_t get_y(void);
};



class OS_button : public OS_item {
    private:
        uint32_t fg_color;
        uint32_t bg_color;
        uint32_t bg_color_selected;
        uint32_t padding_x;
        uint32_t padding_y;
        uint8_t size;
        String text;
    
    public:
        OS_button(uint32_t fg_color, uint32_t bg_color, uint32_t bg_color_selected, uint32_t x, uint32_t y, uint32_t padding_x, uint32_t padding_y, String text, std::function<void(void)> callback_function);
        void draw(void);
        void clear(void);
        uint16_t get_width(void);
        uint16_t get_height(void);
        void set_text(String new_text);
        void update_text(String new_text);
};


class OS_nb_input : public OS_item {
    private:
        int16_t  *nb_pointer;
        float    *nb_pointer_f;
        bool is_active;
        bool is_float;
    
    public:
        OS_nb_input(uint32_t x, uint32_t y, int16_t *nb_pointer);
        OS_nb_input(uint32_t x, uint32_t y, float *nb_pointer);
        void clear(void);
        void draw(void);
        
        static void save_selection(void); // click callback

        int16_t* get_nb_pointer(void);
        float* get_nb_pointer_f(void);
        void set_nb(int16_t value);
        void set_nb(float value);

        bool get_active_state(void);
        void set_active(bool is_active);
        bool get_is_float(void);

};



class OS_curve : public OS_item {
    private:
        uint32_t fg_color;
        uint32_t txt_color;
        uint32_t bg_color;
        bool show_grid;
        uint32_t width; // full width
        uint32_t height; // fixed height
        bool use_params;
        int16_t start_temperature;

        temperature_points_t *points;
        temperature_slopes_t *slopes;
        temperature_durations_t *durations;
    
    public:
        OS_curve(uint32_t fg_color, uint32_t txt_color, uint32_t bg_color, uint32_t x, uint32_t y, bool show_grid, bool use_params = false);
        
        void draw(void);
        void set_params(temperature_points_t *points, temperature_slopes_t *slopes, temperature_durations_t *durations);

        uint32_t get_h(void);
        uint32_t get_w(void);

        void set_starting_temp(int16_t start_temp);
        int16_t get_starting_temp(void);

        temperature_points_t get_points(void);
        temperature_slopes_t get_slopes(void);
        temperature_durations_t get_durations(void);


};

class OS_live_curve : public OS_item {
    private:
        uint32_t fg_color;
        uint32_t width; // full width
        uint32_t height; // fixed height

        OS_curve *source_curve;

        int16_t *temperatures;
        uint16_t *running_time;
        uint16_t last_time_drawn;

        float start_temp;
        uint16_t ref_dur, delta_temp;
        float sec_per_pixel, deg_per_pixel;        
        uint8_t y_start_temp_delta;

        void draw_curve(uint16_t start_pt, uint16_t end_pt);

    public:
        OS_live_curve(uint32_t fg_color, OS_curve *source_curve, int16_t *temperatures, uint16_t *running_time);
        
        void draw(void);
        void update(void);
};



class OS_separator : public OS_item {
    private:
        uint32_t colour;
        // uint8_t thickness;

    public:
        OS_separator(uint32_t colour, uint32_t y);
        void draw(void);
};



class OS_callback : public OS_item {
    private:
        std::function<void(void)> callback_function;

    public:
        OS_callback(std::function<void(void)> func);
        void draw(void);
};



class OS_page {
    private:
        static std::vector<OS_page*> pages;
        static OS_page *current_page;

        // OS_item **items_array;
        std::vector<OS_item *> items;
        // OS_item **selectable_items_array;
        std::vector<OS_item *> selectable_items;
        int32_t selection_pos;
        bool is_active;
        OS_rot_case current_rot_case;

        void manage_rot_number(void);
        void manage_rot_selection(void);

    public:
        OS_page();

        void enable(void);
        void disable(void);

        void add_item(OS_item *item);
        void set_disable_item(OS_item *item, bool disable);

        void update_selection(bool *clicked);

        static OS_page* get_current_page(void);
        
        OS_item* get_selected_item(void);

        int32_t get_selection_pos(void);
        void set_selection_pos(int32_t new_pos);

        void set_rot_case(OS_rot_case current_rot_case);
};

#endif