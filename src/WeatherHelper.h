#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <math.h>

struct WeatherStation {
    String id;
    String name;
    double latitude;
    double longitude;
    double distance;
};

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

    // New methods for individual object updates
    String getOutdoorTempString() const {
        if (!isnan(_currentTemp)) {
            return "Temperature: " + String(_currentTemp, 1) + "°C";
        } else {
            return "Temperature: --.-°C";
        }
    }

    String getOutdoorRhString() const {
        if (!isnan(_currentHumidity)) {
            return "Relative Humidity: " + String((int)_currentHumidity) + "%";
        } else {
            return "Relative Humidity: --%";
        }
    }

private:
    void fetch() {
        _lastFetch = millis();

        // Fetch temperature data to get station list and find closest station
        if (fetchTemperature()) {
            // Then fetch humidity data using the same closest station
            fetchHumidity();

            // Update display with combined data
            updateDisplay();
        } else {
            // If temperature fetch fails, set fallback display
            updateDisplay();
        }
    }

    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        // Haversine formula for calculating distance between two points on Earth
        const double dlat = (lat2 - lat1) * M_PI / 180.0;
        const double dlon = (lon2 - lon1) * M_PI / 180.0;
        const double a = sin(dlat / 2) * sin(dlat / 2) +
                         cos(lat1 * M_PI / 180.0) * cos(lat2 * M_PI / 180.0) *
                             sin(dlon / 2) * sin(dlon / 2);
        const double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return Config::EARTH_RADIUS_KM * c;
    }

    String findClosestStation(const JsonArray &stations) {
        String closestStationId = "";
        double minDistance = Config::MAX_DISTANCE_SEARCH;

        for (JsonVariant station : stations) {
            // NEA APIs sometimes use "location" and sometimes "labelLocation"
            double stationLat, stationLon;

            if (station["labelLocation"]) {
                stationLat = station["labelLocation"]["latitude"];
                stationLon = station["labelLocation"]["longitude"];
            } else if (station["location"]) {
                stationLat = station["location"]["latitude"];
                stationLon = station["location"]["longitude"];
            } else {
                continue; // Skip if no location data
            }

            double distance = calculateDistance(Config::LATITUDE, Config::LONGITUDE,
                                                stationLat, stationLon);

            if (distance < minDistance) {
                minDistance = distance;
                closestStationId = station["id"].as<String>();
                _closestStationName = station["name"].as<String>();
                _closestStationDistance = distance;
            }
        }

        return closestStationId;
    }

    float getStationValue(const JsonArray &readings, const String &stationId) {
        // First try to get data from the preferred station
        for (JsonVariant reading : readings) {
            JsonArray data = reading["data"];
            for (JsonVariant dataPoint : data) {
                if (dataPoint["stationId"].as<String>() == stationId) {
                    float value = dataPoint["value"].as<float>();
                    if (!isnan(value)) {
                        return value;
                    }
                }
            }
        }

        // If preferred station has no data, try any available station
        for (JsonVariant reading : readings) {
            JsonArray data = reading["data"];
            for (JsonVariant dataPoint : data) {
                float value = dataPoint["value"].as<float>();
                if (!isnan(value)) {
                    return value;
                }
            }
        }

        return NAN;
    }

    bool fetchTemperature() {
        WiFiClientSecure secureClient;
        secureClient.setInsecure(); // Skip certificate validation for simplicity
        HTTPClient http;

        if (!http.begin(secureClient, Config::NEA_TEMP_API)) {
            return false;
        }

        http.addHeader("Accept", "application/json");
        http.setTimeout(Config::HTTP_TIMEOUT_MS); // HTTP timeout for HTTPS

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            http.end();

            DynamicJsonDocument doc(Config::JSON_DOC_SIZE);
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["code"] == 0) {
                JsonArray stations = doc["data"]["stations"];
                JsonArray readings = doc["data"]["readings"];

                if (stations.size() > 0 && readings.size() > 0) {
                    _closestStationId = findClosestStation(stations);
                    _currentTemp = getStationValue(readings, _closestStationId);
                    return !isnan(_currentTemp);
                }
            }
        }

        http.end();
        return false;
    }

    bool fetchHumidity() {
        if (_closestStationId.isEmpty()) return false;

        WiFiClientSecure secureClient;
        secureClient.setInsecure(); // Skip certificate validation
        HTTPClient http;

        if (!http.begin(secureClient, Config::NEA_HUMIDITY_API)) {
            return false;
        }

        http.addHeader("Accept", "application/json");
        http.setTimeout(Config::HTTP_TIMEOUT_MS);

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            http.end();

            DynamicJsonDocument doc(Config::JSON_DOC_SIZE);
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["code"] == 0) {
                JsonArray readings = doc["data"]["readings"];
                _currentHumidity = getStationValue(readings, _closestStationId);
                return !isnan(_currentHumidity);
            }
        }

        http.end();
        return false;
    }

    void updateDisplay() {
        // Update individual objects for new frontend
        _disp.updateOutdoorTemp(getOutdoorTempString());
        _disp.updateOutdoorRh(getOutdoorRhString());
    }

    DisplayManager &_disp;
    uint32_t _lastFetch = 0;
    float _currentTemp = NAN;
    float _currentHumidity = NAN;
    String _closestStationId = "";
    String _closestStationName = "";
    double _closestStationDistance = 0.0;
};
