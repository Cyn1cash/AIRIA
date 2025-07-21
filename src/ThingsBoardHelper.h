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
          _weather(weather), _energy(energy), _currentChunk(0), _lastChunkTime(0) {}

    void begin() {
        _httpClient.setTimeout(Config::HTTP_TIMEOUT_MS);
        _lastUpload = 0;
        _lastSuccessfulUpload = 0;
        _currentChunk = 0;
        _lastChunkTime = 0;
    }

    void poll() {
        if (Config::THINGSBOARD_USE_CHUNKED_UPLOAD) {
            // Handle chunked uploads
            if (_currentChunk == 0) {
                // Start new upload cycle
                if (millis() - _lastUpload >= Config::THINGSBOARD_UPLOAD_INTERVAL_MS) {
                    uploadDataChunked();
                }
            } else {
                // Continue with next chunk
                if (millis() - _lastChunkTime >= Config::THINGSBOARD_CHUNK_DELAY_MS) {
                    uploadDataChunked();
                }
            }
        } else {
            // Original single upload method
            if (millis() - _lastUpload >= Config::THINGSBOARD_UPLOAD_INTERVAL_MS) {
                uploadData();
            }
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

    // Test method with minimal JSON
    void testMinimalUpload() {
        String testJson = "{\"temperature\":25.0}";
        Serial.println("Testing minimal JSON: " + testJson);
        if (sendHttpTelemetry(testJson)) {
            Serial.println("Minimal test: SUCCESS");
        } else {
            Serial.println("Minimal test: FAILED - " + _lastError);
        }
    }

    // Force upload now (useful for testing)
    void forceUpload() {
        if (Config::THINGSBOARD_USE_CHUNKED_UPLOAD) {
            _currentChunk = 0;
            _lastUpload = 0;
        } else {
            _lastUpload = 0;
        }
    }

    // Get current chunk info (for debugging)
    uint8_t getCurrentChunk() const { return _currentChunk; }

private:
    void uploadDataChunked() {
        _lastChunkTime = millis();

        // Only upload if we have valid sensor data
        if (!_sensors.isDataValid()) {
            _lastError = ThingsBoardErrors::INVALID_SENSOR_DATA;
            _lastUploadSuccessful = false;
            _currentChunk = 0; // Reset chunk counter
            return;
        }

        // Check if WiFi is connected
        if (WiFi.status() != WL_CONNECTED) {
            _lastError = ThingsBoardErrors::HTTP_CONNECTION_FAILED;
            _lastUploadSuccessful = false;
            _currentChunk = 0; // Reset chunk counter
            return;
        }

        JsonDocument doc;
        String chunkName;

        switch (_currentChunk) {
        case 0: // Environmental data chunk
            chunkName = "Environmental Data";
            doc["indoor_temperature"] = _sensors.getIndoorTemp();
            doc["indoor_humidity"] = _sensors.getIndoorHumidity();
            doc["outdoor_temperature"] = _weather.getCurrentTemp();
            doc["outdoor_humidity"] = _weather.getCurrentHumidity();
            doc["temp_difference"] = _sensors.getTempDifference(_weather.getCurrentTemp());
            doc["humidity_difference"] = _sensors.getHumidityDifference(_weather.getCurrentHumidity());
            break;

        case 1: // Air quality data chunk
            chunkName = "Air Quality Data";
            doc["co_analog_reading"] = _sensors.getCoAnalogReading();
            doc["co_voltage"] = _sensors.getCoVoltage();
            doc["co_ppm"] = _sensors.getCoPPM();
            doc["co_digital_reading"] = _sensors.getCoDigitalReading();
            doc["co_sensor_warmed_up"] = _sensors.isCoSensorWarmedUp();
            doc["ozone_digital_reading"] = _sensors.getOzoneDigitalReading();
            doc["ozone_sensor_warmed_up"] = _sensors.isOzoneSensorWarmedUp();
            break;

        case 2: // Energy data chunk
            chunkName = "Energy Data";
            doc["estimated_power_watts"] = _energy.getEstimatedPowerWatts();
            doc["daily_energy_kwh"] = _energy.getDailyEnergyKWh();
            doc["daily_cost_estimate"] = _energy.getDailyCostEstimate();
            doc["current_cop"] = _energy.getCurrentCOP();
            doc["heat_load_btu"] = _energy.getHeatLoadBTU();
            doc["current_eer"] = _energy.getEER();
            doc["duty_cycle"] = _energy.getCurrentDutyCycle();
            break;

        case 3: // System status chunk
            chunkName = "System Status";
            doc["timestamp"] = millis();
            doc["sensor_last_reading"] = _sensors.getLastReadingTime();
            doc["chunk_sequence"] = _currentChunk;
            doc["upload_cycle"] = _lastUpload;
            break;
        }

        // Convert to string
        String jsonString;
        serializeJson(doc, jsonString);

        Serial.println("Sending " + chunkName + " (Chunk " + String(_currentChunk) + "): " + jsonString);
        Serial.print("JSON length: ");
        Serial.println(jsonString.length());

        // Send chunk
        bool success = sendHttpTelemetry(jsonString);

        if (success) {
            _currentChunk++;
            if (_currentChunk >= 4) {
                // All chunks sent successfully
                _currentChunk = 0;
                _lastUpload = millis();
                _lastSuccessfulUpload = millis();
                _lastUploadSuccessful = true;
                _lastError = "";
                _disp.showThingsBoardSuccess();
                Serial.println("All chunks uploaded successfully!");
            }
        } else {
            // Chunk failed, reset and try again next cycle
            _currentChunk = 0;
            _lastUploadSuccessful = false;
            _disp.showThingsBoardError(_lastError);
            Serial.println("Chunk upload failed, resetting cycle");
        }
    }

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

        // Create JSON payload with all data (original single upload method)
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

        _httpClient.begin(_wifiClient, url);
        _httpClient.addHeader("Content-Type", "application/json");

        int httpResponseCode = _httpClient.POST(jsonData);

        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);

        if (httpResponseCode > 0) {
            String response = _httpClient.getString();
            Serial.println("HTTP Response Body: " + response); // Always show response for debugging

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
    WiFiClient _wifiClient;
    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;
    EnergyEstimator &_energy;

    uint32_t _lastUpload = 0;
    uint32_t _lastSuccessfulUpload = 0;
    bool _lastUploadSuccessful = false;
    String _lastError = "";

    // Chunked upload state
    uint8_t _currentChunk = 0;
    uint32_t _lastChunkTime = 0;
};
