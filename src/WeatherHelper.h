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
            }
        }
        http.end();
    }

    static const char *decodeWeather(int code) {
        if (code == 0)
            return "Clear";
        if (code < 3)
            return "Partly cloudy";
        if (code < 45)
            return "Cloudy";
        if (code < 57)
            return "Fog";
        if (code < 67)
            return "Drizzle";
        if (code < 77)
            return "Rain";
        if (code < 87)
            return "Snow";
        return "Thunder";
    }

    DisplayManager &_disp;
    uint32_t _lastFetch = 0;
};
