#include <pebble.h>
#include "weather.h"

#define IMAGE_SIZE 55
#define NUMBER_OF_IMAGES 9

const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
	RESOURCE_ID_CLEAR_DAY,
	RESOURCE_ID_CLEAR_NIGHT,
	RESOURCE_ID_CLOUDY,
	RESOURCE_ID_SHOWER_RAIN,
	RESOURCE_ID_RAIN,
	RESOURCE_ID_THUNDERSTORM,
	RESOURCE_ID_SNOW,
	RESOURCE_ID_MIST,
	RESOURCE_ID_ERROR
};

static void init(void);
static void destroy(void);
static void window_load(Window*);
static void window_unload(Window*);

static void handle_clock_tick(struct tm*, TimeUnits);
static void init_app_message(void);
static void on_received_handler(DictionaryIterator*, void*);

static Window *window;
static TextLayer *time_layer;
static TextLayer *delimiter_layer;
static TextLayer *date_layer;
static TextLayer *temp_layer;

static BitmapLayer *weather_image_layer;

static GBitmap *weather_image;

static weather_t weather;

/** init fancy watch */
static void init(void) {
	window = window_create();

	// init weather struct
	weather = (weather_t) {
		.icon_id = WEATHER_ICON_ERROR,
		.temp_kelvin = 0,
		.temp_celsius = 0,
		.temp_fahrenheit = 0
	};

	init_app_message();

	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

	const bool animated = true;
	window_stack_push(window, animated);
}

static void init_app_message(void) {
	app_message_open(64, 16);
	
	app_message_register_inbox_received(on_received_handler);
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
	
	text_layer_set_text_color(delimiter_layer, GColorWhite);
	text_layer_set_background_color(delimiter_layer, GColorClear);
	
	text_layer_set_font(delimiter_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	
	text_layer_set_text_alignment(delimiter_layer, GTextAlignmentCenter);

	// setup date layer
	date_layer = text_layer_create((GRect) {
		.origin = {
			0, 60
		},
		.size = {
			bounds.size.w, 22
		}
	});
	
	text_layer_set_text_color(date_layer, GColorWhite);
	text_layer_set_background_color(date_layer, GColorClear);
	
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);

	// setup temp layer
	temp_layer = text_layer_create((GRect) {
		.origin = {
			IMAGE_SIZE/2 + 15, 85 + IMAGE_SIZE/2 - 11
		},
		.size = {
			bounds.size.w - IMAGE_SIZE/2, 22
		}
	});

	text_layer_set_text(temp_layer, "...");
	
	text_layer_set_text_color(temp_layer, GColorWhite);
	text_layer_set_background_color(temp_layer, GColorClear);
	
	text_layer_set_font(temp_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	
	text_layer_set_text_alignment(temp_layer, GTextAlignmentCenter);

	// add children
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(delimiter_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	layer_add_child(window_layer, text_layer_get_layer(temp_layer));
	
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
	// destroy text layer
	text_layer_destroy(time_layer);
	text_layer_destroy(delimiter_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(temp_layer);
	
	// destroy images
	gbitmap_destroy(weather_image);
	
	// destroy bitmap layer
	bitmap_layer_destroy(weather_image_layer);
	
	// unsubscribe services
	tick_timer_service_unsubscribe();
	app_message_deregister_callbacks();
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
	
	// print date
	char *date_string = "XXX. XX. XXX.";
	
	strftime(date_string, sizeof("XXX. XX. XXX."), "%a. %d. %b.", tick_time);

	text_layer_set_text(date_layer, date_string);
}

static void on_received_handler(DictionaryIterator *received, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "received information from PebbleKitJS");

	Tuple *icon_id = dict_find(received, WEATHER_MESSAGE_ICON_ID);
	Tuple *temp_kelvin = dict_find(received, WEATHER_MESSAGE_TEMP_KELVIN);
	Tuple *temp_celsius = dict_find(received, WEATHER_MESSAGE_TEMP_CELSIUS);
	Tuple *temp_fahrenheit = dict_find(received, WEATHER_MESSAGE_TEMP_FAHRENHEIT);
	
	// update weather
	if(icon_id && temp_kelvin && temp_celsius && temp_fahrenheit) {
		weather.icon_id = icon_id->value->int8;
		weather.temp_kelvin = temp_kelvin->value->int8;
		weather.temp_celsius = temp_celsius->value->int8;
		weather.temp_fahrenheit = temp_fahrenheit->value->int8;
	} else {
		weather.icon_id = WEATHER_ICON_ERROR;
		weather.temp_kelvin = 0;
		weather.temp_celsius = 0;
		weather.temp_fahrenheit = 0;
	}
	
	if(weather_image != NULL) {
		gbitmap_destroy(weather_image);
		layer_remove_from_parent(bitmap_layer_get_layer(weather_image_layer));
		bitmap_layer_destroy(weather_image_layer);
	}
	
	// TODO: add option to select metric
	int temperature = weather.temp_kelvin;
	
	// set weather text
	char *temp_string = "XXXXX";
	
	snprintf(temp_string, sizeof("-1337°"), "%d°", temperature);
	
	text_layer_set_text(temp_layer, temp_string);
	
	// set weather image
	Layer *window_layer = window_get_root_layer(window);
	
	weather_image = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[weather.icon_id]);
	weather_image_layer = bitmap_layer_create(GRect(15, 85, IMAGE_SIZE, IMAGE_SIZE));
	
	bitmap_layer_set_bitmap(weather_image_layer, weather_image);
	layer_add_child(window_layer, bitmap_layer_get_layer(weather_image_layer));
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing fancy watch, pushed window: %p", window);

	app_event_loop();
	destroy();
}
