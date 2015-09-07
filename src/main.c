#include <pebble.h>
#include "main.h"
#include "pebble-assist.h"
#include "numbers.h"

Window *my_window;
static Layer *bg_layer;

static int min10;
static int min1;
static int hour10;
static int hour1;
static int sec;

static bool is_bt_connected;
static int batt_level;

static bool is_bt_shown = false;
static bool is_batt_shown = false;
static bool is_sec_shown = false;

// This adds a number of steps every time the time changes.
static const int maxNumMiddleAnimation = 1;
static int numMiddleAnimation = 1;
// which shape is used next
static int middleShape = 0;

AppTimer *animation_timer; //for the animation
const int timer_delay = 25;

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

// Draws the array of spocks from the initial at coordinates (param_x_start, param_y_start)
//a rectangle with param_width columns and param_height rows.
//If any value is illegal (negative or out of bounds) it will use the default:
//  for the starting position, (0,0)
//  for the width and height, until the end of the array.
//
//Returns a boolean if there was any *relevant* change made.
//***The model used as comparison may have irrelevant areas,
//   those spocks that are ok in any direction.
//
// Examples: draw_array(ctx, -1, -1, -1, -1, array, size_w, size_h); will draw in the full screen.
//          draw_array(ctx, -1, -1, 3, 3, array, size_w, size_h); will draw in a 3x3 square from the first item.
//          draw_array(ctx, 8, 0, -1, -1, array, size_w, size_h); will draw in the last two columns.
static bool draw_array(GContext *ctx, int param_x_start, int param_y_start,
                       int param_width, int param_height,
                       GPoint *array_objective,
                       int obj_width, int obj_height) {
  LOG("Drawing an array of tiny clocks");
  int x_start = 0;
  if(param_x_start > 0 && param_x_start < SPOCKS_SCREEN_WIDTH) x_start = param_x_start;
  int y_start = 0;
  if(param_y_start > 0 && param_y_start < SPOCKS_SCREEN_HEIGHT) y_start = param_y_start;
  int width = SPOCKS_SCREEN_WIDTH - x_start;
  if(param_width > 0 && param_width + x_start <= SPOCKS_SCREEN_WIDTH) width = param_width + x_start;
  int height = SPOCKS_SCREEN_HEIGHT - y_start;
  if(param_height > 0 && param_height + y_start <= SPOCKS_SCREEN_HEIGHT) height = param_height + y_start;
  
  bool modified = false;
  for(int i = y_start; i < height; i++) {
    for(int j = x_start; j < width; j++) {
      GPoint center = SPOCKS_CENTERS[i][j];
      center.x = center.x + W_OFFSET;
      //center.y = center.y + H_OFFSET; //Unnecessary at the moment
      
      int32_t angle1 = spocks[i][j].x;
      int32_t angle2 = spocks[i][j].y;
      if(array_objective != NULL) {
        int obj_pos = ((i - y_start) % obj_height) * obj_width + ((j - x_start) % obj_width);
        LOG("obj_pos=%d", obj_pos);
        if(array_objective[obj_pos].x == -1) {
          angle1 = (angle1 + ANGLE_STEP) % 360;
          spocks[i][j].x = angle1;
          angle2 = (angle2 + ANGLE_STEP) % 360;
          spocks[i][j].y = angle2;
        } else {
          if (array_objective[obj_pos].x != angle1) {
            angle1 = (angle1 + ANGLE_STEP) % 360;
            spocks[i][j].x = angle1;
            modified = true;
          }
          if (array_objective[obj_pos].y != angle2) {
            angle2 = (angle2 + ANGLE_STEP) % 360;
            spocks[i][j].y = angle2;
            modified = true;
          }
        }
      }
      angle1 = TRIG_MAX_ANGLE * angle1 / 360;
      angle2 = TRIG_MAX_ANGLE * angle2 / 360;
      
      GPoint point1 = {
        .x = (int16_t)(cos_lookup(angle1) * LENGTH_HAND_SPOCK / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle1) * LENGTH_HAND_SPOCK / TRIG_MAX_RATIO) + center.y,
      };
      GPoint point2 = {
        .x = (int16_t)(cos_lookup(angle2) * LENGTH_HAND_SPOCK / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle2) * LENGTH_HAND_SPOCK / TRIG_MAX_RATIO) + center.y,
      };
      
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx, center, point1);
      graphics_draw_line(ctx, center, point2);
      
    }
  }
  return modified;
}

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
  
  if (min10 != time->tm_min / 10) {
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
  
  DEBUG("Time is %d%d:%d%d", hour10, hour1, min10, min1);
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

void animation_callback(void *data) {
  //LOG("Animation callback");
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  LOG("Updating background layer");
  //Boolean to help with the animation
  bool stop_animation = true;
  
  if(numMiddleAnimation > 0) {
    // Draw the full screen with some geometric pattern
    stop_animation = !draw_array(ctx, -1, -1, -1, -1, SHAPES2X2[middleShape], 2, 2) && stop_animation;
  } else {
    //Updating the time (hours and minutes)
    stop_animation = !draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 NUMBERS[hour10], NUMBER_WIDTH, NUMBER_HEIGHT) && stop_animation;
    
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 NUMBERS[hour1], NUMBER_WIDTH, NUMBER_HEIGHT) && stop_animation;
    
    stop_animation = !draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 NUMBERS[min10], NUMBER_WIDTH, NUMBER_HEIGHT) && stop_animation;
    
    stop_animation = !draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                                 NUMBERS[min1], NUMBER_WIDTH, NUMBER_HEIGHT) && stop_animation;
    
    //Updating the seconds
    if(is_sec_shown) {
      //Corner at the beginning
      stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1, 1, 1, SECONDS1X1[0], 1, 1) && stop_animation;
      for(int i = 1; i <= sec; i++) {
        //Loop for the middle steps
        stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1, SECONDS1X1[1], 1, 1) && stop_animation;
      }
      if(sec < SPOCKS_SCREEN_HEIGHT - 1) {
        //Corner at the end
        stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 2 - sec, 1, 1, SECONDS1X1[2], 1, 1) && stop_animation;
        for(int i = sec + 2; i < SPOCKS_SCREEN_HEIGHT; i++) {
          //Loop for the rest
          stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1, SECONDS1X1[3], 1, 1) && stop_animation;
        }
      }
    } else {
      stop_animation = !draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, 0, 1, -1, NULL, 0, 0) && stop_animation;
    }
    
    //Updating the bluetooth status
    if(is_bt_shown) {
      stop_animation = !draw_array(ctx, 0, 0, 1, 2, is_bt_connected ? BLUETOOTH2X1[1] : BLUETOOTH2X1[0], 2, 1) && stop_animation;
    } else {
      stop_animation = !draw_array(ctx, 0, 0, 1, 2, NULL, 0, 0) && stop_animation;
    }
    
    //Updating the battery level
    if(is_batt_shown) {
      stop_animation = !draw_array(ctx, 0, 0, 1, -1, NULL, 0, 0) && stop_animation;
    } else {
      stop_animation = !draw_array(ctx, 0, 0, 1, -1, NULL, 0, 0) && stop_animation;
    }
    
  }
  
  if(!stop_animation || numMiddleAnimation > 0) {
    if (stop_animation) {
      numMiddleAnimation--;
      middleShape = rand() % NUM_SHAPES;
    }
    animation_timer = app_timer_register(timer_delay, (AppTimerCallback) animation_callback, NULL);
  }
}

static void window_load(Window *window) {
  LOG("Window load");
  is_bt_shown = true; //grab from mem
  is_batt_shown = false; //grab from mem
  is_sec_shown = false; //grab from mem
  
  // Register with TickTimerService
  if(is_sec_shown) 
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  else
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  Layer *window_layer = window_get_root_layer(my_window);
  GRect bounds = layer_get_bounds(window_layer);

  bg_layer = layer_create(bounds);
  
  layer_set_update_proc(bg_layer, bg_update_proc);
  layer_add_child(window_layer, bg_layer);
}

static void window_unload(Window *window) {
  LOG("Window unload");
  layer_destroy(bg_layer);
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
  
  //Subscribe to BluetoothConnectionService
  bluetooth_connection_service_subscribe(bt_handler);
  
  // Make sure the time is displayed from the start
  numMiddleAnimation = maxNumMiddleAnimation;
  update_time();
}

void handle_deinit(void) {
  LOG("DEINIT");
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
