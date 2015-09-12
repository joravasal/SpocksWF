#include "main.h"
#include "patterns.h"

//Stores the angle of each spock in the screen (120).
static GPoint spocks[SPOCKS_SCREEN_HEIGHT][SPOCKS_SCREEN_WIDTH];
//Stores the center point of each Spock, it could be easily calculated on each iteration instead of
//stored in memory. Since memory shouldn't be a problem, better to save the calculations? It might
//help save battery, it's 120 times per minute (at least) calculating the same products and additions.
static GPoint SPOCKS_CENTERS[SPOCKS_SCREEN_HEIGHT][SPOCKS_SCREEN_WIDTH];

void init_centers_array() {
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

void init_spocks_array() {
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
 *Returns a boolean if there was any change made.
 *
 * Examples: draw_array(ctx, -1, -1, -1, -1, array, true, size_w, size_h, color); will draw in the full screen.
 *          draw_array(ctx, -1, -1, 3, 3, array, true, size_w, size_h, color); will draw in a 3x3 square from the first item.
 *          draw_array(ctx, 8, 0, -1, -1, array, true, size_w, size_h, color); will draw in the last two columns.
**/
static bool draw_array(GContext *ctx, int param_x_start, int param_y_start,
                       int param_width, int param_height,
                       GPoint *array_objective, bool is_position_agnostic,
                       int obj_width, int obj_height, GColor spocks_color) {
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
          directions = calculate_spocks_hands_direction(i, j, SHAPES2X2[middle_shape][objective_pos]);
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

bool draw_geometric_pattern_fullscreen(GContext *ctx) {
  bool modified = false;
  //modified = draw_array(ctx, -1, -1, -1, -1, SHAPES2X2[middle_shape], 2, 2, color_spocks_h10) || modified;
  //!!! Because of the color options we need to draw it element by element !!!
  ///// NUMBERS:
  modified = draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape],
                        true, 2, 2, color_spocks_h10) || modified;
  modified = draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape],
                        true, 2, 2, color_spocks_h1) || modified;
  modified = draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape],
                        true, 2, 2, color_spocks_m10) || modified;
  modified = draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape],
                        true, 2, 2, color_spocks_m1) || modified;
  ///// FIRST COLUMN:
  modified = draw_array(ctx, 0, 0, 1, -1, SHAPES2X2[middle_shape], true, 2, 2, color_spocks_bt) || modified;
  ///// LAST COLUMN:
  modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, 0, 1, -1, SHAPES2X2[middle_shape],
                        true, 2, 2, color_spocks_sec) || modified;
  return modified;
}

bool draw_time(GContext *ctx, int hour10, int hour1, int min10, int min1) {
  bool modified = false;
  if (show_zero_for_hours || hour10 != 0) {
    modified = draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[hour10], false,
                          NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h10) || modified;
  } else {
    modified = draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape], true,
                          2, 2, color_spocks_h10) || modified;
  }
  
  modified = draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[hour1], false,
                        NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h1) || modified;
  
  modified = draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[min10], false,
                        NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m10) || modified;
  
  modified = draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, NUMBERS[min1], false,
                        NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m1) || modified;
  return modified;
}

bool draw_date_numbers(GContext *ctx, int d10, int d1, int m10, int m1) {
  bool modified = false;
  if (DATE_FORMAT_MM_DD == date_format && m10 == 0) {
    modified = draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape], true,
                          2, 2, color_spocks_h10) || modified;
  } else {
    modified = draw_array(ctx, 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                          NUMBERS[DATE_FORMAT_DD_MM == date_format ? d10 : m10],
                          false, NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h10) || modified;
  }
  
  modified = draw_array(ctx, NUMBER_WIDTH + 1, 0, NUMBER_WIDTH, NUMBER_HEIGHT,
                        NUMBERS[DATE_FORMAT_DD_MM == date_format ? d1 : m1],
                        false, NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_h1) || modified;
  
  if (DATE_FORMAT_DD_MM == date_format && m10 == 0) {
    modified = draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT, SHAPES2X2[middle_shape], true,
                          2, 2, color_spocks_h10) || modified;
  } else {
    modified = draw_array(ctx, 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                          NUMBERS[DATE_FORMAT_MM_DD == date_format ? d10 : m10],
                          false, NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m10) || modified;
  }
  
  modified = draw_array(ctx, NUMBER_WIDTH + 1, NUMBER_HEIGHT, NUMBER_WIDTH, NUMBER_HEIGHT,
                        NUMBERS[DATE_FORMAT_MM_DD == date_format ? d1 : m1],
                        false, NUMBER_WIDTH, NUMBER_HEIGHT, color_spocks_m1) || modified;
  return modified;
}

bool draw_seconds(GContext *ctx, int sec) {
  bool modified = false;
  if(sec >= 0 && sec <12) {
    //Corner at the beginning
    modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1, 1, 1,
                          SECONDS1X1[0], false, 1, 1, color_spocks_sec) || modified;
    for(int i = 1; i <= sec; i++) {
      //Loop for the middle steps
      modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1,
                            SECONDS1X1[1], false, 1, 1, color_spocks_sec) || modified;
    }
    if(sec < SPOCKS_SCREEN_HEIGHT - 1) {
      //Corner at the end
      modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 2 - sec, 1, 1,
                            SECONDS1X1[2], false, 1, 1, color_spocks_sec) || modified;
      for(int i = sec + 2; i < SPOCKS_SCREEN_HEIGHT; i++) {
        //Loop for the rest
        modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, SPOCKS_SCREEN_HEIGHT - 1 - i, 1, 1,
                              SECONDS1X1[3], false, 1, 1, color_spocks_sec) || modified;
      }
    }
  } else {
    modified = draw_array(ctx, SPOCKS_SCREEN_WIDTH - 1, 0, 1, -1, SHAPES2X2[middle_shape],
                          true, 2, 2, color_spocks_sec) || modified;
  }
  return modified;
}

bool draw_bluetooth(GContext *ctx, bool status) {
  return draw_array(ctx, 0, 0, 1, 2, status ? BLUETOOTH2X1[1] : BLUETOOTH2X1[0], false, 1, 2, color_spocks_bt);
}
bool draw_empty_bluetooth(GContext *ctx) {
  return draw_array(ctx, 0, 0, 1, 2, SHAPES2X2[middle_shape], true, 2, 2, color_spocks_bt);
}

bool draw_battery(GContext *ctx, int batt_level) {
  bool modified = false;
  if(batt_level >= 0 && batt_level <=10) {
    if(batt_level < 2) {
      //First corner, if the battery is really low it has special ways to show it.
      modified = draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - 1, 1, 1, BATTERY1X1[batt_level], false,
                            1, 1, color_spocks_bt) || modified;
    } else {
      //If the battery is not too low, it's all the same from the bottom
      for(int i = 1; i < batt_level; i++) {
        modified = draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - i, 1, 1, BATTERY1X1[2], false,
                              1, 1, color_spocks_bt) || modified;
      }
      //Ends in a corner at the battery level
      modified = draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - batt_level, 1, 1, BATTERY1X1[3], false,
                            1, 1, color_spocks_bt) || modified;
    }
    //Finally, the parts that are empty
    for(int i = batt_level + 1; i <= 10; i++) {
      modified = draw_array(ctx, 0, SPOCKS_SCREEN_HEIGHT - i, 1, 1, BATTERY1X1[4], false,
                            1, 1, color_spocks_bt) || modified;
    }
  } else {
    modified = draw_array(ctx, 0, 2, 1, -1, SHAPES2X2[middle_shape], true, 2, 2, color_spocks_bt) || modified;
  }
  return modified;
}