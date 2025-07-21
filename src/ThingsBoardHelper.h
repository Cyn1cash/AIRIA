#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
#include "WeatherHelper.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// ThingsBoard error messages
namespace ThingsBoardErrors {
    constexpr char MQTT_NOT_CONNECTED[] = "MQTT not connected";
    constexpr char MQTT_PUBLISH_FAILED[] = "MQTT publish failed";
    constexpr char MQTT_CONNECTION_FAILED[] = "MQTT connection failed, rc=";
    constexpr char INVALID_SENSOR_DATA[] = "Invalid sensor data";
}

class ThingsBoardHelper {
public:
    explicit ThingsBoardHelper(DisplayManager &disp, SensorHelper &sensors,
                               WeatherHelper &weather, EnergyEstimator &energy)
        : _wifiClient(), _mqttClient(_wifiClient), _disp(disp), _sensors(sensors),
          _weather(weather), _energy(energy) {}

    void begin() {
        _mqttClient.setServer(Config::THINGSBOARD_MQTT_SERVER, Config::THINGSBOARD_MQTT_PORT);
        _mqttClient.setBufferSize(1024); // Increase buffer size for larger JSON payloads
        _lastReconnectAttempt = 0;
    }

    void poll() {
        // Handle MQTT connection
        if (!_mqttClient.connected()) {
            if (millis() - _lastReconnectAttempt >= Config::MQTT_RECONNECT_DELAY_MS) {
                _lastReconnectAttempt = millis();
                if (reconnect()) {
                    _lastReconnectAttempt = 0;
                }
            }
        } else {
            _mqttClient.loop();
        }

        // Upload data at regular intervals
        if (_mqttClient.connected() &&
            millis() - _lastUpload >= Config::THINGSBOARD_UPLOAD_INTERVAL_MS) {
            uploadData();
        }
    }

    // Get the last successful upload timestamp
    uint32_t getLastUploadTime() const { return _lastSuccessfulUpload; }

    // Get upload status
    bool isUploadSuccessful() const { return _lastUploadSuccessful; }

    // Get last upload error message
    String getLastError() const { return _lastError; }

    // Get MQTT connection status
    bool isMqttConnected() { return _mqttClient.connected(); }

private:
    bool reconnect() {
        if (_mqttClient.connect(Config::THINGSBOARD_CLIENT_ID, Config::THINGSBOARD_USERNAME, Config::THINGSBOARD_PASSWORD)) {
            _lastError = "";
            return true;
        } else {
            _lastError = String(ThingsBoardErrors::MQTT_CONNECTION_FAILED) + String(_mqttClient.state());
            return false;
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

        // Convert to string with proper formatting
        String jsonString;
        serializeJson(doc, jsonString);

        Serial.println("Sending JSON: " + jsonString);
        Serial.print("JSON length: ");
        Serial.println(jsonString.length());
        Serial.print("MQTT connected: ");
        Serial.println(_mqttClient.connected() ? "Yes" : "No");

        // Publish to ThingsBoard via MQTT
        if (publishToThingsBoard(jsonString)) {
            _lastSuccessfulUpload = millis();
            _lastUploadSuccessful = true;
            _lastError = "";

            // Update display with upload success
            _disp.showThingsBoardSuccess();
        } else {
            _lastUploadSuccessful = false;
            // Error message is set in publishToThingsBoard
            _disp.showThingsBoardError(_lastError);
        }
    }

    bool publishToThingsBoard(const String &jsonData) {
        if (!_mqttClient.connected()) {
            _lastError = ThingsBoardErrors::MQTT_NOT_CONNECTED;
            Serial.println("ERROR: MQTT not connected!");
            return false;
        }

        Serial.print("Publishing to topic: ");
        Serial.println(Config::THINGSBOARD_TELEMETRY_TOPIC);

        // Use QoS 0 for better reliability during testing
        bool result = _mqttClient.publish(Config::THINGSBOARD_TELEMETRY_TOPIC, jsonData.c_str(), false);

        Serial.print("Publish result: ");
        Serial.println(result ? "SUCCESS" : "FAILED");

        if (result) {
            return true;
        } else {
            _lastError = ThingsBoardErrors::MQTT_PUBLISH_FAILED;
            return false;
        }
    }

    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;
    EnergyEstimator &_energy;

    uint32_t _lastUpload = 0;
    uint32_t _lastSuccessfulUpload = 0;
    uint32_t _lastReconnectAttempt = 0;
    bool _lastUploadSuccessful = false;
    String _lastError = "";
};
