#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <DHT.h>

class SensorHelper {
public:
    explicit SensorHelper(DisplayManager &disp) : _disp(disp), _dht(Config::DHT22_PIN, DHT22) {}

    void begin() {
        _disp.showDhtInitializing();
        _dht.begin();

        // Initialize CO sensor pins
        pinMode(Config::MQ9_DIGITAL_PIN, INPUT);
        // A0 is analog input by default, no need to set pinMode

        // Initialize ozone sensor pin
        pinMode(Config::MQ131_DIGITAL_PIN, INPUT);

        // Record sensor start times for warmup periods (gas sensors only)
        _coSensorStartTime = millis();
        _ozoneSensorStartTime = millis();
        _coSensorWarmedUp = false;
        _ozoneSensorWarmedUp = false;

        // DHT22 doesn't need warmup time, just initial reading
        readSensors();
        _disp.showDhtInitialized();
    }

    void poll() {
        if (millis() - _lastReading >= Config::SENSOR_REFRESH_MS) {
            readSensors();
        }
    }

    // Getters for sensor values
    float getIndoorTemp() const { return _indoorTemp; }
    float getIndoorHumidity() const { return _indoorHumidity; }
    uint16_t getCoAnalogReading() const { return _coAnalogReading; }
    float getCoVoltage() const { return _coVoltage; }
    float getCoPPM() const { return _coPPM; }
    bool getCoDigitalReading() const { return _coDigitalReading; }
    bool getOzoneDigitalReading() const { return _ozoneDigitalReading; }

    // Active-low corrected methods for alert checking
    bool isCoDetected() const { return !_coDigitalReading; }       // Inverted for active-low
    bool isOzoneDetected() const { return !_ozoneDigitalReading; } // Inverted for active-low

    bool isCoSensorWarmedUp() const { return _coSensorWarmedUp; }
    bool isOzoneSensorWarmedUp() const { return _ozoneSensorWarmedUp; }
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

    // Get formatted DHT22 sensor data string for display
    String getFormattedData() const {
        if (!_dataValid) {
            return "DHT22 Error";
        }
        return "Indoor: " + String(_indoorTemp, 1) + "°C  " + String((int)_indoorHumidity) + "% RH";
    }

    // Get formatted CO sensor data string for display
    String getFormattedCoData() const {
        if (_coSensorWarmedUp) {
            String result = "CO: " + String((int)_coPPM) + " ppm";
            // Optionally show if digital threshold is also triggered
            if (!_coDigitalReading) { // Active-low: LOW = detected
                result += " (high)";
            }
            return result;
        } else {
            return "CO: Warming up...";
        }
    }

    // Get formatted ozone sensor data string for display
    String getFormattedOzoneData() const {
        if (_ozoneSensorWarmedUp) {
            // Active-low logic: LOW = detected (LED ON), HIGH = normal (LED OFF)
            if (!_ozoneDigitalReading) { // LOW = detected
                return "Ozone: Detected";
            } else {
                return "Ozone: Normal";
            }
        } else {
            return "Ozone: Warming up...";
        }
    }

private:
    void readSensors() {
        _lastReading = millis();

        // Check if sensors have warmed up
        if (!_coSensorWarmedUp && (millis() - _coSensorStartTime >= Config::MQ9_WARMUP_TIME_MS)) {
            _coSensorWarmedUp = true;
        }
        if (!_ozoneSensorWarmedUp && (millis() - _ozoneSensorStartTime >= Config::MQ131_WARMUP_TIME_MS)) {
            _ozoneSensorWarmedUp = true;
        }

        // Read temperature and humidity from DHT22
        float temp = _dht.readTemperature();
        float humidity = _dht.readHumidity();

        // Read CO sensor data
        _coAnalogReading = analogRead(Config::MQ9_ANALOG_PIN);
        _coVoltage = (_coAnalogReading / 1023.0) * Config::MQ9_VOLTAGE_REF;
        _coDigitalReading = digitalRead(Config::MQ9_DIGITAL_PIN);

        // Read ozone sensor data
        _ozoneDigitalReading = digitalRead(Config::MQ131_DIGITAL_PIN);

        // Calculate CO concentration in ppm (basic formula - needs calibration!)
        if (_coVoltage > Config::MQ9_CLEAN_AIR_VOLTAGE) {
            float ratio = _coVoltage / Config::MQ9_CLEAN_AIR_VOLTAGE;
            _coPPM = pow(10, ((log10(ratio) - Config::MQ9_CO_CURVE_OFFSET) / Config::MQ9_CO_CURVE_SLOPE));
            // Clamp to reasonable range
            if (_coPPM < 0) _coPPM = 0;
            if (_coPPM > Config::MQ9_MAX_PPM) _coPPM = Config::MQ9_MAX_PPM;
        } else {
            _coPPM = 0; // Clean air
        }

        // Check if DHT22 readings are valid
        if (isnan(temp) || isnan(humidity)) {
            _dataValid = false;
            _failedReadings++;

            // DHT22 reading failed!

            // If we have too many failed readings, try to reinitialize
            if (_failedReadings >= Config::MAX_SENSOR_FAILURES) {
                // Too many DHT22 failures, reinitializing...
                _dht.begin();
                _failedReadings = 0;
            }

            // Still update sensor displays even if DHT22 fails
            _disp.updateCoSensor(getFormattedCoData());
            _disp.updateCoDetails(_coVoltage, _coAnalogReading, _coDigitalReading);
            _disp.updateOzoneSensor(getFormattedOzoneData());
            return;
        }

        // Valid readings
        _indoorTemp = temp;
        _indoorHumidity = humidity;
        _dataValid = true;
        _failedReadings = 0;
        _lastValidReading = millis();

        // Update display with sensor data
        _disp.updateDhtSensor(getFormattedData());
        _disp.updateCoSensor(getFormattedCoData());
        _disp.updateCoDetails(_coVoltage, _coAnalogReading, _coDigitalReading);
        _disp.updateOzoneSensor(getFormattedOzoneData());

        // DHT22 readings logged
    }

    DisplayManager &_disp;
    DHT _dht;

    uint32_t _lastReading = 0;
    uint32_t _lastValidReading = 0;

    float _indoorTemp = 0.0;
    float _indoorHumidity = 0.0;
    bool _dataValid = false;

    uint8_t _failedReadings = 0;

    // CO sensor variables
    uint32_t _coSensorStartTime = 0;
    bool _coSensorWarmedUp = false;
    uint16_t _coAnalogReading = 0;
    float _coVoltage = 0.0;
    float _coPPM = 0.0;
    bool _coDigitalReading = false;

    // Ozone sensor variables
    uint32_t _ozoneSensorStartTime = 0;
    bool _ozoneSensorWarmedUp = false;
    bool _ozoneDigitalReading = false;
};
