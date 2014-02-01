#include <pebble.h>

static void init(void);
static void destroy(void);
static void window_load(Window*);
static void window_unload(Window*);
static void handle_clock_tick(struct tm*, TimeUnits);

static char* int_to_string(unsigned int);

static Window *window;
static TextLayer *time_layer;
static unsigned int counter = 0;

/** init fancy watch */
static void init(void) {
	window = window_create();

	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	const bool animated = true;
	window_stack_push(window, animated);
}

/** destroy fancy watch */
static void destroy(void) {
	window_destroy(window);
}

/** load window */
static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// setup time layer
	time_layer = text_layer_create((GRect) {
		.origin = {
			0, 5
		},
		.size = {
			bounds.size.w, 50
		}
	});
	
	text_layer_set_text(time_layer, "13:37");
	
	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_background_color(time_layer, GColorBlack);
	
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

	// add children
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	
	// call ticker
	time_t now = time(NULL);
	struct tm *tick_time;
	tick_time = localtime(&now);
	
	handle_clock_tick(tick_time, SECOND_UNIT);
	
	// subscribe to tick service
	tick_timer_service_subscribe(SECOND_UNIT, handle_clock_tick);
}

/** unload window */
static void window_unload(Window *window) {
	text_layer_destroy(time_layer);
	
	// unsubscribe services
	tick_timer_service_unsubscribe();
}

static void handle_clock_tick(struct tm *tick_time, TimeUnits units_changed) {
	int seconds = tick_time->tm_sec;
	
	char *time_string = seconds % 2 == 0 ? "%H:%M" : "%H %M";
	
	char *buffer = "00:00";
	
	strftime(buffer, sizeof("00:00"), time_string, tick_time);

	text_layer_set_text(time_layer, buffer);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing fancy watch, pushed window: %p", window);

	app_event_loop();
	destroy();
}