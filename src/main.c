#include <pebble.h>
#include "main.h"
#include "pebble-assist.h"
#include "patterns.h"

Window *my_window;
static Layer *bg_layer;
static Layer *spock_layer;

// Time variables
static int min10;
static int min1;
static int hour10;
static int hour1;
static int sec;

// Watch status
static bool is_bt_connected;
static int batt_level;

// Configuration options
static bool is_bt_shown = false;
static bool is_batt_shown = false;
static bool is_sec_shown = false;

static int hand_length = NORMAL_LENGTH_HAND_SPOCK;

// Colors. The pattern can be BACKGROUND_EQUAL, BACKGROUND_DIFF_SIDE_BAR, BACKGROUND_HOUR_MIN and BACKGROUND_ALL_DIFF
static int color_pattern = BACKGROUND_HOUR_MIN;
static GColor color_background_h10;
static GColor color_background_h1;
static GColor color_background_m10;
static GColor color_background_m1;;
static GColor color_background_sec;
static GColor color_background_bt;
static GColor color_spocks_h10;
static GColor color_spocks_h1;
static GColor color_spocks_m10;
static GColor color_spocks_m1;
static GColor color_spocks_sec;
static GColor color_spocks_bt;

// This adds a certain number of shapes to show every ten minutes or on loading the watchface.
static const int maxNumMiddleAnimation = 1;
static int numMiddleAnimation = 1;
// which shape is used next
static int middleShape = 0;

AppTimer *animation_timer; //for the animation
const int timer_delay = 30;

//Stores the angle of each spock in the screen (120).
static GPoint spocks[SPOCKS_SCREEN_HEIGHT][SPOCKS_SCREEN_WIDTH];
static GPoint SPOCKS_CENTERS[SPOCKS_SCREEN_HEIGHT][SPOCKS_SCREEN_WIDTH];

static void init_centers_array() {
  for (int i = 0; i < SPOCKS_SCREEN_HEIGHT; i++) {
    for (int j = 0; j < SPOCKS_SCREEN_WIDTH; j++) {
      GPoint p = {
        .x = RADIUS_SPOCK * 2 * j + RADIUS_SPOCK,
        .y = RADIUS_SPOCK * 2 * i + RADIUS_SPOCK,
      };
      SPOCKS_CENTERS[i][j] = p;
    }
  }
}

static void init_spocks_array() {
  for (int i = 0; i < SPOCKS_SCREEN_HEIGHT; i++) {
    for (int j = 0; j < SPOCKS_SCREEN_WIDTH; j++) {
      GPoint p = {
        .x = 0,
        .y = 180,
      };
      spocks[i][j] = p;
    }
  }
}

/**
 * This function calculates for a position in the spocks_array, which direction each hand should
 * take to make it the fastest into a defined position.
**/
static GPoint calculate_spocks_hands_direction(int i, int j, GPoint to) {
  GPoint res = {0, 0};
  if(to.x == -1) return res;
  
  //First calculate if by changing the first and second hand will make it more efficient
  int xtox = abs(spocks[i][j].x - to.x);
  int ytoy = abs(spocks[i][j].y - to.y);
  int xtoy = abs(spocks[i][j].x - to.y);
  int ytox = abs(spocks[i][j].y - to.x);
  if (xtox > 180) xtox = 360 - xtox;
  if (ytoy > 180) ytoy = 360 - ytoy;
  if (xtoy > 180) xtoy = 360 - xtoy;
  if (ytox > 180) ytox = 360 - ytox;
  if(xtox + ytoy > xtoy + ytox) {
    //Changing hands of side
    int aux = spocks[i][j].x;
    spocks[i][j].x = spocks[i][j].y;
    spocks[i][j].y = aux;
  }
  
  // Now that each hand of the spock is in the right side (x or y), we check if it's faster to
  //increase or decrease the angles for both
  
  if(spocks[i][j].x != to.x) {
    if(spocks[i][j].x < to.x && spocks[i][j].x - to.x <= 180) {
      res.x = 1;
    } else if(spocks[i][j].x < to.x) {
      res.x = -1;
    } else if(spocks[i][j].x - to.x <= 180) {
      res.x = -1;
    } else {
      res.x = 1;
    }
  }
  
  if(spocks[i][j].y != to.y) {
    if(spocks[i][j].y < to.y && spocks[i][j].y - to.y < 180) {
      res.y = 1;
    } else if(spocks[i][j].y < to.y) {
      res.y = -1;
    } else if(spocks[i][j].y - to.y < 180) {
      res.y = -1;
    } else {
      res.y = 1;
    }
  }
  
  return res;
}

/**
 * Draws the array of spocks from the initial at coordinates (param_x_start, param_y_start)
 * a rectangle with param_width columns and param_height rows.
 * If any value is illegal (negative or out of bounds) it will use the default:
 *  for the starting position, (0,0)
 *  for the width and height, until the end of the array.
 *
 *Returns a boolean if there was any *relevant* change made.
 * ***The model used as comparison may have irrelevant areas,
 *    those spocks that are ok in any direction.
 *
 * Examples: draw_array(ctx, -1, -1, -1, -1, array, true, size_w, size_h, color); will draw in the full screen.
 *          draw_array(ctx, -1, -1, 3, 3, array, true, size_w, size_h, color); will draw in a 3x3 square from the first item.
 *          draw_array(ctx, 8, 0, -1, -1, array, true, size_w, size_h, color); will draw in the last two columns.
**/
static bool draw_array(GContext *ctx, int param_x_start, int param_y_start,
                       int param_width, int param_height,
                       GPoint *array_objective, bool is_position_agnostic,
                       int obj_width, int obj_height, GColor spocks_color) {
  LOG("Drawing an array of tiny clocks");
  //First check if the parameters are valid or else use the default values
  int x_start = 0;
  if(param_x_start > 0 && param_x_start < SPOCKS_SCREEN_WIDTH) x_start = param_x_start;
  int y_start = 0;
  if(param_y_start > 0 && param_y_start < SPOCKS_SCREEN_HEIGHT) y_start = param_y_start;
  int width = SPOCKS_SCREEN_WIDTH;
  if(param_width > 0 && param_width + x_start <= SPOCKS_SCREEN_WIDTH) width = param_width + x_start;
  int height = SPOCKS_SCREEN_HEIGHT;
  if(param_height > 0 && param_height + y_start <= SPOCKS_SCREEN_HEIGHT) height = param_height + y_start;
  
  bool modified = false; //Will be returned to indicate if the animation should stop.
  
  //Loop through all the spocks that have been indicated to change
  for(int i = y_start; i < height; i++) {
    for(int j = x_start; j < width; j++) {
      GPoint center = SPOCKS_CENTERS[i][j];
      center.x = center.x + W_OFFSET;
      //center.y = center.y + H_OFFSET; //Unnecessary at the moment
      
      int32_t angle1 = spocks[i][j].x;
      int32_t angle2 = spocks[i][j].y;
      
      // If we got an objective to reach, then the angles of the hands will change ANGLE_STEP
      //degrees per step in the animation
      if(array_objective != NULL) {
        int objective_pos;
        if(is_position_agnostic) {
          objective_pos = (i % obj_height) * obj_width + (j % obj_width);
        } else {
          objective_pos = ((i - y_start) % obj_height) * obj_width + ((j - x_start) % obj_width);
        }
        
        GPoint directions;
        if(array_objective[objective_pos].x == -1) {
          //If the objective doesn't care about some spock (value {-1,-1}), we use one of the shapes
          //to make it look better.
          //It's quite bad to have it hardcoded like this, if I ever get to use more complex shapes or
          //make some structural changes into those, this will be affected most likely. As a fast
          //solution it should be good enough for now.
          objective_pos = (i % 2) * 2 + (j % 2);
          directions = calculate_spocks_hands_direction(i, j, SHAPES2X2[middleShape][objective_pos]);
        } else {
          directions = calculate_spocks_hands_direction(i, j, array_objective[objective_pos]);
        }
      
        if(directions.x != 0) {
          if (angle1 == 0) angle1 = 360;
          angle1 = (angle1 + (ANGLE_STEP * directions.x)) % 360;
          spocks[i][j].x = angle1;
          modified = true;
        }
        if(directions.y != 0) {
          if (angle2 == 0) angle2 = 360;
          angle2 = (angle2 + (ANGLE_STEP * directions.y)) % 360;
            spocks[i][j].y = angle2;
            modified = true;
        }
      }
      
      angle1 = TRIG_MAX_ANGLE * angle1 / 360;
      angle2 = TRIG_MAX_ANGLE * angle2 / 360;
      
      GPoint point1 = {
        .x = (int16_t)(cos_lookup(angle1) * hand_length / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle1) * hand_length / TRIG_MAX_RATIO) + center.y,
      };
      GPoint point2 = {
        .x = (int16_t)(cos_lookup(angle2) * hand_length / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle2) * hand_length / TRIG_MAX_RATIO) + center.y,
      };
      
      graphics_context_set_stroke_color(ctx, spocks_color);
      graphics_draw_line(ctx, center, point1);
      graphics_draw_line(ctx, center, point2);
      
    }
  }
  return modified;
}

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
  int new_min = -1;
  int new_sec = -1;
  bool res = false;

  if(clock_is_24h_style() == false && hour > 12) {
    hour = hour - 12;
  }
  hour10 = hour / 10;
  hour1 = hour % 10;
  
  // Animate with special patterns every once in a while
  if (min10 != time->tm_min / 10) {
  //if (min1 != time->tm_min % 10) {
    middleShape = rand() % NUM_SHAPES;
    numMiddleAnimation = maxNumMiddleAnimation;
  }
  min10 = time->tm_min / 10;
  
  new_min = time->tm_min % 10;
  if(min1 != new_min) {
    min1 = new_min;
    res = true;
  }
  
  if(is_sec_shown) {
    new_sec = time->tm_sec % 5;
    new_sec = (time->tm_sec - new_sec) / 5;
    if (sec != new_sec) {
      sec = new_sec;
      res = true;
    }
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
  
  if(numMiddleAnimation > 0) {
    // Draw the full screen with some geometric pattern
    //stop_animation = !draw_array(ctx, -1, -1, -1, -1, SHAPES2X2[middleShape], 2, 2, color_spocks_h10) && stop_animation;
    //!!! Because of the color options we need to draw it element by element !!!
    ///// NUMBERS:
    stop_animation = !draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 SHAPES2X2[middleShape], true, 2, 2, color_spocks_h10) && stop_animation;
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 SHAPES2X2[middleShape], true, 2, 2, color_spocks_h1) && stop_animation;
    stop_animation = !draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 SHAPES2X2[middleShape], true, 2, 2, color_spocks_m10) && stop_animation;
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 SHAPES2X2[middleShape], true, 2, 2, color_spocks_m1) && stop_animation;
    ///// FIRST COLUMN:
    stop_animation = !draw_array(ctx, 0, 0, 1, -1, SHAPES2X2[middleShape], true,
                                 2, 2, color_spocks_bt) && stop_animation;
    ///// LAST COLUMN:
    stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, 0, 1, -1,
                                 SHAPES2X2[middleShape], true, 2, 2, color_spocks_sec) && stop_animation;
  } else {
    //Updating the time (hours and minutes)
    stop_animation = !draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[hour10], false, 
                                 NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h10) && stop_animation;
    
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[hour1], false,
                                 NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h1) && stop_animation;
    
    stop_animation = !draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[min10], false,
                                 NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m10) && stop_animation;
    
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[min1], false,
                                 NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m1) && stop_animation;
    
    //Updating the seconds
    if(is_sec_shown) {
      //Corner at the beginning
      stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1, 1, 1,
                                   SECONDS1X1[0], false, 1, 1, color_spocks_sec) && stop_animation;
      for(int i = 1; i <= sec; i++) {
        //Loop for the middle steps
        stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1,
                                     SECONDS1X1[1], false, 1, 1, color_spocks_sec) && stop_animation;
      }
      if(sec < SPOCKS_SCREEN_HEIGHT - 1) {
        //Corner at the end
        stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 2 - sec, 1, 1,
                                     SECONDS1X1[2], false, 1, 1, color_spocks_sec) && stop_animation;
        for(int i = sec + 2; i < SPOCKS_SCREEN_HEIGHT; i++) {
          //Loop for the rest
          stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1,
                                       SECONDS1X1[3], false, 1, 1, color_spocks_sec) && stop_animation;
        }
      }
    } else {
      stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, 0, 1, -1, NULL,
                                   false, 0, 0, color_spocks_sec) && stop_animation;
    }
    
    //Updating the bluetooth status
    if(is_bt_shown) {
      stop_animation = !draw_array(ctx, 0, 0, 1, 2, is_bt_connected ? BLUETOOTH2X1[1] : BLUETOOTH2X1[0],
                                   false, 1, 2, color_spocks_bt) && stop_animation;
    } else {
      stop_animation = !draw_array(ctx, 0, 0, 1, 2, NULL, false, 0, 0, color_spocks_bt) && stop_animation;
    }
    
    //Updating the battery level
    if(is_batt_shown) {
      if(batt_level < 2) {
        //First corner, if the battery is really low it has special ways to show it.
        stop_animation = !draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - 1, 1, 1, BATTERY1X1[batt_level], false,
                                     1, 1, color_spocks_bt) && stop_animation;
      } else {
        //If the battery is not too low, it's all the same from the bottom
        for(int i = 1; i < batt_level; i++) {
          stop_animation = !draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - i, 1, 1, BATTERY1X1[2], false,
                                       1, 1, color_spocks_bt) && stop_animation;
        }
        //Ends in a corner at the battery level
        stop_animation = !draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - batt_level, 1, 1, BATTERY1X1[3], false,
                                     1, 1, color_spocks_bt) && stop_animation;
      }
      //Finally, the parts that are empty
      for(int i = batt_level + 1; i <= 10; i++) {
        stop_animation = !draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - i, 1, 1, BATTERY1X1[4], false,
                                     1, 1, color_spocks_bt) && stop_animation;
      }
    } else {
      stop_animation = !draw_array(ctx, 0, 2, 1, -1, NULL, false, 0, 0, color_spocks_bt) && stop_animation;
    }
    
  }
  
  if(!stop_animation || numMiddleAnimation > 0) {
    if (stop_animation) {
      numMiddleAnimation--;
      if(numMiddleAnimation > 0) {
        middleShape = rand() % NUM_SHAPES;
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
  is_bt_shown = false; //grab from mem
  is_bt_connected = bluetooth_connection_service_peek();
  is_batt_shown = false; //grab from mem
  batt_level = battery_state_service_peek().charge_percent / 10;
  is_sec_shown = true; //grab from mem
  //grab from mem the colors
  set_colors();
  
  middleShape = rand() % NUM_SHAPES;
  
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
  layer_destroy(bg_layer);
  layer_destroy(spock_layer);
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
  
  //Subscribe to BluetoothConnectionService and BatteryStateService
  bluetooth_connection_service_subscribe(bt_handler);
  battery_state_service_subscribe(batt_handler);
  
  // Make sure the time is displayed from the start
  numMiddleAnimation = maxNumMiddleAnimation;
  update_time();
}

void handle_deinit(void) {
  LOG("DEINIT");
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
