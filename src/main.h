#pragma once

#include <pebble.h>
#include "pebble-assist.h"

//Configuration keys
//behaviour options
#define KEY_HOUR_LEAD_ZERO 0

#define KEY_VIBRATE_HOUR 1
#define KEY_VIBRATE_BT_DISC 2
#define KEY_VIBRATE_BT_CONN 3

#define KEY_ALW_SHOW_BT 4
#define KEY_ALW_SHOW_BATT 5
#define KEY_ALW_SHOW_SEC 6

#define KEY_SHAKE_SHOW_DATE 7
#define KEY_SHAKE_SHOW_BT 8
#define KEY_SHAKE_SHOW_BATT 9
#define KEY_SHAKE_SHOW_SEC 10

#define KEY_LENTGH_SHOW_DATE 11
#define KEY_LENGTH_SHOW_BBS 12

#define KEY_ANIMATION_RATE 13

//style options
#define KEY_HAND_LENGTH 14

#define KEY_COLOR_PATTERN 15

#define KEY_COLOR_BG_H10 16
#define KEY_COLOR_SPOCK_H10 17

#define KEY_COLOR_BG_H1 18
#define KEY_COLOR_SPOCK_H1 19

#define KEY_COLOR_BG_M10 20
#define KEY_COLOR_SPOCK_M10 21

#define KEY_COLOR_BG_M1 22
#define KEY_COLOR_SPOCK_M1 23

#define KEY_COLOR_BG_SEC 24
#define KEY_COLOR_SPOCK_SEC 25

#define KEY_COLOR_BG_BT 26
#define KEY_COLOR_SPOCK_BT 27

#define KEY_DATE_FORMAT 28

// Number of geometrical patterns for the middle animations
#define NUM_SHAPES 8

// After dividing the screen on squares with 14px per side,
//there are 4 pixels left on the width
#define W_OFFSET 2
//#define H_OFFSET 0 //for completion's sake, maybe needed if the spocks' sizes change.

// The radius is quite important and it will define how many spocks fit on the screen,
//by increasing it "resolution" for the numbers is lost so it is not really useful.
//If decreased, there is more space for spocks and the numbers look smaller, could be
//interesting to test with a size 5 and see what can fit.
#define RADIUS_SPOCK 7

#define NUMBER_WIDTH 4
#define NUMBER_HEIGHT 6

// At the moment these two values are hardcoded although they depend on the RADIUS_SPOCK.
//With these numbers most of the screen is used, only leaving 4 columns of pixels unused
//on the width (10 spocks * 14px diameter each = 140px out of 144px).
//For the height it gives an exact number: 12 spocks * 14px diameter each = 168 px
#define SPOCKS_SCREEN_WIDTH 10
#define SPOCKS_SCREEN_HEIGHT 12

#define ANGLE_STEP 15

// Configuration options' values

// The length of the hands of the clocks is quite independent from everything else
//and can be modified freely, reducing to 5 creates a nice dashed effect for example, 
//increasing will make the hands reach further with no effect on the numbers
//unless it goes over the radius of a spock twice.
#define NORMAL_LENGTH_HAND_SPOCK 7
#define SHORT_LENGTH_HAND_SPOCK 6
#define VERY_SHORT_LENGTH_HAND_SPOCK 5
#define TOO_SHORT_LENGTH_HAND_SPOCK 4
#define LONG_LENGTH_HAND_SPOCK 9

#define BACKGROUND_EQUAL 0
#define BACKGROUND_DIFF_SIDE_BAR 1
#define BACKGROUND_HOUR_MIN 2
#define BACKGROUND_ALL_DIFF 3

#define ANIMATE_ONLY_ON_SHAKE 0
#define ANIMATE_EVERY_1M 1
#define ANIMATE_EVERY_10M 2
#define ANIMATE_EVERY_15M 3
#define ANIMATE_EVERY_30M 4
#define ANIMATE_EVERY_1H 5

#define DATE_FORMAT_DD_MM 0
#define DATE_FORMAT_MM_DD 1


// Variables that need to be used in main.c and spocks.c
extern int middle_shape;

extern int date_format;
extern bool show_zero_for_hours;

extern int hand_length;

extern GColor color_spocks_h10;
extern GColor color_spocks_h1;
extern GColor color_spocks_m10;
extern GColor color_spocks_m1;
extern GColor color_spocks_sec;
extern GColor color_spocks_bt;