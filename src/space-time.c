#include <pebble.h>

static bool debug = false;

static Window *window;
static Layer *window_layer;

static BitmapLayer *background_layer;
static GBitmap *background_img;

static BitmapLayer *player_layer;
static GBitmap *player_img;

static BitmapLayer *bt_layer;
static GBitmap *bt_img;

static TextLayer *hour_layer;
static TextLayer *minute_layer;

static TextLayer *batt_layer;
static TextLayer *batt_layer;

static GFont *time_font;
static GFont *batt_font;


/*
 * Actually draws the screen.
 *
 * We call this on init to prevent flashing the display
 */

static void do_draw () {

  window_layer = window_get_root_layer(window);
  window_set_background_color(window, GColorBlack);

  // Resources
  time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_18));
  batt_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_12));
  background_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  player_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PLAYERSHIP);

  // Background - stars and bottom-left/bottom-right pane (144x168)
  GRect background_frame = (GRect) {
    .origin = {0,0},
    .size = {144,168}
  };
  background_layer = bitmap_layer_create(background_frame);
  bitmap_layer_set_bitmap(background_layer, background_img);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));


  // "Player" Space Ship
  GRect player_frame = (GRect) {
    .origin = {51,86},
    .size = {44,45}
  };
  player_layer = bitmap_layer_create(player_frame);
  bitmap_layer_set_bitmap(player_layer, player_img);
  layer_add_child(window_layer, bitmap_layer_get_layer(player_layer)); 


  // Hour
  static char hour_text[] = "00";
  GRect hour_frame = (GRect) {
    .origin = {30,136},
    .size = {28, 24}
  };
  hour_layer = text_layer_create(hour_frame);

  text_layer_set_background_color(hour_layer, GColorClear);
  text_layer_set_text_color(hour_layer, GColorWhite);
  text_layer_set_font(hour_layer, time_font);
  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_text_alignment(hour_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(hour_layer));

  // Minute
  static char minute_text[] = "00";
  GRect minute_frame = (GRect) {
    .origin = {89,136},
    .size = {28, 24}
  };
  minute_layer = text_layer_create(minute_frame);
  text_layer_set_background_color(minute_layer, GColorClear);
  text_layer_set_text_color(minute_layer, GColorWhite);
  text_layer_set_font(minute_layer, time_font);
  text_layer_set_text(minute_layer, minute_text);

  text_layer_set_text_alignment(minute_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(minute_layer));

  
  // Battery percentage
  BatteryChargeState charge_state = battery_state_service_peek();
  
  static char battery_text[] = "100%";

  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  GRect batt_frame = (GRect) {
    .origin = {12,1},
    .size = {28, 12}
  };
  batt_layer = text_layer_create(batt_frame);
  text_layer_set_background_color(batt_layer, GColorClear);
  text_layer_set_text_color(batt_layer, GColorWhite);
  text_layer_set_font(batt_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_IMAGINE_12)));

  text_layer_set_text(batt_layer, battery_text);

  text_layer_set_text_alignment(batt_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(batt_layer));

  // Bluetooth connectivity
  if (bluetooth_connection_service_peek()) {
    GRect bt_frame = (GRect) {
      .origin = {2,2},
      .size = {8,12}
    };

    bt_layer = bitmap_layer_create(bt_frame);
    bt_img = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BTCONN);
    bitmap_layer_set_bitmap(bt_layer, bt_img);
    layer_add_child(window_layer, bitmap_layer_get_layer(bt_layer));
  }  

}

static void do_update_time (struct tm *tick_time) {
  // TODO: get hour and minute text
  static char hour_text[] = "00";
  static char minute_text[] = "00";

  char* hour_format = "%I";
  if (clock_is_24h_style()) {
    hour_format = "%R";
  }
  strftime(hour_text, sizeof(hour_text), hour_format, tick_time);
  strftime(minute_text, sizeof(minute_text), "%M", tick_time);

  text_layer_set_text(hour_layer, hour_text);
  text_layer_set_text(minute_layer, minute_text);

}

static void do_update_bt () {
  if (bluetooth_connection_service_peek()) {
    layer_add_child(window_layer, bitmap_layer_get_layer(bt_layer));
  }
  else {
    layer_remove_from_parent(bitmap_layer_get_layer(bt_layer));
  }
}

static void do_update_battery () {
  BatteryChargeState charge_state = battery_state_service_peek();
  static char battery_text[] = "100%";
  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);

  text_layer_set_text(batt_layer, battery_text);

}

void bluetooth_connection_callback (bool connected) {
  APP_LOG(APP_LOG_LEVEL_INFO, "bluetooth connected=%d", (int) connected);
  do_update_bt();
}

static void handle_battery (BatteryChargeState charge_state) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery state changed");
  do_update_battery();
}


static void handle_tick (struct tm *tick_time, TimeUnits units_changed) {
  
  // If the minute was what changed
  APP_LOG(APP_LOG_LEVEL_INFO, "Minute change.");
  do_update_time(tick_time);
  
}

static void window_load (Window *window) {
  
  // Enable the light for debug
  light_enable(debug);

  // Actually draw
  do_draw();
}

static void window_unload (Window *window) {
  // Nuke the layers
  bitmap_layer_destroy(background_layer);
  bitmap_layer_destroy(player_layer);

  text_layer_destroy(hour_layer);
  text_layer_destroy(minute_layer);

  gbitmap_destroy(bt_img);
  gbitmap_destroy(player_img);
  gbitmap_destroy(background_img);

  fonts_unload_custom_font(time_font);
  fonts_unload_custom_font(batt_font);

  // Return backlight to previous state
  light_enable(!false);
}

static void init (void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_fullscreen(window, true);
  const bool animated = true;
  window_stack_push(window, animated);


  do_draw();
  do_update_battery();
  do_update_bt();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
  bluetooth_connection_service_subscribe(bluetooth_connection_callback);
  battery_state_service_subscribe(handle_battery);
}

static void deinit (void) {
  window_destroy(window);

  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
}

int main (void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
