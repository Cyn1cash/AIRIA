#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

class WeatherHelper {
public:
    explicit WeatherHelper(DisplayManager &disp) : _disp(disp) {}

    void begin() {
        _disp.showLocation(Config::LATITUDE, Config::LONGITUDE);
        fetch();
    }

    void poll() {
        if (millis() - _lastFetch >= Config::WEATHER_REFRESH_MS)
            fetch();
    }

    float getCurrentTemp() const { return _currentTemp; }
    float getCurrentHumidity() const { return _currentHumidity; }
    String getCurrentWeather() const { return _currentWeather; }

private:
    void fetch() {
        _lastFetch = millis();

        const String url =
            String("http://api.open-meteo.com/v1/forecast?") + "latitude=" + String(Config::LATITUDE, 4) + "&longitude=" + String(Config::LONGITUDE, 4) + "&current_weather=true" + "&hourly=relative_humidity_2m" + "&timezone=Asia%2FSingapore";

        WiFiClient client;
        HTTPClient http;
        if (!http.begin(client, url))
            return;

        if (http.GET() == HTTP_CODE_OK) {
            DynamicJsonDocument doc(4096);
            if (!deserializeJson(doc, http.getString())) {
                /* -------- current temperature + weather code -------- */
                float temp = doc["current_weather"]["temperature"]; // °C
                int wCode = doc["current_weather"]["weathercode"];  // 0-99
                const char *cTime = doc["current_weather"]["time"];

                /* -------- find matching RH in the hourly arrays ----- */
                float rh = NAN;
                JsonArray times = doc["hourly"]["time"];
                JsonArray hums = doc["hourly"]["relative_humidity_2m"];
                for (size_t i = 0; i < times.size() && i < hums.size(); ++i) {
                    if (strcmp(times[i], cTime) == 0) {
                        rh = hums[i];
                        break;
                    }
                }
                if (isnan(rh) && hums.size())
                    rh = hums[0]; // fallback

                /* ---------------- push to the display --------------- */
                String line =
                    String(temp, 1) + "°C  " + String((int)rh) + "%  •  " + decodeWeather(wCode);

                _disp.updateWeather(line);

                _currentTemp = temp;
                _currentHumidity = rh;
                _currentWeather = decodeWeather(wCode);
            }
        }
        http.end();
    }

    const char *decodeWeather(int code) {
        switch (code) {
        case 0:
            return "Clear sky";
        case 1:
            return "Mainly clear";
        case 2:
            return "Partly cloudy";
        case 3:
            return "Overcast";
        case 45:
            return "Fog";
        case 48:
            return "Depositing rime fog";
        case 51:
            return "Drizzle: Light";
        case 53:
            return "Drizzle: Moderate";
        case 55:
            return "Drizzle: Dense";
        case 56:
            return "Freezing Drizzle: Light";
        case 57:
            return "Freezing Drizzle: Dense";
        case 61:
            return "Rain: Slight";
        case 63:
            return "Rain: Moderate";
        case 65:
            return "Rain: Heavy";
        case 66:
            return "Freezing Rain: Light";
        case 67:
            return "Freezing Rain: Heavy";
        case 71:
            return "Snowfall: Slight";
        case 73:
            return "Snowfall: Moderate";
        case 75:
            return "Snowfall: Heavy";
        case 77:
            return "Snow grains";
        case 80:
            return "Rain showers: Slight";
        case 81:
            return "Rain showers: Moderate";
        case 82:
            return "Rain showers: Violent";
        case 85:
            return "Snow showers: Slight";
        case 86:
            return "Snow showers: Heavy";
        case 95:
            return "Thunderstorm: Slight";
        case 96:
            return "Thunderstorm with slight hail";
        case 99:
            return "Thunderstorm with heavy hail";
        default:
            return "Unknown";
        }
    }
    DisplayManager &_disp;
    uint32_t _lastFetch = 0;
    float _currentTemp = 0.0;
    float _currentHumidity = 0.0;
    String _currentWeather = "";
};
