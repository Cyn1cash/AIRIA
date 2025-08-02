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

    // Print available serial commands
    Serial.println("=== AIRIA Serial Commands ===");
    Serial.println("recommendedConfig - Get configuration recommendations");
    Serial.println("heatLoadDetails - Get detailed heat load analysis");
    Serial.println("heatLoadSummary - Get current heat load summary");
    Serial.println("help - Show this command list");
    Serial.println("==============================");
}

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim(); // Remove whitespace and newlines

        if (command == "recommendedConfig") {
            Serial.println("\n" + energyEstimator.getConfigRecommendations());
        } else if (command == "heatLoadDetails") {
            Serial.println("\n" + energyEstimator.getHeatLoadDetails());
        } else if (command == "heatLoadSummary") {
            Serial.println("\n" + energyEstimator.getHeatLoadSummary());
        } else if (command == "help") {
            Serial.println("\n=== AIRIA Serial Commands ===");
            Serial.println("recommendedConfig - Get configuration recommendations");
            Serial.println("heatLoadDetails - Get detailed heat load analysis");
            Serial.println("heatLoadSummary - Get current heat load summary");
            Serial.println("help - Show this command list");
            Serial.println("==============================");
        } else if (command.length() > 0) {
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
