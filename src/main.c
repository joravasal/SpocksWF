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
static bool is_bt_shown = false;
static bool is_batt_shown = false;
static bool is_sec_shown = false;

static bool bt_show_on_shake = false;
static bool batt_show_on_shake = false;
static bool sec_show_on_shake = false;
static bool is_shake_active = false;
static AppTimer *shake_timer;
static int shake_timer_delay = 30000;
//Look into making the show date different, can the watch really distinguish between the shake on different directions??
//  If it can, different direcion-shake for date would be nice, would require a different timer too.
static bool date_show_on_shake = false;

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
static GColor color_background_m1;;
static GColor color_background_sec;
static GColor color_background_bt;
GColor color_spocks_h10;
GColor color_spocks_h1;
GColor color_spocks_m10;
GColor color_spocks_m1;
GColor color_spocks_sec;
GColor color_spocks_bt;

static AppTimer *animation_timer; //for the animation
const int timer_delay = 30;

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
  
  if(is_sec_shown || (is_shake_active && sec_show_on_shake)) {
    new_sec = time->tm_sec / 5;
    if (sec != new_sec) {
      sec = new_sec;
      res = true;
    }
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
  /*if(connected == true) {
  }
  else {
  }*/
}

static void batt_handler(BatteryChargeState charge) {
  LOG("Battery state changed: %d%%", charge.charge_percent);
  batt_level = charge.charge_percent / 10;
  if(is_batt_shown) {
    layer_mark_dirty(window_get_root_layer(my_window));
  }
}

static void end_shake_timer(void *data) {
  shake_timer = NULL;
  is_shake_active = false;
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
    is_shake_active = true;
    if(batt_show_on_shake) {
      batt_level = battery_state_service_peek().charge_percent / 10;
    }
    if(bt_show_on_shake) {
      is_bt_connected = bluetooth_connection_service_peek();
    }
    if(sec_show_on_shake) {
      tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    }
    if(date_show_on_shake) {
      middle_shape = rand() % NUM_SHAPES;
      current_middle_animation = MAX_ANIMATIONS;
    }
    layer_mark_dirty(window_get_root_layer(my_window));
    shake_timer = app_timer_register(shake_timer_delay, end_shake_timer, NULL);
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      INFO("Y>0");
    } else {
      INFO("Y<0");
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

void animation_callback(void *data) {
  //LOG("Animation callback");
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void set_colors() {
  #ifdef PBL_COLOR
  color_background_h10 = GColorYellow;
  color_background_h1 = GColorYellow;
  color_background_m10 = GColorYellow;
  color_background_m1 = GColorYellow;
  color_background_sec = GColorDarkGreen;
  color_background_bt = GColorDarkGreen;
  color_spocks_h10 = GColorBlack;
  color_spocks_h1 = GColorBlack;
  color_spocks_m10 = GColorBlack;
  color_spocks_m1 = GColorBlack;
  color_spocks_sec = GColorWhite;
  color_spocks_bt = GColorWhite;
  #else
  color_background_h10 = GColorBlack;
  color_background_h1 = GColorWhite;
  color_background_m10 = GColorWhite;
  color_background_m1 = GColorBlack;;
  color_background_sec = GColorWhite;
  color_background_bt = GColorBlack;
  color_spocks_h10 = GColorWhite;
  color_spocks_h1 = GColorWhite;
  color_spocks_m10 = GColorBlack;
  color_spocks_m1 = GColorBlack;
  color_spocks_sec = GColorBlack;
  color_spocks_bt = GColorWhite;
  #endif
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
    if(is_shake_active && date_show_on_shake) {
      stop_animation = !draw_date_numbers(ctx, d10, d1, m10, m1) && stop_animation;
    } else {
      stop_animation = !draw_time(ctx, hour10, hour1, min10, min1) && stop_animation;
    }
    
    //Updating the seconds
    if(is_sec_shown || (is_shake_active && sec_show_on_shake)) {
      stop_animation = !draw_seconds(ctx, sec) && stop_animation;
    } else {
      stop_animation = !draw_seconds(ctx, -1) && stop_animation;
    }
    
    //Updating the bluetooth status
    if(is_bt_shown || (is_shake_active && bt_show_on_shake)) {
      stop_animation = !draw_bluetooth(ctx, is_bt_connected) && stop_animation;
    } else {
      stop_animation = !draw_empty_bluetooth(ctx) && stop_animation;
    }
    
    //Updating the battery level
    if(is_batt_shown || (is_shake_active && batt_show_on_shake)) {
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

static void window_load(Window *window) {
  LOG("Window load");
  //Grab all these values from permanent memory
  is_bt_shown = false;
  is_bt_connected = bluetooth_connection_service_peek();
  is_batt_shown = false;
  batt_level = battery_state_service_peek().charge_percent / 10;
  is_sec_shown = true;
  
  bt_show_on_shake = true;
  batt_show_on_shake = true;
  sec_show_on_shake = true;
  date_show_on_shake = true;
  date_format = DATE_FORMAT_DD_MM;

  show_zero_for_hours = false;
  
  animation_rate = ANIMATE_EVERY_30M;
  
  hand_length = NORMAL_LENGTH_HAND_SPOCK;

  set_colors();
  
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
  window_destroy_safe(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
