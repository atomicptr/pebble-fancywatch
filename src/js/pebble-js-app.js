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

SECOND = 1000;
MINUTE = SECOND * 60;

API_URL = "http://api.openweathermap.org/data/2.5/weather?lat={0}&lon={1}&mode=json";

get_location_and_show_weather = function() {
	navigator.geolocation.getCurrentPosition(function(e) {
		get_weather(e.coords.longitude, e.coords.latitude);
	});
	
	setTimeout(get_location_and_show_weather, 5 * MINUTE);
};

get_weather = function(longitute, latitute) {
	var request = new XMLHttpRequest();
	
	request.open("GET", API_URL.format(longitute, latitute), true);
	
	request.onload = function(e) {
		if(request.readyState == 4 && request.status == 200) {
			var response = JSON.parse(request.responseText);
			
			console.log("ANSWER FROM SERVER: " + request.responseText);
			
			var pebble_data = {
				"icon_id": get_icon_id(response.weather[0].icon),
				"temp_kelvin": Number(Number(response.main.temp).toFixed(0)),
				"temp_celsius": Number(convert_kelvin_to_celsius(Number(response.main.temp)).toFixed(0)),
				"temp_fahrenheit": Number(convert_kelvin_to_fahrenheit(Number(response.main.temp)).toFixed(0))
			};
			
			// send date to pebble
			Pebble.sendAppMessage(pebble_data);
			console.log("Send: " + JSON.stringify(pebble_data, 4) + " to pebble");
		} else {
			console.log("ERROR: connection to weather api failed with code: " + request.status); 
		}
	};
	
	request.send(null);
};

get_icon_id = function(key) {
	switch(key) {
		// clear_day
		case "01d":
			return 0;
		// clear_night
		case "01n":
			return 1;
		// cloudy
		case "02d":
		case "02n":
		case "03d":
		case "03n":
		case "04d":
		case "04n":
			return 2;
		// shower_rain
		case "09d":
		case "09n":
			return 3;
		// rain
		case "10d":
		case "10n":
			return 4;
		case "11d":
		case "11n":
		// thunderstorm
			return 5;
		// snow
		case "13d":
		case "13n":
			return 6;
		// mist
		case "50d":
		case "50n":
			return 7;
		// error
		default:
			return 8;
		
	}
};

convert_kelvin_to_celsius = function(kelvin) {
	return kelvin - 273.15;
};

convert_kelvin_to_fahrenheit = function(kelvin) {
	return (kelvin * 1.8) - 459.67;
};

// String.format seems to be undefined in pebble
if(!String.prototype.format) {
	String.prototype.format = function() {
		var format_replace = function(match, number) {
			return typeof(arguments[number] != undefined ? arguments[number] : match);
		};
	
		return this.replace(/{(\d+)}/g, format_replace)
	};
}

Pebble.addEventListener("ready", function(e) {
	setTimeout(get_location_and_show_weather, 1000);
});
