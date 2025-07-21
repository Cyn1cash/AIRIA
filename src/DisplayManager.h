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
    void showMain() { sendCmd("page main"); }
    void updateClock(const char *hhmmss) {
        updateTextElement("time", hhmmss);
    }
    void updateWeather(const String &line) {
        updateTextElement("main.weather", line);
    }
    void updateDhtSensor(const String &line) {
        updateTextElement("main.dhtSensor", line);
    }
    void updateCoSensor(const String &line) {
        updateTextElement("main.coSensor", line);
    }
    void updateOzoneSensor(const String &line) {
        updateTextElement("main.ozoneSensor", line);
    }
    void updateEnergyEstimate(const String &line) {
        updateTextElement("main.energyEstimate", line);
    }
    void updateEnergyEstimate1(const String &line) {
        updateTextElement("main.energyEst1", line);
    }
    void updateEnergyEstimate2(const String &line) {
        updateTextElement("main.energyEst2", line);
    }
    void updateAlerts(const String &line) {
        updateTextElement("main.alerts", line);
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

private:
    static void sendCmd(const char *cmd) { ::sendCommand(cmd); }
    static void show(const char *id) { sendCmd((String("vis ") + id + ",1").c_str()); }
    static void hide(const char *id) { sendCmd((String("vis ") + id + ",0").c_str()); }

    // Helper method to reduce string concatenation overhead
    static void updateTextElement(const String &element, const String &text) {
        sendCmd((element + ".txt=\"" + text + "\"").c_str());
    }
};
