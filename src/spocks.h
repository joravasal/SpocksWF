#pragma once

#include <pebble.h>

void init_centers_array();
void init_spocks_array();

bool draw_geometric_pattern_fullscreen(GContext *ctx);

bool draw_time(GContext *ctx, int hour10, int hour1, int min10, int min1);
bool draw_date_numbers(GContext *ctx, int d10, int d1, int m10, int m1);

bool draw_seconds(GContext *ctx, int sec);

bool draw_bluetooth(GContext *ctx, bool status);
bool draw_empty_bluetooth(GContext *ctx);

bool draw_battery(GContext *ctx, int level);