var SECOND = 1000;
var MINUTE = SECOND * 60;

var WEATHER_API_URL = "http://api.openweathermap.org/data/2.5/weather?lon={0}&lat={1}&mode=json&appid=fe105672f9f6cb48e2d2e049448f5021";
var FW_CONFIG_URL = "http://stuff.kasoki.de/pebble/fancywatch-config/index.html";

var PEBBLE_EVENT_NEW_WEATHER_INFO = 0;
var PEBBLE_EVENT_CONFIGURATION_CHANGED = 1;

var TIMEOUT_VAR = null;

var LAST_WEATHER_INFO = {}
var WEATHER_INFO_RECEIVED_ONCE = false;

var get_location_and_show_weather = function() {
	console.log("js: try to obtain location");

	navigator.geolocation.getCurrentPosition(function(e) {
		console.log("js: you're currently at coords: long: " +
			e.coords.longitude + ", lat: " + e.coords.latitude);

		get_weather(e.coords.longitude, e.coords.latitude);

		TIMEOUT_VAR = setTimeout(get_location_and_show_weather, 15 * MINUTE);
	}, function(error) {
		console.warn("js: error while locating user....");

		switch(error.code) {
			case error.PERMISSION_DENIED:
				console.warn("js: location permission denied");
				break;
			case error.POSITION_UNAVAILABLE:
				console.warn("js: location position unavailable");
				break;
			case error.TIMEOUT:
				console.warn("js: location timeout");
				break;
			default:
				console.warn("js: location unknown error");
				break;
		}

		if(!WEATHER_INFO_RECEIVED_ONCE) {
			// send error message
			Pebble.sendAppMessage({
				"event_type": PEBBLE_EVENT_NEW_WEATHER_INFO,
			});
		}

		console.warn("js: try to obtain location again in one minute...");

		TIMEOUT_VAR = setTimeout(get_location_and_show_weather, MINUTE);
	}, {
		"timeout": SECOND * 15
	});
};

var get_weather = function(longitude, latitude) {
	var request_url = String.format(WEATHER_API_URL, longitude, latitude);

	console.log("js: request weather info from: " + request_url);

	var request = new XMLHttpRequest();

	request.open("GET", request_url, true);

	request.onload = function(e) {
		if(request.readyState == 4 && request.status == 200) {
			var response = JSON.parse(request.responseText);

			console.log("ANSWER FROM SERVER: " + request.responseText);

			LAST_WEATHER_INFO = {
				"event_type": PEBBLE_EVENT_NEW_WEATHER_INFO,
				"icon_id": get_icon_id(response.weather[0].icon),
				"temp_kelvin": Number(Number(response.main.temp).toFixed(0)),
				"temp_celsius": Number(convert_kelvin_to_celsius(Number(response.main.temp)).toFixed(0)),
				"temp_fahrenheit": Number(convert_kelvin_to_fahrenheit(Number(response.main.temp)).toFixed(0))
			};

			WEATHER_INFO_RECEIVED_ONCE = true;

			// send date to pebble
			Pebble.sendAppMessage(LAST_WEATHER_INFO);
			console.log("Send: " + JSON.stringify(LAST_WEATHER_INFO, 4) + " to pebble");
		} else {
			console.log("ERROR: connection to weather api failed with code: " + request.status);
		}
	};

	request.send(null);
};

var get_icon_id = function(key) {
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

var convert_kelvin_to_celsius = function(kelvin) {
	return kelvin - 273.15;
};

var convert_kelvin_to_fahrenheit = function(kelvin) {
	return (kelvin * 1.8) - 459.67;
};

String.format = function(format) {
	var args = Array.prototype.slice.call(arguments, 1);

	return format.replace(/{(\d+)}/g, function(match, number) {
		return typeof args[number] != 'undefined' ? args[number] : match;
	});
};

Pebble.addEventListener("ready", function(e) {
	setTimeout(get_location_and_show_weather, 1000);
});

Pebble.addEventListener("showConfiguration", function(e) {
	console.log("FW - showing configuration");

	var options = JSON.parse(window.localStorage.getItem("fw_options"));

	console.log("received options from localstorage: " + JSON.stringify(options));

	var suffix = "";

	if(options != null) {
		suffix += "?";

		var once = false;

		for(var item in options) {
			once = true;

			suffix += item + "=" + options[item];
			suffix += "&";
		}

		if(once) {
			suffix.substring(0, suffix.length - 1);
		}
	}

	console.log("FW - open config url: " + (FW_CONFIG_URL + suffix));
	Pebble.openURL(FW_CONFIG_URL + suffix);
});

Pebble.addEventListener("webviewclosed", function(e) {
	console.log("FW - configuration closed");

	var options = JSON.parse(decodeURIComponent(e.response));

	console.log(JSON.stringify(options));

	if(options.fezzes_are == "cool") {
		var pebble_data = {
			"event_type": PEBBLE_EVENT_CONFIGURATION_CHANGED,
			"temp_metric": Number(options.temp_metric),
			"show_battery": Number(options.show_battery),
			"use_12hour": Number(options.use_12hour)
		};

		// store options in local storage
		var configs = {
			"temp_metric": Number(options.temp_metric),
			"show_battery": Number(options.show_battery),
			"use_12hour": Number(options.use_12hour)
		};

		window.localStorage.setItem("fw_options", JSON.stringify(configs));

		console.log("FW - stored options in localstorage: " + window.localStorage.getItem("fw_options"));

		// send date to pebble
		Pebble.sendAppMessage(pebble_data);
		console.log("Send: " + JSON.stringify(pebble_data, 4) + " to pebble");

		// force pebble to retrieve weather data again
		clearTimeout(TIMEOUT_VAR);

		// TODO: send cached last weather instead
		get_location_and_show_weather();
	}
});
