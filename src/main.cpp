#include "AlertManager.h"
#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
#include "ThingsBoardHelper.h"
#include "TimeHelper.h"
#include "WeatherHelper.h"
#include "WiFiHelper.h"

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
}
