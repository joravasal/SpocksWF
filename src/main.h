#pragma once

#include <pebble.h>
  
// After dividing the screen on squares with 14px per side,
//there are 4 pixels left on the width
#define W_OFFSET 2
//#define H_OFFSET 0 //for completion's sake, maybe needed if the spocks' sizes change.

// The length of the hands of the clocks is quite independent from everything else
//and can be modified freely, reducing to 5 creates a nice dashed effect, 
//increasing will make the hands reach further with no effect on the numbers.
#define LENGTH_HAND_SPOCK 7

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

#define ANGLE_STEP 5
  
#define HIDE_BLUETOOTH 0
#define SHOW_BLUETOOTH 1
#define HIDE_BATTERY 0
#define SHOW_BATTERY 1
#define HIDE_SECONDS 0
#define SHOW_SECONDS 1
