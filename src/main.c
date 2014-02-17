// Copyright (c) 2014 Christopher "Kasoki" Kaster
//
// This file is part of pebble-fancywatch <https://github.com/kasoki/pebble-fancywatch>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <pebble.h>
#include "weather.h"

#define EVENT_TYPE 0

#define IMAGE_SIZE 55
#define NUMBER_OF_IMAGES 9

#define TIME_POS_Y 25
#define DATE_POS_Y TIME_POS_Y + 55
#define WEATHER_POS_Y DATE_POS_Y + 25

const int WEATHER_IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
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

const int BATTERY_IMAGE_RESOURCE_IDS[4] = {
	RESOURCE_ID_BATTERY_0,
	RESOURCE_ID_BATTERY_1,
	RESOURCE_ID_BATTERY_2,
	RESOURCE_ID_BATTERY_3
};

enum {
	PEBBLE_EVENT_IDENT_NEW_WEATHER_INFO = 0,
	PEBBLE_EVENT_IDENT_CONFIGURATION_CHANGED = 1
};

enum {
	CONFIGURATION_IDENT_TEMP_METRIC = 1,
	CONFIGURATION_IDENT_SHOW_BATTERY = 2
};

enum {
	PERSIST_KEY_TEMP_METRIC = 0,
	PERSIST_KEY_SHOW_BATTERY = 1,
};

enum {
	BATTERY_HIDE = 0,
	BATTERY_SHOW = 1
};

static void init(void);
static void destroy(void);
static void window_load(Window*);
static void window_unload(Window*);

static void handle_clock_tick(struct tm*, TimeUnits);
static void init_app_message(void);
static void update_battery_indicator(void);
static int get_battery_image(uint8_t);
static bool battery_state_changed(void);
static void on_received_handler(DictionaryIterator*, void*);
static void on_weather_handler_received(DictionaryIterator*, void*);
static void on_configuration_handler_received(DictionaryIterator*, void*);

static Window *window;
static TextLayer *time_layer;
static TextLayer *date_layer;
static TextLayer *temp_layer;

static BitmapLayer *weather_image_layer;
static BitmapLayer *battery_image_layer;

static GBitmap *weather_image;
static GBitmap *battery_image;

static weather_t weather;
static BatteryChargeState charge_state;

static uint8_t old_battery_percent = -1;

static int TEMPERATURE_METRIC = WEATHER_CONFIGURATION_IDENT_CELSIUS;
static int SHOW_BATTERY = BATTERY_SHOW;

/** init fancy watch */
static void init(void) {
	// retrieve data
	TEMPERATURE_METRIC = persist_exists(PERSIST_KEY_TEMP_METRIC) ?
		persist_read_int(PERSIST_KEY_TEMP_METRIC) : WEATHER_CONFIGURATION_IDENT_CELSIUS;
	SHOW_BATTERY = persist_exists(PERSIST_KEY_SHOW_BATTERY) ?
		persist_read_int(PERSIST_KEY_SHOW_BATTERY) : BATTERY_SHOW;

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

/** init app messages */
static void init_app_message(void) {
	app_message_open(64, 16);

	app_message_register_inbox_received(on_received_handler);
}

/** destroy fancy watch */
static void destroy(void) {
	// store data
	persist_write_int(PERSIST_KEY_TEMP_METRIC, TEMPERATURE_METRIC);
	persist_write_int(PERSIST_KEY_SHOW_BATTERY, SHOW_BATTERY);

	window_destroy(window);
}

/** load and setup window */
static void window_load(Window *window) {
	window_set_background_color(window, GColorBlack);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	// setup time layer
	time_layer = text_layer_create((GRect) {
		.origin = {
			0, TIME_POS_Y
		},
		.size = {
			bounds.size.w, 50
		}
	});

	text_layer_set_text_color(time_layer, GColorWhite);
	text_layer_set_background_color(time_layer, GColorClear);

	text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));

	text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);

	// setup date layer
	date_layer = text_layer_create((GRect) {
		.origin = {
			0, DATE_POS_Y
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
			bounds.size.w / 2 + 15, WEATHER_POS_Y + IMAGE_SIZE/2 - 11
		},
		.size = {
			bounds.size.w / 2, 22
		}
	});

	text_layer_set_text_color(temp_layer, GColorWhite);
	text_layer_set_background_color(temp_layer, GColorClear);

	text_layer_set_font(temp_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));

	text_layer_set_text_alignment(temp_layer, GTextAlignmentLeft);

	// add children
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	layer_add_child(window_layer, text_layer_get_layer(temp_layer));

	// call ticker
	time_t now = time(NULL);
	struct tm *tick_time;
	tick_time = localtime(&now);

	handle_clock_tick(tick_time, MINUTE_UNIT);

	// subscribe to tick service
	tick_timer_service_subscribe(MINUTE_UNIT, handle_clock_tick);
}

/** unload window */
static void window_unload(Window *window) {
	// destroy text layer
	text_layer_destroy(time_layer);
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

/** is called every tick (every second if not low energy mode) */
static void handle_clock_tick(struct tm *tick_time, TimeUnits units_changed) {
	charge_state = battery_state_service_peek();

	if(SHOW_BATTERY == BATTERY_SHOW) {
		if(battery_state_changed()) {
			update_battery_indicator();
		}
	} else {
		// destroy battery stuff
		if(battery_image != NULL) {
			gbitmap_destroy(battery_image);
			layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
			bitmap_layer_destroy(battery_image_layer);
		}
	}

	// print time
	char *buffer = "00:00";

	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);

	text_layer_set_text(time_layer, buffer);

	// print date
	char *date_string = "XXX. XX. XXX.";

	strftime(date_string, sizeof("XXX. XX. XXX."), "%a. %d. %b.", tick_time);

	text_layer_set_text(date_layer, date_string);
}

static void update_battery_indicator(void) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	uint8_t percent = charge_state.charge_percent;

	int battery_resource_id = get_battery_image(percent);

	if(battery_image != NULL) {
		gbitmap_destroy(battery_image);
		layer_remove_from_parent(bitmap_layer_get_layer(battery_image_layer));
		bitmap_layer_destroy(battery_image_layer);
	}

	battery_image = gbitmap_create_with_resource(BATTERY_IMAGE_RESOURCE_IDS[battery_resource_id]);
	battery_image_layer = bitmap_layer_create((GRect) {
		.origin = {
			bounds.size.w / 2 + 13, 0
		},
		.size = {
			bounds.size.w / 2, 42
		}
	});

	bitmap_layer_set_bitmap(battery_image_layer, battery_image);
	layer_add_child(window_layer, bitmap_layer_get_layer(battery_image_layer));
}

static int get_battery_image(uint8_t percent) {
	if(percent >= 75) {
		return 3;
	} else if(percent >= 50) {
		return 2;
	} else if(percent > 20) {
		return 1;
	} else {
		return 0;
	}
}

static bool battery_state_changed(void) {
	uint8_t percent = charge_state.charge_percent;
	bool has_changed = false;

	has_changed = get_battery_image(old_battery_percent) != get_battery_image(percent);

	old_battery_percent = percent;

	return has_changed;
}

/** message from PebbleKit JS received */
static void on_received_handler(DictionaryIterator *received, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "received information from PebbleKitJS");

	Tuple *event_type = dict_find(received, EVENT_TYPE);

	switch(event_type->value->int8) {
		case PEBBLE_EVENT_IDENT_NEW_WEATHER_INFO:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "received new weather event");
			on_weather_handler_received(received, context);
			break;
		case PEBBLE_EVENT_IDENT_CONFIGURATION_CHANGED:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "received configuration changed event");
			on_configuration_handler_received(received, context);
			break;
		default:
			APP_LOG(APP_LOG_LEVEL_DEBUG, "received invalid event id: %d", event_type->value->int8);
			break;
	}
}

static void on_weather_handler_received(DictionaryIterator *received, void *context) {
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

	int temperature = -1;

	switch(TEMPERATURE_METRIC) {
		case WEATHER_CONFIGURATION_IDENT_CELSIUS:
			temperature = weather.temp_celsius;
			break;
		case WEATHER_CONFIGURATION_IDENT_FAHRENHEIT:
			temperature = weather.temp_fahrenheit;
			break;
		case WEATHER_CONFIGURATION_IDENT_KELVIN:
			temperature = weather.temp_kelvin;
			break;
		default:
			temperature = weather.temp_celsius;
			break;
	}

	// set weather text
	char *temp_string = "XXXXX";

	snprintf(temp_string, sizeof("-1337°"), "%d°", temperature);

	text_layer_set_text(temp_layer, temp_string);

	// set weather image
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	weather_image = gbitmap_create_with_resource(WEATHER_IMAGE_RESOURCE_IDS[weather.icon_id]);
	weather_image_layer = bitmap_layer_create((GRect) {
		.origin = {
			bounds.size.w / 2 - IMAGE_SIZE - 5, WEATHER_POS_Y
		},
		.size = {
			bounds.size.w / 2, IMAGE_SIZE
		}
	});

	bitmap_layer_set_bitmap(weather_image_layer, weather_image);
	layer_add_child(window_layer, bitmap_layer_get_layer(weather_image_layer));
}

static void on_configuration_handler_received(DictionaryIterator *received, void *context) {
	Tuple *temp_metric = dict_find(received, CONFIGURATION_IDENT_TEMP_METRIC);
	Tuple *show_battery = dict_find(received, CONFIGURATION_IDENT_SHOW_BATTERY);

	TEMPERATURE_METRIC = temp_metric->value->int8;
	SHOW_BATTERY = show_battery->value->int8;
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing fancy watch, pushed window: %p", window);

	app_event_loop();
	destroy();
}
