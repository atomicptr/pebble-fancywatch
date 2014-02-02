SECOND = 1000;
MINUTE = SECOND * 60;

API_URL = "http://api.openweathermap.org/data/2.5/weather?lat={0}&lon={1}&mode=json";

get_location_and_show_weather = function() {
	navigatior.geolocation.getCurrentPosition(function(e) {
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
			
			var pebble_data = {};
			
			pebble_data.icon_id = response.weather.icon;
			pebble_data.icon = get_icon_name_by_id(response.weather.icon);
			pebble_data.temp_kelvin = Number(response.main.temp).toFixed(0);
			pebble_data.temp_celsius = convert_kelvin_to_celsius(Number(response.main.temp)).toFixed(0);
			pebble_data.temp_fahrenheit = convert_kelvin_to_fahrenheit(Number(response.main.temp)).toFixed(0);
			
			Pebble.sendAppMessage(pebble_data);
		} else {
			console.log("ERROR: connection to weather api failed with code: " + request.status); 
		}
	};
	
	request.send(null);
};

get_icon_name_by_id = function(id) {
	switch(id) {
		case "01d":
			return "clear_day";
		case "01n":
			return "clear_night";
		case "02d":
		case "02n":
		case "03d":
		case "03n":
		case "04d":
		case "04n":
			return "cloudy";
		case "09d":
		case "09n":
			return "shower_rain";
		case "10d":
		case "10n":
			return "rain";
		case "11d":
		case "11n":
			return "thunderstorm";
		case "13d":
		case "13n":
			return "snow";
		case "50d":
		case "50n":
			return "mist";
		default:
			return "error";
		
	}
};

convert_kelvin_to_celsius = function(kelvin) {
	return kelvin - 273.15;
};

convert_kelvin_to_fahrenheit = function(kelvin) {
	return (kelvin * 1.8) 459.67;
};

Pebble.addEventListener("ready", function(e) {
	setTimeout(get_location_and_show_weather, 1000);
});
