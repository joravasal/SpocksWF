#include <pebble.h>
#include "main.h"
#include "pebble-assist.h"
#include "numbers.h"

// After dividing the screen on squares with 14px per side,
//there are 4 pixels left on the width
#define W_OFFSET 2

Window *my_window;
static Layer *bg_layer;

static int min10;
static int min1;
static int hour10;
static int hour1;

static const int maxNumMiddleAnimation = 2;
static int numMiddleAnimation = 2;
static int middleShape = 0;

AppTimer *animation_timer; //for the animation
const int timer_delay = 25;

static GPoint array_hour_10[NUMBER_HEIGHT][NUMBER_WIDTH] = {
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
};

static GPoint array_hour_1[NUMBER_HEIGHT][NUMBER_WIDTH] = {
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
};

static GPoint array_minute_10[NUMBER_HEIGHT][NUMBER_WIDTH] = {
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
};

static GPoint array_minute_1[NUMBER_HEIGHT][NUMBER_WIDTH] = {
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
  {{0, 180}, {0, 180}, {0, 180}, {0, 180},},
};

static bool draw_array(GContext *ctx, GPoint array[NUMBER_HEIGHT][NUMBER_WIDTH],
                       const GPoint array_obj[NUMBER_HEIGHT][NUMBER_WIDTH],
                       int offset_w, int offset_h) {
  LOG("Drawing an array of tiny clocks");
  bool equals = true;
  for(int i = 0; i < NUMBER_HEIGHT; i++) {
    for(int j = 0; j < NUMBER_WIDTH; j++) {
      //DEBUG("Step i = %d - j = %d", i, j);
      GPoint center = NUMBER_CENTERS[i][j];
      center.x = center.x + offset_w + W_OFFSET;
      center.y = center.y + offset_h;
      
      int32_t angle1 = array[i][j].x;
      int32_t angle2 = array[i][j].y;
      if(array_obj != NULL) {
        if(array_obj[i][j].x == -1) {
          angle1 = (angle1 + ANGLE_STEP) % 360;
          array[i][j].x = angle1;
          angle2 = (angle2 + ANGLE_STEP) % 360;
          array[i][j].y = angle2;
        } else {
          if (array_obj[i][j].x != angle1) {
            angle1 = (angle1 + ANGLE_STEP) % 360;
            array[i][j].x = angle1;
            equals = false;
          }
          if (array_obj[i][j].y != angle2) {
            angle2 = (angle2 + ANGLE_STEP) % 360;
            array[i][j].y = angle2;
            equals = false;
          }
        }
      }
      angle1 = TRIG_MAX_ANGLE * angle1 / 360;
      angle2 = TRIG_MAX_ANGLE * angle2 / 360;
      
      GPoint point1 = {
        .x = (int16_t)(cos_lookup(angle1) * RADIUS / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle1) * RADIUS / TRIG_MAX_RATIO) + center.y,
      };
      GPoint point2 = {
        .x = (int16_t)(cos_lookup(angle2) * RADIUS / TRIG_MAX_RATIO) + center.x,
        .y = (int16_t)(-sin_lookup(angle2) * RADIUS / TRIG_MAX_RATIO) + center.y,
      };
      
      graphics_context_set_stroke_color(ctx, GColorBlack);
      graphics_draw_line(ctx, center, point1);
      graphics_draw_line(ctx, center, point2);
      
    }
  }
  return equals;
}

static void update_time() {
  LOG("Updating time");
  time_t now = time(NULL);
  struct tm *time = localtime(&now);
  int hour = time->tm_hour;

  min10 = time->tm_min / 10;
  min1 = time->tm_min % 10;
  if(clock_is_24h_style() == false && hour > 12) {
    hour = hour - 12;
  }
  hour10 = hour / 10;
  hour1 = hour % 10;
  
  DEBUG("Time is %d%d:%d%d", hour10, hour1, min10, min1);
}

static void tick_handler(struct tm *time, TimeUnits units_changed) {
  LOG("tick-tock");
  update_time();
  middleShape = rand() % NUM_SHAPES;
  layer_mark_dirty(window_get_root_layer(my_window));
}

void animation_callback(void *data) {
  LOG("Animation callback");
  layer_mark_dirty(window_get_root_layer(my_window));
}

static void bg_update_proc(Layer *layer, GContext *ctx) {
  LOG("Updating background layer");
  bool stop_animation = true;
  stop_animation = draw_array(ctx, array_hour_10, (numMiddleAnimation > 0 ? SHAPES[middleShape] : NUMBERS[hour10]), 0, 0) && stop_animation;
  stop_animation = draw_array(ctx, array_hour_1, (numMiddleAnimation > 0 ? SHAPES[middleShape] : NUMBERS[hour1]), NUMBER_WIDTH*DIAMETER, 0) && stop_animation;
  stop_animation = draw_array(ctx, array_minute_10, (numMiddleAnimation > 0 ? SHAPES[middleShape] : NUMBERS[min10]), 0, NUMBER_HEIGHT*DIAMETER) && stop_animation;
  stop_animation = draw_array(ctx, array_minute_1, (numMiddleAnimation > 0 ? SHAPES[middleShape] : NUMBERS[min1]), NUMBER_WIDTH*DIAMETER, NUMBER_HEIGHT*DIAMETER) && stop_animation;
  if(!stop_animation || numMiddleAnimation > 0) {
    if (stop_animation) {
      numMiddleAnimation--;
      middleShape = rand() % NUM_SHAPES;
    }
    animation_timer = app_timer_register(timer_delay, (AppTimerCallback) animation_callback, NULL);
  } else {
    numMiddleAnimation = maxNumMiddleAnimation;
  }
}

static void window_load(Window *window) {
  LOG("Window load");
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
  
  // Make sure the time is displayed from the start
  update_time();
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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
