#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_UTC_layer, *s_date_layer;




static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  struct tm *tick_timeUTC = gmtime(&temp);
  

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);


//Do the same for UTC Time
  static char s_bufferUTC[9];
  strftime(s_bufferUTC, sizeof(s_bufferUTC),"%H:%M", tick_timeUTC);
  
//Get the date
  static char date_buffer[16];
strftime(date_buffer, sizeof(date_buffer), "%a-%d-%b", tick_time);

  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
 
  // Add the UTC label
  static char UTCTimeLabel[13] = "UTC: ";
   UTCTimeLabel[5]= '\0';
  strcat(UTCTimeLabel, s_bufferUTC);
 text_layer_set_text(s_UTC_layer, UTCTimeLabel);
  
  //Write the date
  text_layer_set_text(s_date_layer, date_buffer);
}

static void update_face(GColor TextColor, GColor BackgroundColor) {
  //Set Colors  
  //GColor TextColor;
  //GColor BackgroundColor;
    //TextColor = GColorWhite;
   // BackgroundColor= GColorBlack;
  
  //Set Background Color
  window_set_background_color(s_main_window, BackgroundColor);
  
 

  // Improve the layout to be more like a watchface for local time
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, TextColor);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Improve the layout to be more like a watchface for UTC time
  text_layer_set_background_color(s_UTC_layer, GColorClear);
  text_layer_set_text_color(s_UTC_layer, TextColor);
  text_layer_set_font(s_UTC_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_UTC_layer, GTextAlignmentCenter);
  
  // Create date TextLayer

text_layer_set_text_color(s_date_layer, TextColor);
text_layer_set_background_color(s_date_layer, GColorClear);
text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));

  
  
}

static void bluetooth_callback(bool connected) {
  // Invert if disconnected
  update_face(GColorWhite, GColorBlack);
  
  if(!connected) {
    vibes_double_pulse();
    update_face(GColorBlack, GColorWhite);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

   // Create the TextLayer with specific bounds for local Time
  s_time_layer = text_layer_create(GRect(0, 40, bounds.size.w, 50));
  
    // Create the TextLayer with specific bounds for UTC Time
  s_UTC_layer = text_layer_create(GRect(0, 143, bounds.size.w, 50));
  
  //Create the Date Layer
  s_date_layer = text_layer_create(GRect(0, 85, bounds.size.w, 30));
  
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_UTC_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  update_face( GColorWhite, GColorBlack);
  
  //Update Bluetooth
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}


static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_UTC_layer);
  text_layer_destroy(s_date_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Register Bluetooth Service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}