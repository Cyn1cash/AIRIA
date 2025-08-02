#include "AlertManager.h"
#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
#include "ThingsBoardHelper.h"
#include "TimeHelper.h"
#include "WeatherHelper.h"
#include "WiFiHelper.h"
#include <NexTouch.h>

/* ---------- Singletons ---------- */
DisplayManager display;
WiFiHelper wifi(display);
TimeHelper timeManager(display);
WeatherHelper weather(display);
SensorHelper sensors(display);
EnergyEstimator energyEstimator(display, sensors, weather);
ThingsBoardHelper thingsBoard(display, sensors, weather, energyEstimator);
AlertManager alertManager(display, sensors, energyEstimator, weather);

/* ---------- Arduino lifecycle ---------- */
void setup() {
    Serial.begin(9600);
    display.begin();

    wifi.begin();
}

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove whitespace and newlines

        // Configuration and monitoring commands
        if (command == "recommendedConfig") {
            Serial.println("\n" + energyEstimator.getConfigRecommendations());
        } else if (command == "heatLoadDetails") {
            Serial.println("\n" + energyEstimator.getHeatLoadDetails());
        } else if (command == "heatLoadSummary") {
            Serial.println("\n" + energyEstimator.getHeatLoadSummary());
        }

        // AC state control commands
        else if (command == "acOn") {
            energyEstimator.setACOn();
            Serial.println("AC turned ON (will start in STARTING state)");
        } else if (command == "acOff") {
            energyEstimator.setACOff();
            Serial.println("AC turned OFF");
        }

        // System status commands
        else if (command == "status") {
            Serial.println("\n=== SYSTEM STATUS ===");

            // Convert AC state enum to readable string
            String acStateStr = "";
            switch (energyEstimator.getACState()) {
            case ACPowerState::OFF:
                acStateStr = "OFF";
                break;
            case ACPowerState::STARTING:
                acStateStr = "STARTING";
                break;
            case ACPowerState::RUNNING:
                acStateStr = "RUNNING";
                break;
            case ACPowerState::IDLE:
                acStateStr = "IDLE";
                break;
            }

            Serial.println("AC State: " + acStateStr);
            Serial.println("Current Power: " + String(energyEstimator.getEstimatedPowerWatts()) + "W");
            Serial.println("Heat Load: " + String(energyEstimator.getCurrentHeatLoadWatts()) + "W");
            Serial.println("Indoor Temp: " + String(sensors.getIndoorTemp()) + "°C");
            Serial.println("Indoor Humidity: " + String(sensors.getIndoorHumidity()) + "%");
            Serial.println("Outdoor Temp: " + String(weather.getCurrentTemp()) + "°C");
            Serial.println("Outdoor Humidity: " + String(weather.getCurrentHumidity()) + "%");
            Serial.println("Data Valid: " + String(sensors.isDataValid() ? "Yes" : "No"));
            Serial.println("Target Temp: " + String(Config::TARGET_INDOOR_TEMP) + "°C");
            Serial.println("Temp Difference: " + String(abs(weather.getCurrentTemp() - sensors.getIndoorTemp())) + "°C");
        }

        // Sensor information commands
        else if (command == "sensorInfo") {
            Serial.println("\n=== SENSOR INFO ===");
            Serial.println(sensors.getIndoorTempString());
            Serial.println(sensors.getIndoorRhString());
            Serial.println(sensors.getIndoorStatusString());
            Serial.println(sensors.getCoValueString());
            Serial.println(sensors.getCoStatusString());
            Serial.println(sensors.getOzoneStatusString());
            Serial.println("CO Warmed Up: " + String(sensors.isCoSensorWarmedUp() ? "Yes" : "No"));
            Serial.println("Ozone Warmed Up: " + String(sensors.isOzoneSensorWarmedUp() ? "Yes" : "No"));
        }

        // Energy information commands
        else if (command == "energyInfo") {
            Serial.println("\n=== ENERGY INFO ===");
            Serial.println(energyEstimator.getCurrentDrawString());
            Serial.println(energyEstimator.getDailyEstimateString());
            Serial.println(energyEstimator.getEnergyStatusString());
            Serial.println("Today's Runtime: " + String(energyEstimator.getTodaysRuntimeHours(), 2) + " hours");
            Serial.println("Today's Energy: " + String(energyEstimator.getTodaysEnergyKWh(), 3) + " kWh");
            Serial.println("Current COP: " + String(energyEstimator.getCurrentCOP(), 2));
            Serial.println("Current EER: " + String(energyEstimator.getEER(), 2));
        }

        // Weather information commands
        else if (command == "weatherInfo") {
            Serial.println("\n=== WEATHER INFO ===");
            Serial.println(weather.getOutdoorTempString());
            Serial.println(weather.getOutdoorRhString());
            Serial.println("Raw Temp: " + String(weather.getCurrentTemp()) + "°C");
            Serial.println("Raw Humidity: " + String(weather.getCurrentHumidity()) + "%");
        }

        // Alert system commands
        else if (command == "alertInfo") {
            Serial.println("\n=== ALERT INFO ===");
            Serial.println("Alert Manager Status: Active");
            // Add any alert-specific status information if available
        }

        // Diagnostic commands
        else if (command == "autoStart") {
            bool wouldStart = energyEstimator.wouldAutoStart();
            Serial.println("Would auto-start: " + String(wouldStart ? "Yes" : "No"));
            Serial.println("Heat load threshold: " + String(Config::AUTO_ON_HEAT_LOAD_THRESHOLD) + "W");
            Serial.println("Current heat load: " + String(energyEstimator.getCurrentHeatLoadWatts()) + "W");
        } else if (command == "autoStop") {
            bool wouldStop = energyEstimator.wouldAutoStop();
            Serial.println("Would auto-stop: " + String(wouldStop ? "Yes" : "No"));
            Serial.println("Heat load threshold: " + String(Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) + "W");
            Serial.println("Current heat load: " + String(energyEstimator.getCurrentHeatLoadWatts()) + "W");
        } else if (command == "powerAnalysis") {
            Serial.println("\n=== POWER ANALYSIS ===");
            float heatLoad = energyEstimator.getCurrentHeatLoadWatts();
            float currentPower = energyEstimator.getEstimatedPowerWatts();
            float tempDiff = abs(weather.getCurrentTemp() - sensors.getIndoorTemp());

            Serial.println("Current Heat Load: " + String(heatLoad) + "W");
            Serial.println("Current Power Draw: " + String(currentPower) + "W");
            Serial.println("Temperature Difference: " + String(tempDiff) + "°C");

            Serial.println("\nPower Configuration:");
            Serial.println("- AC Base Power: " + String(Config::AC_BASE_POWER_WATTS) + "W");
            Serial.println("- AC Min Power: " + String(Config::AC_MIN_POWER_WATTS) + "W");
            Serial.println("- AC Max Power: " + String(Config::AC_MAX_POWER_WATTS) + "W");
            Serial.println("- Fan Only Power: " + String(Config::AC_FAN_ONLY_POWER_WATTS) + "W");

            Serial.println("\nThresholds:");
            Serial.println("- Auto ON threshold: " + String(Config::AUTO_ON_HEAT_LOAD_THRESHOLD) + "W");
            Serial.println("- Auto OFF threshold: " + String(Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) + "W");
            Serial.println("- Target temperature: " + String(Config::TARGET_INDOOR_TEMP) + "°C");
            Serial.println("- Temperature deadband: " + String(Config::TEMP_DEADBAND) + "°C");

            if (heatLoad < Config::AUTO_OFF_HEAT_LOAD_THRESHOLD) {
                Serial.println("\n** DIAGNOSIS: Heat load is below AUTO_OFF threshold **");
                Serial.println("   This explains why power consumption is low.");
            } else if (tempDiff <= Config::TEMP_DEADBAND) {
                Serial.println("\n** DIAGNOSIS: Temperature difference is within deadband **");
                Serial.println("   AC is likely in IDLE mode (fan only).");
            }
        } else if (command == "forceCalculation") {
            Serial.println("Forcing energy calculation update...");
            energyEstimator.poll(); // Force a calculation
            Serial.println("Calculation complete. Check status for updated values.");
        }

        // Help command
        else if (command == "help") {
            Serial.println("\n=== AVAILABLE COMMANDS ===");
            Serial.println("Configuration & Monitoring:");
            Serial.println("  recommendedConfig - Show configuration recommendations");
            Serial.println("  heatLoadDetails   - Show detailed heat load analysis");
            Serial.println("  heatLoadSummary   - Show heat load summary");
            Serial.println("");
            Serial.println("AC Control:");
            Serial.println("  acOn              - Turn AC on");
            Serial.println("  acOff             - Turn AC off");
            Serial.println("");
            Serial.println("System Status:");
            Serial.println("  status            - Show complete system status");
            Serial.println("  sensorInfo        - Show sensor readings");
            Serial.println("  energyInfo        - Show energy consumption info");
            Serial.println("  weatherInfo       - Show weather data");
            Serial.println("  alertInfo         - Show alert system status");
            Serial.println("");
            Serial.println("Diagnostics:");
            Serial.println("  autoStart         - Check if AC would auto-start");
            Serial.println("  autoStop          - Check if AC would auto-stop");
            Serial.println("  powerAnalysis     - Detailed power consumption analysis");
            Serial.println("  forceCalculation  - Force energy calculation update");
            Serial.println("  help              - Show this help message");
            Serial.println("");
            Serial.println("=== TROUBLESHOOTING TIPS ===");
            Serial.println("If power shows only ~57W with AC running:");
            Serial.println("1. Check 'status' - AC might be in IDLE state");
            Serial.println("2. Run 'powerAnalysis' for detailed diagnosis");
            Serial.println("3. Check if heat load < 400W (auto-idle threshold)");
            Serial.println("4. Use 'acOff' then 'acOn' to restart AC");
        }

        // Unknown command
        else {
            Serial.println("Unknown command: " + command);
            Serial.println("Type 'help' for available commands");
        }
    }
}

void loop() {
    // Wait for Wi-Fi connection before initializing other components
    if (wifi.poll()) {
        // Initialize all components once Wi-Fi is connected
        timeManager.begin();
        sensors.begin();
        weather.begin();
        energyEstimator.begin();
        thingsBoard.begin();
        alertManager.begin();

        delay(Config::COMPONENT_INIT_DELAY_MS); // Allow components to initialize
        display.showMain();
    }

    // Poll all components continuously
    timeManager.poll();
    weather.poll();
    sensors.poll();         // Poll sensors continuously
    energyEstimator.poll(); // Calculate energy usage
    thingsBoard.poll();     // Upload data to ThingsBoard
    alertManager.poll();    // Check for alerts and manage buzzer

    // Handle serial commands for debugging
    handleSerialCommands();
}
