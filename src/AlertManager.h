#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
#include "WeatherHelper.h"
#include <Arduino.h>
#include <NexButton.h>
#include <NexHardware.h>

enum class AlertType {
    NONE = 0,
    TEMP_HIGH,
    TEMP_LOW,
    TEMP_DIFFERENCE_HIGH,
    HUMIDITY_HIGH,
    HUMIDITY_LOW,
    HUMIDITY_DIFFERENCE_HIGH,
    CO_HIGH,
    OZONE_DETECTED,
    POWER_HIGH,
    DAILY_COST_HIGH
};

struct AlertInfo {
    AlertType type;
    String message;
    bool active;
    uint32_t firstTriggered;
};

// Forward declaration for callback
class AlertManager;
void buzzerButtonCallback(void *ptr);

class AlertManager {
public:
    explicit AlertManager(DisplayManager &disp, SensorHelper &sensors, EnergyEstimator &energy, WeatherHelper &weather)
        : _disp(disp), _sensors(sensors), _energy(energy), _weather(weather) {}

    void begin() {
        if (Config::BUZZER_ENABLED) {
            pinMode(Config::BUZZER_PIN, OUTPUT);
            digitalWrite(Config::BUZZER_PIN, LOW);
        }

        // Initialize alerts
        initializeAlerts();

        // Set up buzzer button callback
        _buzzerButton.attachPop(buzzerButtonCallback, this);

        // Set initial buzzer button state on display (enabled by default)
        sendCommand("details.buzzer.val=1");
        _buzzerRuntimeEnabled = true;
    }

    void poll() {
        // Check for alerts at regular intervals
        if (millis() - _lastAlertCheck >= Config::ALERT_CHECK_INTERVAL_MS) {
            checkAlerts();
            updateDisplay();
            _lastAlertCheck = millis();
        }

        // Handle buzzer only when needed with precise timing (every 10ms for accuracy)
        if (Config::BUZZER_ENABLED &&
            (hasActiveAlerts() || _buzzerState != BuzzerState::IDLE) &&
            (millis() - _lastBuzzerCheck >= 10)) {
            handleBuzzer();
            _lastBuzzerCheck = millis();
        }
    }

    // Get current alerts as a formatted string for display
    String getActiveAlertsString() const {
        String result = "";
        bool hasAlerts = false;

        for (const auto &alert : _alerts) {
            if (alert.active) {
                if (hasAlerts) {
                    result += ", ";
                }
                result += alert.message;
                hasAlerts = true;
            }
        }

        return hasAlerts ? ("Alerts: " + result) : "No alerts";
    } // Check if any alerts are currently active
    bool hasActiveAlerts() const {
        for (const auto &alert : _alerts) {
            if (alert.active) {
                return true;
            }
        }
        return false;
    }

    // Get number of active alerts
    uint8_t getActiveAlertCount() const {
        uint8_t count = 0;
        for (const auto &alert : _alerts) {
            if (alert.active) {
                count++;
            }
        }
        return count;
    }

    // Handle buzzer button press from Nextion display
    void handleBuzzerButtonPress() {
        uint32_t buttonValue;

        // Send command to get the button's val attribute
        sendCommand("get details.buzzer.val");
        if (recvRetNumber(&buttonValue)) {
            _buzzerRuntimeEnabled = (buttonValue == 1);
            // If buzzer is disabled, ensure it's turned off immediately
            if (!_buzzerRuntimeEnabled && Config::BUZZER_ENABLED) {
                digitalWrite(Config::BUZZER_PIN, LOW);
                _buzzerState = BuzzerState::IDLE;
                _beepCount = 0;
            }
        }
    }

    // Get the buzzer button object for nexLoop
    static NexButton *getBuzzerButton() {
        return &_buzzerButton;
    }

    // Get current buzzer runtime enable state
    bool isBuzzerRuntimeEnabled() const {
        return _buzzerRuntimeEnabled;
    }

private:
    void initializeAlerts() {
        _alerts[0] = {AlertType::TEMP_HIGH, "High Temp", false, 0};
        _alerts[1] = {AlertType::TEMP_LOW, "Low Temp", false, 0};
        _alerts[2] = {AlertType::TEMP_DIFFERENCE_HIGH, "High Temp Diff", false, 0};
        _alerts[3] = {AlertType::HUMIDITY_HIGH, "High Humidity", false, 0};
        _alerts[4] = {AlertType::HUMIDITY_LOW, "Low Humidity", false, 0};
        _alerts[5] = {AlertType::HUMIDITY_DIFFERENCE_HIGH, "High Humidity Diff", false, 0};
        _alerts[6] = {AlertType::CO_HIGH, "High CO", false, 0};
        _alerts[7] = {AlertType::OZONE_DETECTED, "Ozone Detected", false, 0};
        _alerts[8] = {AlertType::POWER_HIGH, "High Power", false, 0};
        _alerts[9] = {AlertType::DAILY_COST_HIGH, "High Cost", false, 0};
    }

    void checkAlerts() {
        uint32_t currentTime = millis();

        // Check temperature alerts
        if (_sensors.isDataValid() && !isnan(_weather.getCurrentTemp())) {
            float indoorTemp = _sensors.getIndoorTemp();
            float outdoorTemp = _weather.getCurrentTemp();

            checkAndUpdateAlert(AlertType::TEMP_HIGH, indoorTemp > Config::TEMP_HIGH_THRESHOLD, currentTime);
            checkAndUpdateAlert(AlertType::TEMP_LOW, indoorTemp < Config::TEMP_LOW_THRESHOLD, currentTime);

            // Check temperature difference alert
            float tempDifference = abs(outdoorTemp - indoorTemp);
            checkAndUpdateAlert(AlertType::TEMP_DIFFERENCE_HIGH, tempDifference > Config::TEMP_DIFFERENCE_HIGH_THRESHOLD, currentTime);

            float humidity = _sensors.getIndoorHumidity();
            checkAndUpdateAlert(AlertType::HUMIDITY_HIGH, humidity > Config::HUMIDITY_HIGH_THRESHOLD, currentTime);
            checkAndUpdateAlert(AlertType::HUMIDITY_LOW, humidity < Config::HUMIDITY_LOW_THRESHOLD, currentTime);

            // Check humidity difference alert
            if (!isnan(_weather.getCurrentHumidity())) {
                float outdoorHumidity = _weather.getCurrentHumidity();
                float humidityDifference = abs(outdoorHumidity - humidity);
                checkAndUpdateAlert(AlertType::HUMIDITY_DIFFERENCE_HIGH, humidityDifference > Config::HUMIDITY_DIFFERENCE_HIGH_THRESHOLD, currentTime);
            }
        }

        // Check CO alerts
        if (_sensors.isCoSensorWarmedUp()) {
            float coPPM = _sensors.getCoPPM();
            checkAndUpdateAlert(AlertType::CO_HIGH, coPPM > Config::CO_HIGH_THRESHOLD, currentTime);
        }

        // Check ozone alerts
        if (_sensors.isOzoneSensorWarmedUp() && Config::OZONE_ALERT_ON_DETECTION) {
            bool ozoneDetected = _sensors.isOzoneDetected(); // Use active-low corrected method
            checkAndUpdateAlert(AlertType::OZONE_DETECTED, ozoneDetected, currentTime);
        }

        // Check energy alerts
        float powerWatts = _energy.getEstimatedPowerWatts();
        checkAndUpdateAlert(AlertType::POWER_HIGH, powerWatts > Config::POWER_HIGH_THRESHOLD, currentTime);

        float dailyCost = _energy.getDailyCostEstimate();
        checkAndUpdateAlert(AlertType::DAILY_COST_HIGH, dailyCost > Config::DAILY_COST_HIGH_THRESHOLD, currentTime);
    }

    void checkAndUpdateAlert(AlertType type, bool condition, uint32_t currentTime) {
        for (auto &alert : _alerts) {
            if (alert.type == type) {
                if (condition && !alert.active) {
                    // Alert just became active
                    alert.active = true;
                    alert.firstTriggered = currentTime;
                } else if (!condition && alert.active) {
                    // Alert just became inactive
                    alert.active = false;
                    alert.firstTriggered = 0;
                }
                break;
            }
        }
    }

    void updateDisplay() {
        String alertsString = getActiveAlertsString();
        _disp.updateAlerts(alertsString);
    }

    void handleBuzzer() {
        // Skip buzzer functionality if disabled in config or runtime
        if (!Config::BUZZER_ENABLED || !_buzzerRuntimeEnabled) {
            // Ensure buzzer is off when disabled
            if (Config::BUZZER_ENABLED) {
                digitalWrite(Config::BUZZER_PIN, LOW);
            }
            _buzzerState = BuzzerState::IDLE;
            _beepCount = 0;
            return;
        }

        if (!hasActiveAlerts()) {
            // No alerts, ensure buzzer is off
            digitalWrite(Config::BUZZER_PIN, LOW);
            _buzzerState = BuzzerState::IDLE;
            _beepCount = 0;
            return;
        }

        uint32_t currentTime = millis();

        switch (_buzzerState) {
        case BuzzerState::IDLE:
            // Start beeping sequence
            _buzzerState = BuzzerState::BEEPING;
            _buzzerStartTime = currentTime;
            _beepCount = 0;
            digitalWrite(Config::BUZZER_PIN, HIGH);
            break;

        case BuzzerState::BEEPING:
            if (currentTime - _buzzerStartTime >= Config::BUZZER_BEEP_DURATION_MS) {
                // Turn off buzzer and enter pause
                digitalWrite(Config::BUZZER_PIN, LOW);
                _buzzerState = BuzzerState::PAUSED;
                _buzzerStartTime = currentTime;
                _beepCount++;
            }
            break;

        case BuzzerState::PAUSED:
            if (currentTime - _buzzerStartTime >= Config::BUZZER_BEEP_INTERVAL_MS) {
                if (_beepCount < Config::BUZZER_BEEP_COUNT) {
                    // Start next beep
                    _buzzerState = BuzzerState::BEEPING;
                    _buzzerStartTime = currentTime;
                    digitalWrite(Config::BUZZER_PIN, HIGH);
                } else {
                    // Wait before next sequence (longer pause)
                    _buzzerState = BuzzerState::WAITING;
                    _buzzerStartTime = currentTime;
                }
            }
            break;

        case BuzzerState::WAITING:
            // Wait for a longer period before starting next beep sequence
            if (currentTime - _buzzerStartTime >= (Config::ALERT_CHECK_INTERVAL_MS * 2)) {
                // Start new beeping sequence
                _buzzerState = BuzzerState::IDLE;
            }
            break;
        }
    }

    enum class BuzzerState {
        IDLE,
        BEEPING,
        PAUSED,
        WAITING
    };

    DisplayManager &_disp;
    SensorHelper &_sensors;
    EnergyEstimator &_energy;
    WeatherHelper &_weather;

    uint32_t _lastAlertCheck = 0;
    uint32_t _lastBuzzerCheck = 0; // Add timing control for buzzer
    AlertInfo _alerts[10];         // Array to store all alert types (increased from 9 to 10)

    // Buzzer control
    bool _buzzerRuntimeEnabled = true; // Runtime buzzer enable/disable state
    static NexButton _buzzerButton;    // Buzzer control button on Nextion display

    // Buzzer state management
    BuzzerState _buzzerState = BuzzerState::IDLE;
    uint32_t _buzzerStartTime = 0;
    uint8_t _beepCount = 0;

    friend void buzzerButtonCallback(void *ptr);
};

NexButton AlertManager::_buzzerButton(2, 11, "buzzer");

// Callback function for buzzer button press
void buzzerButtonCallback(void *ptr) {
    AlertManager *alertManager = static_cast<AlertManager *>(ptr);
    if (alertManager) {
        alertManager->handleBuzzerButtonPress();
    }
}
