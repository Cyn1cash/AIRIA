#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include "SensorHelper.h"
#include "WeatherHelper.h"

class EnergyEstimator {
public:
    explicit EnergyEstimator(DisplayManager &disp, SensorHelper &sensors, WeatherHelper &weather)
        : _disp(disp), _sensors(sensors), _weather(weather) {}

    void begin() {
        // Energy estimator initialized
    }

    void poll() {
        if (millis() - _lastCalculation >= Config::ENERGY_CALC_REFRESH_MS) {
            calculateEnergyUsage();
        }
    }

    // Get estimated power consumption in watts
    float getEstimatedPowerWatts() const { return _estimatedPowerWatts; }

    // Get daily energy consumption estimate in kWh
    float getDailyEnergyKWh() const { return _dailyEnergyKWh; }

    // Get cost estimate per day (assuming electricity rate)
    float getDailyCostEstimate() const { return _dailyEnergyKWh * Config::ELECTRICITY_RATE_PER_KWH; }

    // Get current COP (Coefficient of Performance) - efficiency metric
    float getCurrentCOP() const { return _currentCOP; }

    // Get heat load in BTU/hr
    float getHeatLoadBTU() const { return _heatLoadBTU; }

    // Get energy efficiency ratio
    float getEER() const { return _currentEER; }

    // Get estimated duty cycle (fraction of time AC is running)
    float getCurrentDutyCycle() const { return _currentDutyCycle; }

private:
    void calculateEnergyUsage() {
        _lastCalculation = millis();

        // Only calculate if we have valid sensor data
        if (!_sensors.isDataValid()) {
            return;
        }

        float indoorTemp = _sensors.getIndoorTemp();
        float outdoorTemp = _weather.getCurrentTemp();
        float indoorHumidity = _sensors.getIndoorHumidity();
        float outdoorHumidity = _weather.getCurrentHumidity();

        // Enhanced energy estimation model based on HVAC thermodynamics

        // 1. Calculate heat load using sensible and latent heat components
        float sensibleHeatLoad = calculateSensibleHeatLoad(indoorTemp, outdoorTemp);
        float latentHeatLoad = calculateLatentHeatLoad(indoorHumidity, outdoorHumidity, indoorTemp, outdoorTemp);
        float totalHeatLoad = sensibleHeatLoad + latentHeatLoad;

        // Store BTU/hr for reference
        _heatLoadBTU = totalHeatLoad * Config::WATTS_TO_BTU_HR;

        // 2. Calculate Coefficient of Performance (COP) based on temperature difference
        float tempDifference = abs(outdoorTemp - indoorTemp);
        _currentCOP = calculateCOP(tempDifference, outdoorTemp);

        // 3. Calculate actual power consumption
        _estimatedPowerWatts = totalHeatLoad / _currentCOP;

        // 4. Apply efficiency degradation factors
        _estimatedPowerWatts *= calculateEfficiencyFactor(tempDifference, outdoorHumidity);

        // 5. Apply AC unit specific factors
        _estimatedPowerWatts *= Config::AC_UNIT_EFFICIENCY_FACTOR;

        // 6. Calculate EER (Energy Efficiency Ratio)
        _currentEER = _heatLoadBTU / _estimatedPowerWatts;

        // Ensure reasonable bounds
        _estimatedPowerWatts = constrain(_estimatedPowerWatts, Config::AC_MIN_POWER_WATTS, Config::AC_MAX_POWER_WATTS);

        // Calculate estimated duty cycle based on temperature control needs
        float dutyCycle = calculateDutyCycle(indoorTemp, outdoorTemp);
        _currentDutyCycle = dutyCycle;

        // Calculate daily energy consumption (kWh) with realistic duty cycle
        _dailyEnergyKWh = (_estimatedPowerWatts * 24.0 * dutyCycle) / 1000.0;

        // Update display with energy information
        updateEnergyDisplay();
    }

    void updateEnergyDisplay() {
        String energyLine = String((int)_estimatedPowerWatts) + "W  •  " +
                            String(_dailyEnergyKWh, 1) + "kWh/day  •  " +
                            "$" + String(getDailyCostEstimate(), 2) + "/day  •  " +
                            String((int)(_currentDutyCycle * 100)) + "% duty";

        _disp.updateEnergyEstimate(energyLine);
    }

    // Calculate sensible heat load based on temperature difference
    float calculateSensibleHeatLoad(float indoorTemp, float outdoorTemp) {
        // Sensible heat load = U * A * ΔT
        // Where U = overall heat transfer coefficient, A = surface area, ΔT = temperature difference
        float tempDifference = abs(outdoorTemp - indoorTemp);

        // Simplified calculation using room characteristics
        float sensibleLoad = Config::ROOM_HEAT_TRANSFER_COEFF *
                             Config::ROOM_SURFACE_AREA *
                             tempDifference;

        return sensibleLoad;
    }

    // Calculate latent heat load based on humidity difference
    float calculateLatentHeatLoad(float indoorHumidity, float outdoorHumidity, float indoorTemp, float outdoorTemp) {
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

    // Calculate estimated duty cycle (fraction of time AC is running)
    float calculateDutyCycle(float indoorTemp, float outdoorTemp) {
        // Calculate how far we are from target temperature
        float tempError = abs(indoorTemp - Config::TARGET_INDOOR_TEMP);
        float outdoorTempDiff = abs(outdoorTemp - Config::TARGET_INDOOR_TEMP);

        // Base duty cycle on outdoor conditions - hotter outside = more runtime needed
        float baseDutyCycle = Config::BASE_DUTY_CYCLE;

        // If outdoor temp is close to target, AC barely needs to run
        if (outdoorTempDiff <= Config::DUTY_CYCLE_TEMP_THRESHOLD) {
            return baseDutyCycle;
        }

        // Calculate load-based duty cycle
        // More extreme outdoor temps = higher duty cycle
        float loadDutyCycle = (outdoorTempDiff - Config::DUTY_CYCLE_TEMP_THRESHOLD) * Config::DUTY_CYCLE_LOAD_FACTOR;
        loadDutyCycle = constrain(loadDutyCycle, 0.0, 0.7); // Max 70% for load

        // Add correction based on how well we're maintaining target temp
        float controlDutyCycle = 0.0;
        if (tempError > 1.0) {
            // If we're not maintaining target well, increase duty cycle
            controlDutyCycle = (tempError - 1.0) * Config::DUTY_CYCLE_CONTROL_FACTOR;
            controlDutyCycle = constrain(controlDutyCycle, 0.0, 0.3); // Max 30% correction
        }

        // Total duty cycle
        float totalDutyCycle = baseDutyCycle + loadDutyCycle + controlDutyCycle;

        // Ensure reasonable bounds
        return constrain(totalDutyCycle, Config::MIN_DUTY_CYCLE, Config::MAX_DUTY_CYCLE);
    }

    DisplayManager &_disp;
    SensorHelper &_sensors;
    WeatherHelper &_weather;

    uint32_t _lastCalculation = 0;
    float _estimatedPowerWatts = 0.0;
    float _dailyEnergyKWh = 0.0;
    float _currentCOP = 0.0;
    float _heatLoadBTU = 0.0;
    float _currentEER = 0.0;
    float _currentDutyCycle = 0.0;
};
