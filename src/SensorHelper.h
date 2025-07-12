#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <DHT.h>
#include <DHT_U.h>

class SensorHelper {
public:
    explicit SensorHelper(DisplayManager &disp) : _disp(disp), _dht(Config::DHT22_PIN, DHT22) {}

    void begin() {
        _dht.begin();
        Serial.println("DHT22 sensor initialized");
        // Wait for sensor to stabilize
        delay(2000);
        // Take initial reading
        readSensors();
    }

    void poll() {
        if (millis() - _lastReading >= Config::SENSOR_REFRESH_MS) {
            readSensors();
        }
    }

    // Getters for sensor values
    float getIndoorTemp() const { return _indoorTemp; }
    float getIndoorHumidity() const { return _indoorHumidity; }
    bool isDataValid() const { return _dataValid; }

    // Get the last successful reading timestamp
    uint32_t getLastReadingTime() const { return _lastValidReading; }

    // Calculate temperature difference (indoor - outdoor)
    float getTempDifference(float outdoorTemp) const {
        if (!_dataValid) return NAN;
        return _indoorTemp - outdoorTemp;
    }

    // Calculate humidity difference (indoor - outdoor)
    float getHumidityDifference(float outdoorHumidity) const {
        if (!_dataValid) return NAN;
        return _indoorHumidity - outdoorHumidity;
    }

    // Get formatted sensor data string for display
    String getFormattedData() const {
        if (!_dataValid) {
            return "Sensor Error";
        }
        return String(_indoorTemp, 1) + "°C  " + String((int)_indoorHumidity) + "%  •  Indoor";
    }

private:
    void readSensors() {
        _lastReading = millis();

        // Read temperature and humidity
        float temp = _dht.readTemperature();
        float humidity = _dht.readHumidity();

        // Check if readings are valid
        if (isnan(temp) || isnan(humidity)) {
            _dataValid = false;
            _failedReadings++;

            Serial.println("DHT22 reading failed!");

            // If we have too many failed readings, try to reinitialize
            if (_failedReadings >= Config::MAX_SENSOR_FAILURES) {
                Serial.println("Too many DHT22 failures, reinitializing...");
                _dht.begin();
                _failedReadings = 0;
            }
            return;
        }

        // Valid readings
        _indoorTemp = temp;
        _indoorHumidity = humidity;
        _dataValid = true;
        _failedReadings = 0;
        _lastValidReading = millis();

        // Update display with indoor sensor data
        _disp.updateIndoorSensors(getFormattedData());

        Serial.println("DHT22 - Temp: " + String(_indoorTemp, 2) + "°C, Humidity: " + String(_indoorHumidity, 2) + "%");
    }

    DisplayManager &_disp;
    DHT _dht;

    uint32_t _lastReading = 0;
    uint32_t _lastValidReading = 0;

    float _indoorTemp = 0.0;
    float _indoorHumidity = 0.0;
    bool _dataValid = false;

    uint8_t _failedReadings = 0;
};
