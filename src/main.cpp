#include "Config.h"
#include "DisplayManager.h"
#include "EnergyEstimator.h"
#include "SensorHelper.h"
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

/* ---------- Arduino lifecycle ---------- */
void setup() {
    Serial.begin(9600);
    display.begin();

    // Initialize sensors early
    sensors.begin();

    wifi.begin();
}

void loop() {
    if (wifi.poll()) {
        timeManager.begin();
        weather.begin();
        energyEstimator.begin();
        delay(3000);
        display.showMain();
    }
    timeManager.poll();
    weather.poll();
    sensors.poll();         // Poll sensors continuously
    energyEstimator.poll(); // Calculate energy usage
}
