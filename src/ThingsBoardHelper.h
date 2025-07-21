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

// ThingsBoard error messages
namespace ThingsBoardErrors {
    constexpr char HTTP_CONNECTION_FAILED[] = "HTTP connection failed";
    constexpr char HTTP_REQUEST_FAILED[] = "HTTP request failed";
    constexpr char INVALID_SENSOR_DATA[] = "Invalid sensor data";
    constexpr char JSON_SERIALIZATION_FAILED[] = "JSON serialization failed";
}

class ThingsBoardHelper {
public:
    explicit ThingsBoardHelper(DisplayManager &disp, SensorHelper &sensors,
                               WeatherHelper &weather, EnergyEstimator &energy)
        : _httpClient(), _disp(disp), _sensors(sensors),
          _weather(weather), _energy(energy) {}

    void begin() {
        _httpClient.setTimeout(Config::HTTP_TIMEOUT_MS);
        _lastUpload = 0;
        _lastSuccessfulUpload = 0;
    }

    void poll() {
        // Upload data at regular intervals
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

    // Get connection status (for HTTP, always true if WiFi is connected)
    bool isConnected() { return WiFi.status() == WL_CONNECTED; }

private:
    void uploadData() {
        _lastUpload = millis();

        // Only upload if we have valid sensor data
        if (!_sensors.isDataValid()) {
            _lastError = ThingsBoardErrors::INVALID_SENSOR_DATA;
            _lastUploadSuccessful = false;
            return;
        }

        // Check if WiFi is connected
        if (WiFi.status() != WL_CONNECTED) {
            _lastError = ThingsBoardErrors::HTTP_CONNECTION_FAILED;
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

        Serial.println("Sending JSON via HTTP: " + jsonString);
        Serial.print("JSON length: ");
        Serial.println(jsonString.length());

        // Send via HTTP to ThingsBoard
        if (sendHttpTelemetry(jsonString)) {
            _lastSuccessfulUpload = millis();
            _lastUploadSuccessful = true;
            _lastError = "";

            // Update display with upload success
            _disp.showThingsBoardSuccess();
        } else {
            _lastUploadSuccessful = false;
            // Error message is set in sendHttpTelemetry
            _disp.showThingsBoardError(_lastError);
        }
    }

    bool sendHttpTelemetry(const String &jsonData) {
        // Use the complete URL from configuration
        String url = String(Config::THINGSBOARD_HTTP_URL);

        Serial.println("HTTP URL: " + url);

        _httpClient.begin(url);
        _httpClient.addHeader("Content-Type", "application/json");

        int httpResponseCode = _httpClient.POST(jsonData);

        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = _httpClient.getString();
            if (response.length() > 0) {
                Serial.println("HTTP Response: " + response);
            }

            _httpClient.end();

            if (httpResponseCode == 200) {
                return true;
            } else {
                _lastError = "HTTP " + String(httpResponseCode) + ": " + response;
                return false;
            }
        } else {
            _lastError = ThingsBoardErrors::HTTP_REQUEST_FAILED + String(" (") + String(httpResponseCode) + ")";
            _httpClient.end();
            return false;
        }
    }

    HTTPClient _httpClient;
    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;
    EnergyEstimator &_energy;

    uint32_t _lastUpload = 0;
    uint32_t _lastSuccessfulUpload = 0;
    bool _lastUploadSuccessful = false;
    String _lastError = "";
};
