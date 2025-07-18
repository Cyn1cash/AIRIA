#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
#include "WeatherHelper.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

class ThingsBoardHelper {
public:
    explicit ThingsBoardHelper(DisplayManager &disp, SensorHelper &sensors,
                               WeatherHelper &weather, EnergyEstimator &energy)
        : _disp(disp), _sensors(sensors), _weather(weather), _energy(energy) {}

    void begin() {
        // ThingsBoard helper initialized - no startup sequence needed
    }

    void poll() {
        if (millis() - _lastUpload >= Config::THINGSBOARD_UPLOAD_INTERVAL_MS) {
            uploadData();
        }
    }

    // Get the last successful upload timestamp
    uint32_t getLastUploadTime() const { return _lastSuccessfulUpload; }

    // Get upload status
    bool isUploadSuccessful() const { return _lastUploadSuccessful; }

    // Get last upload error message
    String getLastError() const { return _lastError; }

private:
    void uploadData() {
        _lastUpload = millis();

        // Only upload if we have valid sensor data
        if (!_sensors.isDataValid()) {
            _lastError = "Invalid sensor data";
            _lastUploadSuccessful = false;
            return;
        }

        // Create JSON payload with all collected data
        JsonDocument doc;

        // Indoor sensor data
        doc["indoor_temperature"] = _sensors.getIndoorTemp();
        doc["indoor_humidity"] = _sensors.getIndoorHumidity();

        // CO sensor data
        doc["co_analog_reading"] = _sensors.getCoAnalogReading();
        doc["co_voltage"] = _sensors.getCoVoltage();
        doc["co_ppm"] = _sensors.getCoPPM();
        doc["co_digital_reading"] = _sensors.getCoDigitalReading();
        doc["co_sensor_warmed_up"] = _sensors.isCoSensorWarmedUp();

        // Ozone sensor data
        doc["ozone_digital_reading"] = _sensors.getOzoneDigitalReading();
        doc["ozone_sensor_warmed_up"] = _sensors.isOzoneSensorWarmedUp();

        // Outdoor weather data
        doc["outdoor_temperature"] = _weather.getCurrentTemp();
        doc["outdoor_humidity"] = _weather.getCurrentHumidity();

        // Energy calculations
        doc["estimated_power_watts"] = _energy.getEstimatedPowerWatts();
        doc["daily_energy_kwh"] = _energy.getDailyEnergyKWh();
        doc["daily_cost_estimate"] = _energy.getDailyCostEstimate();
        doc["current_cop"] = _energy.getCurrentCOP();
        doc["heat_load_btu"] = _energy.getHeatLoadBTU();
        doc["current_eer"] = _energy.getEER();
        doc["duty_cycle"] = _energy.getCurrentDutyCycle();

        // Temperature differences
        doc["temp_difference"] = _sensors.getTempDifference(_weather.getCurrentTemp());
        doc["humidity_difference"] = _sensors.getHumidityDifference(_weather.getCurrentHumidity());

        // System status
        doc["timestamp"] = millis();
        doc["sensor_last_reading"] = _sensors.getLastReadingTime();

        // Convert to string
        String jsonString;
        serializeJson(doc, jsonString);

        // Send HTTP POST request
        if (sendToThingsBoard(jsonString)) {
            _lastSuccessfulUpload = millis();
            _lastUploadSuccessful = true;
            _lastError = "";

            // Update display with upload success
            _disp.showThingsBoardSuccess();
        } else {
            _lastUploadSuccessful = false;
            // Error message is set in sendToThingsBoard
            _disp.showThingsBoardError(_lastError);
        }
    }

    bool sendToThingsBoard(const String &jsonData) {
        WiFiClient client;
        HTTPClient http;

        if (!http.begin(client, Config::THINGSBOARD_URL)) {
            _lastError = "Failed to begin HTTP connection";
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        http.setTimeout(10000); // 10 second timeout

        int httpCode = http.POST(jsonData);

        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
            String response = http.getString();
            http.end();
            return true;
        } else {
            _lastError = "HTTP Error: " + String(httpCode);
            if (httpCode > 0) {
                String response = http.getString();
                if (response.length() > 0) {
                    _lastError += " - " + response.substring(0, 50); // Limit error message length
                }
            }
            http.end();
            return false;
        }
    }

    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;
    EnergyEstimator &_energy;

    uint32_t _lastUpload = 0;
    uint32_t _lastSuccessfulUpload = 0;
    bool _lastUploadSuccessful = false;
    String _lastError = "";
};
