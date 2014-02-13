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

#ifndef __FW_WEATHER_H__
#define __FW_WEATHER_H__

enum {
	WEATHER_MESSAGE_ICON_ID = 1,
	WEATHER_MESSAGE_TEMP_KELVIN = 2,
	WEATHER_MESSAGE_TEMP_CELSIUS = 3,
	WEATHER_MESSAGE_TEMP_FAHRENHEIT = 4
};

enum {
	WEATHER_CONFIGURATION_IDENT_CELSIUS = 0,
	WEATHER_CONFIGURATION_IDENT_FAHRENHEIT = 1,
	WEATHER_CONFIGURATION_IDENT_KELVIN = 2
};

enum {
	WEATHER_ICON_CLEAR_DAY,
	WEATHER_ICON_CLEAR_NIGHT,
	WEATHER_ICON_CLOUDY,
	WEATHER_ICON_SHOWER_RAIN,
	WEATHER_ICON_RAIN,
	WEATHER_ICON_THUNDERSTORM,
	WEATHER_ICON_SNOW,
	WEATHER_ICON_MIST,
	WEATHER_ICON_ERROR
};

typedef struct {
	int icon_id;
	int temp_kelvin;
	int temp_celsius;
	int temp_fahrenheit;
} weather_t;

#endif