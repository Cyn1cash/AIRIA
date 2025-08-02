#pragma once
#include <ESP8266WiFi.h>
#include <Nextion.h>

class DisplayManager {
public:
    void begin() {
        nexInit();
        sendCmd("page start");
        sendCmd("start.arduinoConn.txt=\"Arduino connected!\"");
        hide("wifiConn");
        hide("timeSync");
        hide("dhtSensor");
    }

    void showWifiConnecting(uint16_t attempt = 0) {
        show("wifiConn");
        String msg = "Waiting for Wi-Fi connection...";
        if (attempt > 1) msg += " [" + String(attempt) + "]";
        updateTextElement("start.wifiConn", msg);
    }
    void showWifiConnected(const char *ssid, const IPAddress &ip) {
        show("wifiConn");
        updateTextElement("start.wifiConn", String("Connected to ") + ssid + "!");
        updateTextElement("details.ssid", String("SSID: ") + ssid);
        updateTextElement("details.ipAddress", String("IP: ") + ip.toString());
    }

    void showTimeSyncing() {
        show("timeSync");
        updateTextElement("start.timeSync", "Synchronising time...");
    }
    void showTimeSynced() {
        show("timeSync");
        updateTextElement("start.timeSync", "Synchronised with NTP pool!");
    }

    void showDhtInitializing() {
        show("dhtSensor");
        updateTextElement("start.dhtSensor", "Initialising DHT22 sensor...");
    }
    void showDhtInitialized() {
        show("dhtSensor");
        updateTextElement("start.dhtSensor", "DHT22 sensor ready!");
    }

    void showThingsBoardSuccess() {
        // Show active upload indicator on details page
        updateTextElement("details.uploadStatus", "↗ Uploading to ThingsBoard");
    }
    void showThingsBoardError(const String &error) {
        // Show error on details page
        updateTextElement("details.uploadStatus", "✗ Upload failed: " + error);
    }

    /* ---------- Main page ---------- */
    void showMain() {
        sendCmd("page main");
        initializeStatusIndicators();
    }

    void initializeStatusIndicators() {
        // Initialize all indicators to normal state (hide all warning indicators)
        hide("warning");          // Outdoor weather
        hide("indoorIndWarning"); // Indoor conditions
        hide("energyIndWarning"); // Energy usage
        hide("airIndWarning");    // Air quality
    }
    void updateClock(const char *hhmmss) {
        updateTextElement("time", hhmmss);
    }

    /* ---------- New specific object updates ---------- */
    void updateOutdoorTemp(const String &text) {
        updateTextElement("main.outTemp", text);
    }
    void updateOutdoorRh(const String &text) {
        updateTextElement("main.outRh", text);
    }
    void updateIndoorTemp(const String &text) {
        updateTextElement("main.inTemp", text);
    }
    void updateIndoorRh(const String &text) {
        updateTextElement("main.inRh", text);
    }
    void updateIndoorStatus(const String &text) {
        updateTextElement("main.inStatus", text);
    }
    void updateCurrentDraw(const String &text) {
        updateTextElement("main.currentDraw", text);
    }
    void updateDailyEstimate(const String &text) {
        updateTextElement("main.dailyEst", text);
    }
    void updateEnergyStatus(const String &text) {
        updateTextElement("main.energyStatus", text);
    }
    void updateCoValue(const String &text) {
        updateTextElement("main.coVal", text);
    }
    void updateCoStatus(const String &text) {
        updateTextElement("main.coStatus", text);
    }
    void updateOzoneStatus(const String &text) {
        updateTextElement("main.ozoneStatus", text);
    }

    /* -------- Status Indicators --------- */
    void updateOutdoorIndicator(bool isNormal) {
        if (isNormal) {
            hide("warning"); // Hide frowny face for outdoor weather
        } else {
            show("warning"); // Show frowny face for outdoor weather
        }
    }

    void updateIndoorIndicator(bool isNormal) {
        if (isNormal) {
            hide("indoorIndWarning"); // Hide frowny face for indoor conditions
        } else {
            show("indoorIndWarning"); // Show frowny face for indoor conditions
        }
    }

    void updateEnergyIndicator(bool isNormal) {
        if (isNormal) {
            hide("energyIndWarning"); // Hide frowny face for energy usage
        } else {
            show("energyIndWarning"); // Show frowny face for energy usage
        }
    }

    void updateAirQualityIndicator(bool isNormal) {
        if (isNormal) {
            hide("airIndWarning"); // Hide frowny face for air quality
        } else {
            show("airIndWarning"); // Show frowny face for air quality
        }
    }

    /* -------- Details page --------- */
    void showLocation(double lat, double lon) {
        updateTextElement("details.latitude", String("Latitude: ") + String(lat, 15));
        updateTextElement("details.longitude", String("Longitude: ") + String(lon, 15));
    }
    void updateCoDetails(float voltage, uint16_t analogReading, bool digitalReading) {
        String digitalState = digitalReading ? "HIGH" : "LOW";
        updateTextElement("details.coDetails",
                          String("CO: ") + String(voltage, 2) + "V (ADC: " + String(analogReading) + ") (D: " + digitalState + ")");
    }

    /* -------- Heat Load page ------- */
    void updateHeatLoadTotal(const String &value) {
        updateTextElement("heatload.total", value);
    }
    void updateHeatLoadSensible(const String &value) {
        updateTextElement("heatload.sensible", value);
    }
    void updateHeatLoadLatent(const String &value) {
        updateTextElement("heatload.latent", value);
    }
    void updateHeatLoadIndoor(const String &value) {
        updateTextElement("heatload.indoor", value);
    }
    void updateHeatLoadOutdoor(const String &value) {
        updateTextElement("heatload.outdoor", value);
    }
    void updateHeatLoadDifferences(const String &value) {
        updateTextElement("heatload.differences", value);
    }
    void updateHeatLoadThresholdOn(const String &value) {
        updateTextElement("heatload.thresholdOn", value);
    }
    void updateHeatLoadThresholdOff(const String &value) {
        updateTextElement("heatload.thresholdOff", value);
    }
    void updateHeatLoadRecommendation(const String &value) {
        updateTextElement("heatload.recommendation", value);
    }

private:
    static void sendCmd(const char *cmd) { ::sendCommand(cmd); }
    static void show(const char *id) { sendCmd((String("vis ") + id + ",1").c_str()); }
    static void hide(const char *id) { sendCmd((String("vis ") + id + ",0").c_str()); }

    // Helper method to reduce string concatenation overhead
    static void updateTextElement(const String &element, const String &text) {
        sendCmd((element + ".txt=\"" + text + "\"").c_str());
    }
};
