#include <pebble.h>

static void init(void);
static void destroy(void);
static void window_load(Window*);
static void window_unload(Window*);
static void handle_clock_tick(struct tm*, TimeUnits);

static Window *window;
static TextLayer *time_layer;
static TextLayer *delimiter_layer;

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
	text_layer_set_background_color(time_layer, GColorClear);
	
	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	
	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

	// setup time delimiter
	delimiter_layer = text_layer_create((GRect) {
		.origin = {
			bounds.size.w / 2 - 10, 5
		},
		.size = {
			20, 50
		}
	});

	text_layer_set_text(delimiter_layer, ":");
	
	text_layer_set_text_color(delimiter_layer, GColorWhite);
	text_layer_set_background_color(delimiter_layer, GColorClear);
	
	text_layer_set_font(delimiter_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	
	text_layer_set_text_alignment(delimiter_layer, GTextAlignmentCenter);

	// add children
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(delimiter_layer));
	
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
	text_layer_destroy(delimiter_layer);
	
	// unsubscribe services
	tick_timer_service_unsubscribe();
}

/** called every second */
static void handle_clock_tick(struct tm *tick_time, TimeUnits units_changed) {
	// handle delimiter animation
	int seconds = tick_time->tm_sec;
	
	char *delimiter_string = seconds % 2 == 0 ? " " : ":";
	
	text_layer_set_text(delimiter_layer, delimiter_string);
	
	// print time
	char *buffer = "00:00";
	
	strftime(buffer, sizeof("00:00"), "%H %M", tick_time);

	text_layer_set_text(time_layer, buffer);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing fancy watch, pushed window: %p", window);

	app_event_loop();
	destroy();
}
