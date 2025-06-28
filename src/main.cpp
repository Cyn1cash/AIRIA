#include "Config.h"
#include "DisplayManager.h"
#include "TimeHelper.h"
#include "WeatherHelper.h"
#include "WiFiHelper.h"

/* ---------- Singletons ---------- */
DisplayManager display;
WiFiHelper wifi(display);
TimeHelper timeManager(display);
WeatherHelper weather(display);

/* ---------- Arduino lifecycle ---------- */
void setup() {
    Serial.begin(9600);
    display.begin();

    wifi.begin();
}

void loop() {
    if (wifi.poll()) {
        timeManager.begin();
        weather.begin();
        delay(3000);
        display.showMain();
    }
    timeManager.poll();
    weather.poll();
}
