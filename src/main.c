#include <pebble.h>
#include "main.h"
#include "spocks.h"

Window *my_window;
static Layer *bg_layer;
static Layer *spock_layer;

// Time variables
static int min10;
static int min1;
static int hour10;
static int hour1;
static int sec;
static int d10;
static int d1;
static int m10;
static int m1;
int date_format;
//static *char dw;
//static *char m;

// Watch status
static bool is_bt_connected;
static int batt_level;

// Configuration options
static bool vibrate_on_hour = false;
static bool vibrate_on_bt_disc = false;
static bool vibrate_on_bt_conn = false;

static bool is_bt_shown = false;
static bool is_batt_shown = false;
static bool is_sec_shown = false;

static bool bt_show_on_shake = false;
static bool batt_show_on_shake = false;
static bool sec_show_on_shake = false;
static bool date_show_on_shake = false;

static bool is_shake_x_active = false;
static bool is_shake_y_active = false;
static AppTimer *shake_x_timer;
static int shake_bbs_timer_delay = 30000;

static AppTimer *shake_y_timer;
static int shake_date_timer_delay = 30000;

bool show_zero_for_hours = false;

static int animation_rate = ANIMATE_EVERY_1M;
// This adds a certain number of shapes to show every once in a while or on loading the watchface.
static const int MAX_ANIMATIONS = 1;
static int current_middle_animation = 1;
// which shape is used next
int middle_shape = 0;

int hand_length = NORMAL_LENGTH_HAND_SPOCK;

// Colors. The pattern can be BACKGROUND_EQUAL, BACKGROUND_DIFF_SIDE_BAR, BACKGROUND_HOUR_MIN and BACKGROUND_ALL_DIFF
static int color_pattern = BACKGROUND_HOUR_MIN;
static GColor color_background_h10;
static GColor color_background_h1;
static GColor color_background_m10;
static GColor color_background_m1;
static GColor color_background_sec;
static GColor color_background_bt;
GColor color_spocks_h10;
GColor color_spocks_h1;
GColor color_spocks_m10;
GColor color_spocks_m1;
GColor color_spocks_sec;
GColor color_spocks_bt;

static AppTimer *animation_timer; //for the animation
const int timer_delay = 90;

/**
 * Returns if the time change requires a redraw of the screen.
 * Necessary as the screen won't change per second, but each 5 seconds if this data is shown.
 * 
**/
static bool update_time() {
  LOG("Updating time");
  time_t now = time(NULL);
  struct tm *time = localtime(&now);
  int hour = time->tm_hour;
  int new_min1 = -1;
  int new_sec = -1;
  bool res = false;
  
  d10 = (time->tm_mday + 1) / 10;
  d1 = (time->tm_mday + 1) % 10;
  m10 = (time->tm_mon + 1) / 10;
  m1 = (time->tm_mon + 1) % 10;

  if(!clock_is_24h_style() && hour > 12) {
    hour = hour - 12;
  }
  hour10 = hour / 10;
  hour1 = hour % 10;
  
  min10 = time->tm_min / 10;
  new_min1 = time->tm_min % 10;
  if(min1 != new_min1) {
    min1 = new_min1;
    res = true;
  }
  
  if(is_sec_shown || (is_shake_y_active && sec_show_on_shake)) {
    new_sec = time->tm_sec / 5;
    if (sec != new_sec) {
      sec = new_sec;
      res = true;
    }
  }
  
  if(time->tm_min == 0 && time->tm_sec == 0 && vibrate_on_hour) {
    vibes_short_pulse();
  }
  
  // Animate with special patterns every once in a while
  if ((animation_rate == ANIMATE_EVERY_1M && time->tm_sec == 0) ||
      (animation_rate == ANIMATE_EVERY_10M && time->tm_min % 10 == 0 && time->tm_sec == 0) ||
      (animation_rate == ANIMATE_EVERY_15M && time->tm_min % 15 == 0 && time->tm_sec == 0) ||
      (animation_rate == ANIMATE_EVERY_30M && time->tm_min % 30 == 0 && time->tm_sec == 0) ||
      (animation_rate == ANIMATE_EVERY_1H && time->tm_min == 0 && time->tm_sec == 0)
     ) {
    middle_shape = rand() % NUM_SHAPES;
    current_middle_animation = MAX_ANIMATIONS;
  }
  
  return res;
}

static void tick_handler(struct tm *time, TimeUnits units_changed) {
  LOG("tick-tock");
  if (update_time())
    layer_mark_dirty(window_get_root_layer(my_window));
}

static void bt_handler(bool connected) {
  LOG("Bluetooth connection changed: %d", connected);
  is_bt_connected = connected;
  if(is_bt_shown) {
    layer_mark_dirty(window_get_root_layer(my_window));
  }
  if(connected == true && vibrate_on_bt_conn) {
    vibes_short_pulse();
  } else if (connected == false && vibrate_on_bt_disc) {
    vibes_double_pulse();
  }
}

static void batt_handler(BatteryChargeState charge) {
  LOG("Battery state changed: %d%%", charge.charge_percent);
  batt_level = charge.charge_percent / 10;
  if(is_batt_shown) {
    layer_mark_dirty(window_get_root_layer(my_window));
  }
}

static void end_shake_x_timer(void *data) {
  shake_x_timer = NULL;
  is_shake_x_active = false;
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void end_shake_y_timer(void *data) {
  shake_y_timer = NULL;
  is_shake_y_active = false;
  if(sec_show_on_shake && !is_sec_shown) {
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  }
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void shake_handler(AccelAxisType axis, int32_t direction) {
  switch(axis) {
  case ACCEL_AXIS_X:
    if(direction > 0) {
      INFO("X>0");
    } else {
      INFO("X<0");
    }
    //If it was active, deactivate
    if(is_shake_x_active) {
      end_shake_x_timer(NULL);
    } else {
      is_shake_x_active = true;
      if(date_show_on_shake) {
        middle_shape = rand() % NUM_SHAPES;
        current_middle_animation = MAX_ANIMATIONS;
      }
      layer_mark_dirty(window_get_root_layer(my_window));
      shake_x_timer = app_timer_register(shake_date_timer_delay, end_shake_x_timer, NULL);
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      INFO("Y>0");
    } else {
      INFO("Y<0");
    }
    //If it was active, deactivate
    if(is_shake_y_active) {
      end_shake_y_timer(NULL);
    } else {
      is_shake_y_active = true;
      if(batt_show_on_shake) {
        batt_level = battery_state_service_peek().charge_percent / 10;
      }
      if(bt_show_on_shake) {
        is_bt_connected = bluetooth_connection_service_peek();
      }
      if(sec_show_on_shake) {
        tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
      }
      layer_mark_dirty(window_get_root_layer(my_window));
      shake_y_timer = app_timer_register(shake_bbs_timer_delay, end_shake_y_timer, NULL);
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      INFO("Z>0");
    } else {
      INFO("Z<0");
    }
    break;
  }
}

static GColor GColorFromInt(int value) {
#ifdef PBL_COLOR
  return GColorFromHEX(value);
#else 
  if(value >= 0 && value < 16777215/2) {
    return GColorBlack;
  } else {
    return GColorWhite;
  }
#endif
}

static void set_right_spock_colors() {
  switch(color_pattern) {
    case BACKGROUND_EQUAL:
      color_spocks_h1 = color_spocks_h10;
      color_spocks_m10 = color_spocks_h10;
      color_spocks_m1 = color_spocks_h10;
      color_spocks_sec = color_spocks_h10;
      color_spocks_bt = color_spocks_h10;
      break;
    case BACKGROUND_DIFF_SIDE_BAR:
      color_spocks_h1 = color_spocks_h10;
      color_spocks_m10 = color_spocks_h10;
      color_spocks_m1 = color_spocks_h10;
      break;
    case BACKGROUND_HOUR_MIN:
      color_spocks_h1 = color_spocks_h10;
      color_spocks_m1 = color_spocks_m10;
      break;
    case BACKGROUND_ALL_DIFF:
      break;
  }
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  LOG("JS message received");
  Tuple *t = dict_read_first(iter);
  
  int new_val = -1;
  int old_color = -1;
  
  while(t) {
    switch (t->key) {
      case KEY_DATE_FORMAT:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          (new_val != date_format || !persist_exists(KEY_DATE_FORMAT))) {
          date_format = new_val;
          persist_write_int(KEY_DATE_FORMAT, date_format);
        }
        break;
        
      case KEY_HOUR_LEAD_ZERO:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != show_zero_for_hours || !persist_exists(KEY_HOUR_LEAD_ZERO))) {
          show_zero_for_hours = (new_val == 1);
          persist_write_bool(KEY_HOUR_LEAD_ZERO, show_zero_for_hours);
        }
        break;
        
      case KEY_VIBRATE_HOUR:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != vibrate_on_hour || !persist_exists(KEY_VIBRATE_HOUR))) {
          vibrate_on_hour = (new_val == 1);
          persist_write_bool(KEY_VIBRATE_HOUR, vibrate_on_hour);
        }
        break;
        
      case KEY_VIBRATE_BT_DISC:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != vibrate_on_bt_disc || !persist_exists(KEY_VIBRATE_BT_DISC))) {
          vibrate_on_bt_disc = (new_val == 1);
          persist_write_bool(KEY_VIBRATE_BT_DISC, vibrate_on_bt_disc);
        }
        break;
        
      case KEY_VIBRATE_BT_CONN:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != vibrate_on_bt_conn || !persist_exists(KEY_VIBRATE_BT_CONN))) {
          vibrate_on_bt_conn = (new_val == 1);
          persist_write_bool(KEY_VIBRATE_BT_CONN, vibrate_on_bt_conn);
        }
        break;
        
      case KEY_ALW_SHOW_BT:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != is_bt_shown || !persist_exists(KEY_ALW_SHOW_BT))) {
          is_bt_shown = (new_val == 1);
          persist_write_bool(KEY_ALW_SHOW_BT, is_bt_shown);
        }
        break;
        
      case KEY_ALW_SHOW_BATT:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != is_batt_shown || !persist_exists(KEY_ALW_SHOW_BATT))) {
          is_batt_shown = (new_val == 1);
          persist_write_bool(KEY_ALW_SHOW_BATT, is_batt_shown);
        }
        break;
        
      case KEY_ALW_SHOW_SEC:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != is_sec_shown || !persist_exists(KEY_ALW_SHOW_SEC))) {
          is_sec_shown = (new_val == 1);
          if(is_sec_shown) 
            tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
          else
            tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
          persist_write_bool(KEY_ALW_SHOW_SEC, is_sec_shown);
        }
        break;
        
      case KEY_SHAKE_SHOW_DATE:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != date_show_on_shake || !persist_exists(KEY_SHAKE_SHOW_DATE))) {
          date_show_on_shake = (new_val == 1);
          persist_write_bool(KEY_SHAKE_SHOW_DATE, date_show_on_shake);
        }
        break;
        
      case KEY_SHAKE_SHOW_BT:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != bt_show_on_shake || !persist_exists(KEY_SHAKE_SHOW_BT))) {
          bt_show_on_shake = (new_val == 1);
          persist_write_bool(KEY_SHAKE_SHOW_BT, bt_show_on_shake);
        }
        break;
        
      case KEY_SHAKE_SHOW_BATT:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != batt_show_on_shake || !persist_exists(KEY_SHAKE_SHOW_BATT))) {
          batt_show_on_shake = (new_val == 1);
          persist_write_bool(KEY_SHAKE_SHOW_BATT, batt_show_on_shake);
        }
        break;
        
      case KEY_SHAKE_SHOW_SEC:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 1 && 
          ((new_val == 1) != sec_show_on_shake || !persist_exists(KEY_SHAKE_SHOW_SEC))) {
          sec_show_on_shake = (new_val == 1);
          persist_write_bool(KEY_SHAKE_SHOW_SEC, sec_show_on_shake);
        }
        break;
        
      case KEY_LENTGH_SHOW_DATE:
        new_val = t->value->int16 * 1000;
        if(new_val > 0 && new_val <= 60000 && 
          (new_val != shake_date_timer_delay || !persist_exists(KEY_LENTGH_SHOW_DATE))) {
          shake_date_timer_delay = new_val;
          persist_write_int(KEY_LENTGH_SHOW_DATE, shake_date_timer_delay);
        }
        break;
        
      case KEY_LENGTH_SHOW_BBS:
        new_val = t->value->int16 * 1000;
        if(new_val > 0 && new_val <= 60000 && 
          (new_val != shake_bbs_timer_delay || !persist_exists(KEY_LENGTH_SHOW_BBS))) {
          shake_bbs_timer_delay = new_val;
          persist_write_int(KEY_LENGTH_SHOW_BBS, shake_bbs_timer_delay);
        }
        break;
        
      case KEY_ANIMATION_RATE:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 5 && 
          (new_val != animation_rate || !persist_exists(KEY_ANIMATION_RATE))) {
          animation_rate = new_val;
          persist_write_int(KEY_ANIMATION_RATE, animation_rate);
        }
        break;
        
      case KEY_HAND_LENGTH:
        new_val = t->value->int16;
        if(new_val >= 4 && new_val <= 9 && 
          (new_val != hand_length || !persist_exists(KEY_HAND_LENGTH))) {
          hand_length = new_val;
          persist_write_int(KEY_HAND_LENGTH, hand_length);
        }
        break;
        
      case KEY_COLOR_PATTERN:
        new_val = t->value->int16;
        if(new_val >= 0 && new_val <= 3 && 
          (new_val != color_pattern || !persist_exists(KEY_COLOR_PATTERN))) {
          color_pattern = new_val;
          persist_write_int(KEY_COLOR_PATTERN, color_pattern);
        }
        break;
        
      case KEY_COLOR_BG_H10:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_H10, -1);
        if(new_val != old_color) {
          color_background_h10 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_H10, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_H10:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_H10, -1);
        if(new_val != old_color) {
          color_spocks_h10 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_H10, new_val);
        }
        break;
        
      case KEY_COLOR_BG_H1:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_H1, -1);
        if(new_val != old_color) {
          color_background_h1 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_H1, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_H1:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_H1, -1);
        if(new_val != old_color) {
          color_spocks_h1 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_H1, new_val);
        }
        break;
        
      case KEY_COLOR_BG_M10:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_M10, -1);
        if(new_val != old_color) {
          color_background_m10 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_M10, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_M10:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_M10, -1);
        if(new_val != old_color) {
          color_spocks_m10 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_M10, new_val);
        }
        break;
        
      case KEY_COLOR_BG_M1:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_M1, -1);
        if(new_val != old_color) {
          color_background_m1 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_M1, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_M1:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_M1, -1);
        if(new_val != old_color) {
          color_spocks_m1 = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_M1, new_val);
        }
        break;
        
      case KEY_COLOR_BG_SEC:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_SEC, -1);
        if(new_val != old_color) {
          color_background_sec = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_SEC, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_SEC:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_SEC, -1);
        if(new_val != old_color) {
          color_spocks_sec = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_SEC, new_val);
        }
        break;
        
      case KEY_COLOR_BG_BT:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_BG_BT, -1);
        if(new_val != old_color) {
          color_background_bt = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_BG_BT, new_val);
        }
        break;
        
      case KEY_COLOR_SPOCK_BT:
        new_val = t->value->int32;
        old_color = persist_read_int_safe(KEY_COLOR_SPOCK_BT, -1);
        if(new_val != old_color) {
          color_spocks_bt = GColorFromInt(new_val);
          persist_write_int(KEY_COLOR_SPOCK_BT, new_val);
        }
        break;
    }
    
    t = dict_read_next(iter);
  }
  
  set_right_spock_colors();
  layer_mark_dirty(window_get_root_layer(my_window));
}

void animation_callback(void *data) {
  //LOG("Animation callback");
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void spock_layer_update_proc(Layer *layer, GContext *ctx) {
  LOG("Updating background layer");
  //Boolean to help with the animation
  bool stop_animation = true;
  
  if(current_middle_animation > 0) {
    // Draw the full screen with some geometric pattern
    stop_animation = !draw_geometric_pattern_fullscreen(ctx) && stop_animation;
  } else {
    //Updating the time (hours and minutes or date)
    if(is_shake_x_active && date_show_on_shake) {
      stop_animation = !draw_date_numbers(ctx, d10, d1, m10, m1) && stop_animation;
    } else {
      stop_animation = !draw_time(ctx, hour10, hour1, min10, min1) && stop_animation;
    }
    
    //Updating the seconds
    if(is_sec_shown || (is_shake_y_active && sec_show_on_shake)) {
      stop_animation = !draw_seconds(ctx, sec) && stop_animation;
    } else {
      stop_animation = !draw_seconds(ctx, -1) && stop_animation;
    }
    
    //Updating the bluetooth status
    if(is_bt_shown || (is_shake_y_active && bt_show_on_shake)) {
      stop_animation = !draw_bluetooth(ctx, is_bt_connected) && stop_animation;
    } else {
      stop_animation = !draw_empty_bluetooth(ctx) && stop_animation;
    }
    
    //Updating the battery level
    if(is_batt_shown || (is_shake_y_active && batt_show_on_shake)) {
      stop_animation = !draw_battery(ctx, batt_level) && stop_animation;
    } else {
      stop_animation = !draw_battery(ctx, -1) && stop_animation;
    }
    
  }
  
  if(!stop_animation || current_middle_animation > 0) {
    if (stop_animation) {
      current_middle_animation--;
      if(current_middle_animation > 0) {
        middle_shape = rand() % NUM_SHAPES;
      }
    }
    animation_timer = app_timer_register(timer_delay, (AppTimerCallback) animation_callback, NULL);
  }
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  switch(color_pattern) {
    case BACKGROUND_EQUAL:
      //All the background has the same color
      graphics_context_set_fill_color(ctx, color_background_h10);
      graphics_fill_rect(ctx, GRect(0, 0, PEBBLE_WIDTH, PEBBLE_HEIGHT), 0, GCornerNone);
      break;
      
    case BACKGROUND_DIFF_SIDE_BAR:
      //Color for the BT and battery bar
      // Needs an extra pixel somehow on the horizontal size.
      graphics_context_set_fill_color(ctx, color_background_bt);
      graphics_fill_rect(ctx, GRect(0, 0, RADIUS_SPOCK*2 + W_OFFSET + 1, PEBBLE_HEIGHT), 0, GCornerNone);
      //Color for the time
      // Needs an extra pixel somehow on the horizontal start point.
      graphics_context_set_fill_color(ctx, color_background_h10);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 + W_OFFSET + 1, 0, 
                                    RADIUS_SPOCK*2 * (SPOCKS_SCREEN_WIDTH - 2) + W_OFFSET, PEBBLE_HEIGHT), 0, GCornerNone);
      //Color for the seconds bar
      graphics_context_set_fill_color(ctx, color_background_sec);
      graphics_fill_rect(ctx, GRect(PEBBLE_WIDTH - RADIUS_SPOCK*2 - W_OFFSET, 0,
                                    RADIUS_SPOCK*2 + W_OFFSET, PEBBLE_HEIGHT), 0, GCornerNone);
      break;
      
    case BACKGROUND_HOUR_MIN:
      //Color for the BT and battery bar
      // Needs an extra pixel somehow on the horizontal size.
      graphics_context_set_fill_color(ctx, color_background_bt);
      graphics_fill_rect(ctx, GRect(0, 0, RADIUS_SPOCK*2 + W_OFFSET + 1, PEBBLE_HEIGHT), 0, GCornerNone);
      //Color for the hour
      // Needs an extra pixel somehow on the horizontal start point.
      graphics_context_set_fill_color(ctx, color_background_h10);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 + W_OFFSET + 1, 0, 
                                    RADIUS_SPOCK*2 * (SPOCKS_SCREEN_WIDTH - 2) + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the minutes
      // Needs an extra pixel somehow on the horizontal start point.
      graphics_context_set_fill_color(ctx, color_background_m10);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 + W_OFFSET + 1, RADIUS_SPOCK*2 * NUMBER_HEIGHT, 
                                    RADIUS_SPOCK*2 * (SPOCKS_SCREEN_WIDTH - 2) + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the seconds bar
      graphics_context_set_fill_color(ctx, color_background_sec);
      graphics_fill_rect(ctx, GRect(PEBBLE_WIDTH - RADIUS_SPOCK*2 - W_OFFSET, 0,
                                    RADIUS_SPOCK*2 + W_OFFSET, PEBBLE_HEIGHT), 0, GCornerNone);
      break;
      
    case BACKGROUND_ALL_DIFF:
      //Color for the BT and battery bar
      // Needs an extra pixel somehow on the horizontal size.
      graphics_context_set_fill_color(ctx, color_background_bt);
      graphics_fill_rect(ctx, GRect(0, 0, RADIUS_SPOCK*2 + W_OFFSET + 1, PEBBLE_HEIGHT), 0, GCornerNone);
      //Color for the hour, first digit
      // Needs an extra pixel somehow on the horizontal start point.
      graphics_context_set_fill_color(ctx, color_background_h10);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 + W_OFFSET + 1, 0, 
                                    RADIUS_SPOCK*2 * NUMBER_WIDTH + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the hour, second digit
      graphics_context_set_fill_color(ctx, color_background_h1);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 * (NUMBER_WIDTH + 1) + W_OFFSET, 0, 
                                    RADIUS_SPOCK*2 * NUMBER_WIDTH + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the minutes, first digit
      // Needs an extra pixel somehow on the horizontal start point.
      graphics_context_set_fill_color(ctx, color_background_m10);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 + W_OFFSET + 1, RADIUS_SPOCK*2 * NUMBER_HEIGHT, 
                                    RADIUS_SPOCK*2 * NUMBER_WIDTH + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the minutes, second digit
      graphics_context_set_fill_color(ctx, color_background_m1);
      graphics_fill_rect(ctx, GRect(RADIUS_SPOCK*2 * (NUMBER_WIDTH + 1) + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT, 
                                    RADIUS_SPOCK*2 * NUMBER_WIDTH + W_OFFSET, RADIUS_SPOCK*2 * NUMBER_HEIGHT),
                         0, GCornerNone);
      //Color for the seconds bar
      graphics_context_set_fill_color(ctx, color_background_sec);
      graphics_fill_rect(ctx, GRect(PEBBLE_WIDTH - RADIUS_SPOCK*2 - W_OFFSET, 0,
                                    RADIUS_SPOCK*2 + W_OFFSET, PEBBLE_HEIGHT), 0, GCornerNone);
      break;
  }
}

static void load_saved_config(){
  //Behaviour
  show_zero_for_hours = persist_read_bool_safe(KEY_HOUR_LEAD_ZERO, false);
  
  vibrate_on_hour = persist_read_bool_safe(KEY_VIBRATE_HOUR, false);
  vibrate_on_bt_disc = persist_read_bool_safe(KEY_VIBRATE_BT_DISC, false);
  vibrate_on_bt_conn = persist_read_bool_safe(KEY_VIBRATE_BT_CONN, false);
  
  is_bt_shown = persist_read_bool_safe(KEY_ALW_SHOW_BT, false);
  is_batt_shown = persist_read_bool_safe(KEY_ALW_SHOW_BATT, false);
  is_sec_shown = persist_read_bool_safe(KEY_ALW_SHOW_SEC, false);
  
  bt_show_on_shake = persist_read_bool_safe(KEY_SHAKE_SHOW_BT, true);
  batt_show_on_shake = persist_read_bool_safe(KEY_SHAKE_SHOW_BATT, true);
  sec_show_on_shake = persist_read_bool_safe(KEY_SHAKE_SHOW_SEC, true);
  date_show_on_shake = persist_read_bool_safe(KEY_SHAKE_SHOW_DATE, true);
  
  shake_bbs_timer_delay = persist_read_int_safe(KEY_LENGTH_SHOW_BBS, 30000);
  shake_date_timer_delay = persist_read_int_safe(KEY_LENTGH_SHOW_DATE, 10000);
  
  date_format = persist_read_int_safe(KEY_DATE_FORMAT, DATE_FORMAT_DD_MM);
  
  animation_rate = persist_read_int_safe(KEY_ANIMATION_RATE, ANIMATE_EVERY_30M);
  
  //Style
  hand_length = persist_read_int_safe(KEY_HAND_LENGTH, NORMAL_LENGTH_HAND_SPOCK);
  
  color_pattern = persist_read_int_safe(KEY_COLOR_PATTERN, BACKGROUND_HOUR_MIN);

  int saved_color = persist_read_int_safe(KEY_COLOR_BG_H10, 0);
  color_background_h10 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_BG_H1, 0);
  color_background_h1 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_BG_M10, 0);
  color_background_m10 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_BG_M1, 0);
  color_background_m1 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_BG_SEC, 0);
  color_background_sec = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_BG_BT, 0);
  color_background_bt = GColorFromInt(saved_color);
  
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_H10, 16777215);
  color_spocks_h10 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_H1, 16777215);
  color_spocks_h1 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_M10, 16777215);
  color_spocks_m10 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_M1, 16777215);
  color_spocks_m1 = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_SEC, 16777215);
  color_spocks_sec = GColorFromInt(saved_color);
  saved_color = persist_read_int_safe(KEY_COLOR_SPOCK_BT, 16777215);
  color_spocks_bt = GColorFromInt(saved_color);
  
  set_right_spock_colors();
}

static void window_load(Window *window) {
  LOG("Window load");
  
  load_saved_config();
  is_bt_connected = bluetooth_connection_service_peek();
  batt_level = battery_state_service_peek().charge_percent / 10;
  
  middle_shape = rand() % NUM_SHAPES;
  
  update_time();
  current_middle_animation = MAX_ANIMATIONS;
  
  // Register with TickTimerService
  if(is_sec_shown) 
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  else
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_bounds(window_layer);

  bg_layer = layer_create(bounds);
  spock_layer = layer_create(bounds);
  
  layer_set_update_proc(bg_layer, bg_update_proc);
  layer_add_child(window_layer, bg_layer);
  
  layer_set_update_proc(spock_layer, spock_layer_update_proc);
  layer_add_child(window_layer, spock_layer);
}

static void window_unload(Window *window) {
  LOG("Window unload");
  layer_destroy_safe(bg_layer);
  layer_destroy_safe(spock_layer);
}

void handle_init(void) {
  LOG("INIT");
  // Create main Window element and assign to pointer
  my_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(my_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });
  
  window_stack_push(my_window, true);
  
  app_message_register_inbox_received((AppMessageInboxReceived) inbox_received_handler);
  app_message_open_max();
  
  init_centers_array();
  init_spocks_array();
  
  //Subscribe to BluetoothConnectionService and BatteryStateService and AccelerometerService
  bluetooth_connection_service_subscribe(bt_handler);
  battery_state_service_subscribe(batt_handler);
  accel_tap_service_subscribe(shake_handler);
}

void handle_deinit(void) {
  LOG("DEINIT");
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  accel_tap_service_unsubscribe();
  window_destroy_safe(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
