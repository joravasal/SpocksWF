#pragma once

#include <pebble.h>
  
#define RADIUS 7
#define DIAMETER 14
#define NUMBER_WIDTH 4
#define NUMBER_HEIGHT 6

#define ANGLE_STEP 5
  
static const GPoint NUMBER_CENTERS[NUMBER_HEIGHT][NUMBER_WIDTH] = {
  {{RADIUS, RADIUS,}, {RADIUS * 3, RADIUS,}, {RADIUS * 5, RADIUS,}, {RADIUS * 7, RADIUS,},},
  {{RADIUS, RADIUS * 3,}, {RADIUS * 3, RADIUS * 3,}, {RADIUS * 5, RADIUS * 3,}, {RADIUS * 7, RADIUS * 3,},},
  {{RADIUS, RADIUS * 5,}, {RADIUS * 3, RADIUS * 5,}, {RADIUS * 5, RADIUS * 5,}, {RADIUS * 7, RADIUS * 5,},},
  {{RADIUS, RADIUS * 7,}, {RADIUS * 3, RADIUS * 7,}, {RADIUS * 5, RADIUS * 7,}, {RADIUS * 7, RADIUS * 7,},},
  {{RADIUS, RADIUS * 9,}, {RADIUS * 3, RADIUS * 9,}, {RADIUS * 5, RADIUS * 9,}, {RADIUS * 7, RADIUS * 9,},},
  {{RADIUS, RADIUS * 11,}, {RADIUS * 3, RADIUS * 11,}, {RADIUS * 5, RADIUS * 11,}, {RADIUS * 7, RADIUS * 11,},},
};