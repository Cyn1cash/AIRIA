#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include "SensorHelper.h"
#include "WeatherHelper.h"

enum class ACPowerState {
    OFF,      // AC is completely off
    STARTING, // AC just turned on, high power draw
    RUNNING,  // AC is actively cooling
    IDLE      // AC is on but compressor is off (fan only)
};

class EnergyEstimator {
public:
    explicit EnergyEstimator(DisplayManager &disp, SensorHelper &sensors, WeatherHelper &weather)
        : _disp(disp), _sensors(sensors), _weather(weather), _acState(ACPowerState::OFF) {}

    void begin() {
        // Initialize energy estimator with AC off
        _acState = ACPowerState::OFF;
        _lastStateChange = millis();
        _totalRuntimeToday = 0;
        _dailyEnergyConsumed = 0.0;
        _lastDayReset = millis();
    }

    void poll() {
        if (millis() - _lastCalculation >= Config::ENERGY_CALC_REFRESH_MS) {
            updateACState();
            calculateEnergyUsage();
            trackDailyUsage();
        }
    }

    // Manual AC control methods
    void setACOn() {
        if (_acState == ACPowerState::OFF) {
            _acState = ACPowerState::STARTING;
            _lastStateChange = millis();
        }
    }

    void setACOff() {
        if (_acState != ACPowerState::OFF) {
            // Add runtime to today's total before turning off
            _totalRuntimeToday += (millis() - _lastStateChange) / Config::MILLIS_TO_SECONDS; // seconds
            _acState = ACPowerState::OFF;
            _lastStateChange = millis();
        }
    }

    // Get current AC state
    ACPowerState getACState() const { return _acState; }

    // Get today's total runtime in hours
    float getTodaysRuntimeHours() const {
        uint32_t currentRuntime = _totalRuntimeToday;
        if (_acState != ACPowerState::OFF) {
            currentRuntime += (millis() - _lastStateChange) / Config::MILLIS_TO_SECONDS;
        }
        return currentRuntime / Config::SECONDS_TO_HOURS;
    }

    // Get estimated power consumption in watts (0 if AC is off)
    float getEstimatedPowerWatts() const { return _estimatedPowerWatts; }

    // Get today's actual energy consumption in kWh
    float getTodaysEnergyKWh() const { return _dailyEnergyConsumed; }

    // Get projected daily energy consumption estimate in kWh
    float getDailyEnergyKWh() const { return _dailyEnergyKWh; }

    // Get cost estimate for today's actual usage
    float getTodaysCostEstimate() const { return _dailyEnergyConsumed * Config::ELECTRICITY_RATE_PER_KWH; }

    // Get projected daily cost estimate
    float getDailyCostEstimate() const { return _dailyEnergyKWh * Config::ELECTRICITY_RATE_PER_KWH; }

    // Get current COP (Coefficient of Performance) - efficiency metric
    float getCurrentCOP() const { return _currentCOP; }

    // Get heat load in BTU/hr
    float getHeatLoadBTU() const { return _heatLoadBTU; }

    // Get energy efficiency ratio
    float getEER() const { return _currentEER; }

    // Get current duty cycle (percentage of day AC has been running)
    float getCurrentDutyCycle() const { return _currentDutyCycle; }

    // Get current heat load in watts (for monitoring auto control decisions)
    float getCurrentHeatLoadWatts() const {
        if (!_sensors.isDataValid() || isnan(_weather.getCurrentTemp()) || isnan(_weather.getCurrentHumidity())) {
            return 0.0;
        }

        float indoorTemp = _sensors.getIndoorTemp();
        float outdoorTemp = _weather.getCurrentTemp();
        float indoorHumidity = _sensors.getIndoorHumidity();
        float outdoorHumidity = _weather.getCurrentHumidity();

        float sensibleLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
        float latentLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
        return sensibleLoad + latentLoad;
    }

    // Check if AC would auto-start based on current conditions
    bool wouldAutoStart() const {
        return getCurrentHeatLoadWatts() > Config::AUTO_ON_HEAT_LOAD_THRESHOLD;
    }

    // New methods for individual object updates
    String getCurrentDrawString() const {
        return "Current Usage: " + String((int)_estimatedPowerWatts) + " W";
    }

    String getDailyEstimateString() const {
        return "Daily: " + String(_dailyEnergyKWh, 2) + " kWh/day";
    }

    String getEnergyStatusString() const {
        // Check if AC is off first
        if (_acState == ACPowerState::OFF) {
            return "Status: AC Off";
        }

        // Check power threshold
        if (_estimatedPowerWatts > Config::POWER_HIGH_THRESHOLD) {
            return "Status: Power High";
        }

        // Check daily cost threshold
        if (getDailyCostEstimate() > Config::DAILY_COST_HIGH_THRESHOLD) {
            return "Status: Cost High";
        }

        return "Status: Normal";
    }

    // Check if AC would auto-stop based on current conditions
    bool wouldAutoStop() const {
        return getCurrentHeatLoadWatts() < Config::AUTO_OFF_HEAT_LOAD_THRESHOLD;
    }

    // Get detailed heat load breakdown for configuration and monitoring
    String getHeatLoadDetails() const {
        if (!_sensors.isDataValid() || isnan(_weather.getCurrentTemp()) || isnan(_weather.getCurrentHumidity())) {
            return "Heat Load: INVALID DATA";
        }

        float indoorTemp = _sensors.getIndoorTemp();
        float outdoorTemp = _weather.getCurrentTemp();
        float indoorHumidity = _sensors.getIndoorHumidity();
        float outdoorHumidity = _weather.getCurrentHumidity();

        // Calculate individual components
        float sensibleLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
        float latentLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
        float totalLoad = sensibleLoad + latentLoad;

        // Calculate percentages
        float sensiblePercent = (totalLoad > 0) ? (sensibleLoad / totalLoad * 100.0) : 0.0;
        float latentPercent = (totalLoad > 0) ? (latentLoad / totalLoad * 100.0) : 0.0;

        // Temperature and humidity differences
        float tempDiff = abs(outdoorTemp - indoorTemp);
        float humidityDiff = abs(outdoorHumidity - indoorHumidity);

        // Build detailed report
        String details = "";
        details += "=== HEAT LOAD ANALYSIS ===\n";
        details += "Total Heat Load: " + String((int)totalLoad) + "W\n";
        details += "  - Sensible: " + String((int)sensibleLoad) + "W (" + String((int)sensiblePercent) + "%)\n";
        details += "  - Latent: " + String((int)latentLoad) + "W (" + String((int)latentPercent) + "%)\n\n";

        details += "=== CONDITIONS ===\n";
        details += "Indoor: " + String(indoorTemp, 1) + "°C, " + String((int)indoorHumidity) + "%RH\n";
        details += "Outdoor: " + String(outdoorTemp, 1) + "°C, " + String((int)outdoorHumidity) + "%RH\n";
        details += "Temp Diff: " + String(tempDiff, 1) + "°C\n";
        details += "Humidity Diff: " + String(humidityDiff, 1) + "%\n\n";

        details += "=== AUTO THRESHOLDS ===\n";
        details += "Auto ON: " + String((int)Config::AUTO_ON_HEAT_LOAD_THRESHOLD) + "W ";
        details += (totalLoad > Config::AUTO_ON_HEAT_LOAD_THRESHOLD) ? "[WOULD START]" : "[below]";
        details += "\nAuto OFF: " + String((int)Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) + "W ";
        details += (totalLoad < Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) ? "[WOULD STOP]" : "[above]";
        details += "\n\n";

        details += "=== RECOMMENDATIONS ===\n";
        if (totalLoad < 200) {
            details += "Very low load - consider higher AUTO_OFF threshold\n";
        } else if (totalLoad > 1500) {
            details += "Very high load - verify room parameters\n";
        }

        if (sensiblePercent > 80) {
            details += "Mostly temperature-driven - check insulation\n";
        } else if (latentPercent > 60) {
            details += "High humidity load - check ventilation\n";
        }

        if (tempDiff > 12) {
            details += "Large temp difference - peak cooling needed\n";
        } else if (tempDiff < 3) {
            details += "Small temp difference - minimal cooling\n";
        }

        return details;
    }

    // Update heat load details on Nextion display
    void updateHeatLoadDisplay() const {
        if (!_sensors.isDataValid() || isnan(_weather.getCurrentTemp()) || isnan(_weather.getCurrentHumidity())) {
            _disp.updateHeatLoadTotal("No Data");
            return;
        }

        float indoorTemp = _sensors.getIndoorTemp();
        float outdoorTemp = _weather.getCurrentTemp();
        float indoorHumidity = _sensors.getIndoorHumidity();
        float outdoorHumidity = _weather.getCurrentHumidity();

        // Calculate components
        float sensibleLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
        float latentLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
        float totalLoad = sensibleLoad + latentLoad;

        // Calculate percentages
        float sensiblePercent = (totalLoad > 0) ? (sensibleLoad / totalLoad * 100.0) : 0.0;
        float latentPercent = (totalLoad > 0) ? (latentLoad / totalLoad * 100.0) : 0.0;

        // Temperature and humidity differences
        float tempDiff = abs(outdoorTemp - indoorTemp);
        float humidityDiff = abs(outdoorHumidity - indoorHumidity);

        // Update display components
        _disp.updateHeatLoadTotal(String((int)totalLoad) + "W");
        _disp.updateHeatLoadSensible(String((int)sensibleLoad) + "W (" + String((int)sensiblePercent) + "%)");
        _disp.updateHeatLoadLatent(String((int)latentLoad) + "W (" + String((int)latentPercent) + "%)");

        _disp.updateHeatLoadIndoor("In: " + String(indoorTemp, 1) + "°C " + String((int)indoorHumidity) + "%");
        _disp.updateHeatLoadOutdoor("Out: " + String(outdoorTemp, 1) + "°C " + String((int)outdoorHumidity) + "%");
        _disp.updateHeatLoadDifferences("ΔT:" + String(tempDiff, 1) + "°C ΔH:" + String(humidityDiff, 1) + "%");

        // Threshold status
        String onStatus = (totalLoad > Config::AUTO_ON_HEAT_LOAD_THRESHOLD) ? "WOULD START" : "below";
        String offStatus = (totalLoad < Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) ? "WOULD STOP" : "above";

        _disp.updateHeatLoadThresholdOn("ON:" + String((int)Config::AUTO_ON_HEAT_LOAD_THRESHOLD) + "W " + onStatus);
        _disp.updateHeatLoadThresholdOff("OFF:" + String((int)Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) + "W " + offStatus);

        // Recommendations
        String recommendation = "";
        if (totalLoad < 200) {
            recommendation = "Very low load - raise AUTO_OFF";
        } else if (totalLoad > 1500) {
            recommendation = "Very high load - check room config";
        } else if (sensiblePercent > 80) {
            recommendation = "Temperature-driven - check insulation";
        } else if (latentPercent > 60) {
            recommendation = "High humidity - check ventilation";
        } else if (tempDiff > 12) {
            recommendation = "Large temp diff - peak cooling";
        } else if (tempDiff < 3) {
            recommendation = "Small temp diff - minimal cooling";
        } else {
            recommendation = "Heat load is balanced";
        }

        _disp.updateHeatLoadRecommendation(recommendation);
    }

    // Get simplified heat load summary for display
    String getHeatLoadSummary() const {
        if (!_sensors.isDataValid() || isnan(_weather.getCurrentTemp()) || isnan(_weather.getCurrentHumidity())) {
            return "Heat Load: No Data";
        }

        float totalLoad = getCurrentHeatLoadWatts();
        String status = "";

        if (totalLoad > Config::AUTO_ON_HEAT_LOAD_THRESHOLD) {
            status = "HIGH";
        } else if (totalLoad > Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) {
            status = "MED";
        } else {
            status = "LOW";
        }

        return "Heat Load: " + String((int)totalLoad) + "W (" + status + ")";
    }

    // Get configuration recommendations based on current conditions
    String getConfigRecommendations() const {
        String recommendations = "";
        recommendations += "=== CONFIGURATION RECOMMENDATIONS ===\n\n";

        // Room size recommendations
        float roomVolume = Config::ROOM_AIR_VOLUME;
        float roomArea = Config::ROOM_SURFACE_AREA;

        recommendations += "Current Room Config:\n";
        recommendations += "- Volume: " + String(roomVolume, 0) + "m³\n";
        recommendations += "- Surface Area: " + String(roomArea, 0) + "m²\n";
        recommendations += "- Heat Transfer Coeff: " + String(Config::ROOM_HEAT_TRANSFER_COEFF, 1) + "\n\n";

        // Threshold recommendations based on room size
        float recommendedOnThreshold = roomVolume * 10.0; // ~10W per m³
        float recommendedOffThreshold = recommendedOnThreshold * 0.5;

        recommendations += "Recommended Thresholds:\n";
        recommendations += "- AUTO_ON_HEAT_LOAD_THRESHOLD: " + String((int)recommendedOnThreshold) + "W\n";
        recommendations += "- AUTO_OFF_HEAT_LOAD_THRESHOLD: " + String((int)recommendedOffThreshold) + "W\n";
        recommendations += "(Current: " + String((int)Config::AUTO_ON_HEAT_LOAD_THRESHOLD) + "W / ";
        recommendations += String((int)Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) + "W)\n\n";

        // AC power recommendations
        float minRecommendedPower = roomVolume * 15.0; // ~15W per m³
        float maxRecommendedPower = roomVolume * 25.0; // ~25W per m³

        recommendations += "Recommended AC Power Range:\n";
        recommendations += "- MIN: " + String((int)minRecommendedPower) + "W\n";
        recommendations += "- MAX: " + String((int)maxRecommendedPower) + "W\n";
        recommendations += "(Current: " + String((int)Config::AC_MIN_POWER_WATTS) + "W / ";
        recommendations += String((int)Config::AC_MAX_POWER_WATTS) + "W)\n\n";

        recommendations += "=== TUNING TIPS ===\n";
        recommendations += "1. Monitor heat load for 1 week\n";
        recommendations += "2. Note typical HIGH/MED/LOW values\n";
        recommendations += "3. Set AUTO_ON = typical HIGH value\n";
        recommendations += "4. Set AUTO_OFF = 50% of AUTO_ON\n";
        recommendations += "5. Adjust based on comfort/efficiency\n";

        return recommendations;
    }

    // Calculate sensible heat load based on temperature difference (public for monitoring)
    float calculateSensibleHeatLoad(float indoorTemp, float outdoorTemp) const {
        // Sensible heat load = U * A * ΔT
        // Where U = overall heat transfer coefficient, A = surface area, ΔT = temperature difference
        float tempDifference = abs(outdoorTemp - indoorTemp);

        // Simplified calculation using room characteristics
        float sensibleLoad = Config::ROOM_HEAT_TRANSFER_COEFF *
                             Config::ROOM_SURFACE_AREA *
                             tempDifference;

        return sensibleLoad;
    }

    // Calculate latent heat load based on humidity difference (public for monitoring)
    float calculateLatentHeatLoad(float indoorHumidity, float outdoorHumidity, float indoorTemp, float outdoorTemp) const {
        // Latent heat load considers moisture removal
        float humidityDiff = abs(outdoorHumidity - indoorHumidity);

        // Higher temperatures increase latent heat capacity
        float avgTemp = (indoorTemp + outdoorTemp) / 2.0;
        float tempFactor = 1.0 + (avgTemp - 20.0) * Config::LATENT_HEAT_TEMP_FACTOR;

        // Latent load = humidity difference × air volume × latent heat factor
        float latentLoad = humidityDiff *
                           Config::ROOM_AIR_VOLUME *
                           Config::LATENT_HEAT_FACTOR *
                           tempFactor;

        return latentLoad;
    }

private:
    // Update AC state based on temperature control needs
    void updateACState() {
        if (!_sensors.isDataValid()) {
            return;
        }

        float indoorTemp = _sensors.getIndoorTemp();
        float tempError = abs(indoorTemp - Config::TARGET_INDOOR_TEMP);

        switch (_acState) {
        case ACPowerState::OFF:
            // Auto turn on if heat load exceeds threshold
            if (!isnan(_weather.getCurrentTemp()) && !isnan(_weather.getCurrentHumidity())) {
                float outdoorTemp = _weather.getCurrentTemp();
                float indoorHumidity = _sensors.getIndoorHumidity();
                float outdoorHumidity = _weather.getCurrentHumidity();

                // Calculate current heat load to determine if AC should turn on
                float sensibleLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
                float latentLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
                float totalHeatLoad = sensibleLoad + latentLoad;

                // Auto turn on if total heat load exceeds threshold
                if (totalHeatLoad > Config::AUTO_ON_HEAT_LOAD_THRESHOLD) {
                    _acState = ACPowerState::STARTING;
                    _lastStateChange = millis();
                }
            }
            break;

        case ACPowerState::STARTING:
            // Transition from starting to running after startup period
            if (millis() - _lastStateChange >= Config::AC_STARTUP_TIME_MS) {
                _acState = ACPowerState::RUNNING;
                _lastStateChange = millis();
            }
            break;

        case ACPowerState::RUNNING:
            // Switch to idle if target temperature is reached
            if (tempError <= Config::TEMP_DEADBAND) {
                _acState = ACPowerState::IDLE;
                _lastStateChange = millis();
            }
            break;

        case ACPowerState::IDLE:
            // Return to running if temperature drifts too far from target
            if (tempError > Config::TEMP_DEADBAND + Config::TEMP_DEADBAND_TOLERANCE) {
                _acState = ACPowerState::RUNNING;
                _lastStateChange = millis();
            }
            // Auto turn off if heat load is very low and AC has been running for minimum time
            else if (!isnan(_weather.getCurrentTemp()) && !isnan(_weather.getCurrentHumidity()) &&
                     (millis() - _lastStateChange >= Config::AUTO_OFF_MIN_TIME_MS)) {
                float outdoorTemp = _weather.getCurrentTemp();
                float indoorHumidity = _sensors.getIndoorHumidity();
                float outdoorHumidity = _weather.getCurrentHumidity();

                // Calculate current heat load
                float sensibleLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
                float latentLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
                float totalHeatLoad = sensibleLoad + latentLoad;

                // Auto turn off if heat load is below lower threshold (hysteresis)
                if (totalHeatLoad < Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) {
                    // Add runtime to today's total before turning off
                    _totalRuntimeToday += (millis() - _lastStateChange) / Config::MILLIS_TO_SECONDS;
                    _acState = ACPowerState::OFF;
                    _lastStateChange = millis();
                }
            }
            break;
        }
    }

    void trackDailyUsage() {
        // Reset daily stats at midnight (24 hour period)
        if (millis() - _lastDayReset >= Config::MILLISECONDS_PER_DAY) {
            _totalRuntimeToday = 0;
            _dailyEnergyConsumed = 0.0;
            _lastDayReset = millis();
        }

        // Add current energy consumption to daily total
        if (_acState != ACPowerState::OFF) {
            float timeSinceLastCalc = (millis() - _lastCalculation) / Config::MILLIS_TO_SECONDS / Config::SECONDS_TO_HOURS; // hours
            _dailyEnergyConsumed += _estimatedPowerWatts * timeSinceLastCalc / Config::WATTS_TO_KILOWATTS;                  // kWh
        }
    }

    void calculateEnergyUsage() {
        _lastCalculation = millis();

        // Calculate power consumption based on current AC state
        if (_acState == ACPowerState::OFF) {
            _estimatedPowerWatts = 0.0;
            _currentCOP = 0.0;
            _heatLoadBTU = 0.0;
            _currentEER = 0.0;
            _currentDutyCycle = 0.0;
        } else {
            // Only calculate if we have valid sensor data
            if (!_sensors.isDataValid()) {
                return;
            }

            float indoorTemp = _sensors.getIndoorTemp();
            float outdoorTemp = _weather.getCurrentTemp();
            float indoorHumidity = _sensors.getIndoorHumidity();
            float outdoorHumidity = _weather.getCurrentHumidity();

            // Calculate heat load (same as before)
            float sensibleHeatLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
            float latentHeatLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
            float totalHeatLoad = sensibleHeatLoad + latentHeatLoad;

            _heatLoadBTU = totalHeatLoad * Config::WATTS_TO_BTU_HR;

            // Calculate COP
            float tempDifference = abs(outdoorTemp - indoorTemp);
            _currentCOP = calculateCOP(tempDifference, outdoorTemp);

            // Calculate base power consumption
            float basePower = totalHeatLoad / _currentCOP;
            basePower *= calculateEfficiencyFactor(tempDifference, outdoorHumidity);
            basePower *= Config::AC_UNIT_EFFICIENCY_FACTOR;

            // Apply state-specific power multipliers
            switch (_acState) {
            case ACPowerState::STARTING:
                // High power draw during startup
                _estimatedPowerWatts = basePower * Config::AC_STARTUP_POWER_MULTIPLIER;
                break;

            case ACPowerState::RUNNING:
                // Full power consumption
                _estimatedPowerWatts = basePower;
                break;

            case ACPowerState::IDLE:
                // Only fan power consumption
                _estimatedPowerWatts = Config::AC_FAN_ONLY_POWER_WATTS;
                break;

            default:
                _estimatedPowerWatts = 0.0;
                break;
            }

            // Ensure reasonable bounds
            _estimatedPowerWatts = constrain(_estimatedPowerWatts, 0.0, Config::AC_MAX_POWER_WATTS);

            // Calculate EER
            _currentEER = (_estimatedPowerWatts > 0) ? _heatLoadBTU / _estimatedPowerWatts : 0.0;

            // Calculate current duty cycle as percentage of day AC has been running
            float todayHours = getTodaysRuntimeHours();
            float dayProgress = (millis() - _lastDayReset) / Config::MILLIS_TO_SECONDS / Config::SECONDS_TO_HOURS; // hours since day reset
            _currentDutyCycle = (dayProgress > 0) ? (todayHours / dayProgress) : 0.0;
            _currentDutyCycle = constrain(_currentDutyCycle, 0.0, 1.0);
        }

        // Calculate projected daily energy consumption
        float todayProjectedHours = getTodaysRuntimeHours();
        if (_acState != ACPowerState::OFF) {
            // Estimate remaining runtime for today based on current conditions
            float avgPowerToday = (_dailyEnergyConsumed > 0 && todayProjectedHours > 0) ? (_dailyEnergyConsumed * Config::WATTS_TO_KILOWATTS / todayProjectedHours) : _estimatedPowerWatts;

            // Simple projection: assume similar usage pattern continues
            float remainingHoursInDay = Config::HOURS_PER_DAY - ((millis() - _lastDayReset) / Config::MILLIS_TO_SECONDS / Config::SECONDS_TO_HOURS);
            float projectedAdditionalHours = remainingHoursInDay * _currentDutyCycle;

            _dailyEnergyKWh = _dailyEnergyConsumed + (avgPowerToday * projectedAdditionalHours / Config::WATTS_TO_KILOWATTS);
        } else {
            // AC is off, just use energy consumed so far today
            _dailyEnergyKWh = _dailyEnergyConsumed;
        }

        // Update display
        updateEnergyDisplay();
    }

    void updateEnergyDisplay() {
        String stateStr = "";
        switch (_acState) {
        case ACPowerState::OFF:
            stateStr = "OFF";
            break;
        case ACPowerState::STARTING:
            stateStr = "STARTING";
            break;
        case ACPowerState::RUNNING:
            stateStr = "RUNNING";
            break;
        case ACPowerState::IDLE:
            stateStr = "IDLE";
            break;
        }

        // Update individual objects for new frontend
        _disp.updateCurrentDraw(getCurrentDrawString());
        _disp.updateDailyEstimate(getDailyEstimateString());
        _disp.updateEnergyStatus(getEnergyStatusString());
    }

    // Calculate Coefficient of Performance (COP) based on conditions
    float calculateCOP(float tempDifference, float outdoorTemp) {
        // COP decreases with larger temperature differences and higher outdoor temperatures
        // Carnot COP = T_cold / (T_hot - T_cold) - theoretical maximum

        float indoorTempK = Config::TARGET_INDOOR_TEMP + 273.15; // Convert to Kelvin
        float outdoorTempK = outdoorTemp + 273.15;

        // Theoretical Carnot COP
        float carnotCOP = indoorTempK / (outdoorTempK - indoorTempK);

        // Real COP is a fraction of Carnot COP
        float realCOP = carnotCOP * Config::AC_CARNOT_EFFICIENCY;

        // Apply degradation factors
        float degradationFactor = 1.0 - (tempDifference * Config::COP_DEGRADATION_FACTOR);
        realCOP *= (degradationFactor > Config::MIN_COP_DEGRADATION) ? degradationFactor : Config::MIN_COP_DEGRADATION;

        // Ensure reasonable bounds
        return constrain(realCOP, Config::MIN_COP, Config::MAX_COP);
    }

    // Calculate efficiency factor based on operating conditions
    float calculateEfficiencyFactor(float tempDifference, float outdoorHumidity) {
        float efficiencyFactor = 1.0;

        // High temperature difference reduces efficiency
        if (tempDifference > Config::OPTIMAL_TEMP_DIFFERENCE) {
            efficiencyFactor *= (1.0 + (tempDifference - Config::OPTIMAL_TEMP_DIFFERENCE) * Config::TEMP_EFFICIENCY_PENALTY);
        }

        // High humidity reduces efficiency (harder to remove moisture)
        if (outdoorHumidity > Config::OPTIMAL_HUMIDITY) {
            efficiencyFactor *= (1.0 + (outdoorHumidity - Config::OPTIMAL_HUMIDITY) * Config::HUMIDITY_EFFICIENCY_PENALTY);
        }

        return efficiencyFactor;
    }

    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;

    // AC State tracking
    ACPowerState _acState;
    uint32_t _lastStateChange = 0;
    uint32_t _totalRuntimeToday = 0;  // Total runtime today in seconds
    uint32_t _lastDayReset = 0;       // Last time daily stats were reset
    float _dailyEnergyConsumed = 0.0; // Energy consumed today in kWh

    // Energy calculation variables
    uint32_t _lastCalculation = 0;
    float _estimatedPowerWatts = 0.0;
    float _dailyEnergyKWh = 0.0;
    float _currentCOP = 0.0;
    float _heatLoadBTU = 0.0;
    float _currentEER = 0.0;
    float _currentDutyCycle = 0.0; // Kept for compatibility, but now calculated differently
};
