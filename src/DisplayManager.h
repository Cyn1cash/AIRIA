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
        sendCmd((String("start.wifiConn.txt=\"") + msg + "\"").c_str());
    }
    void showWifiConnected(const char *ssid, const IPAddress &ip) {
        show("wifiConn");
        sendCmd((String("start.wifiConn.txt=\"Connected to ") + ssid + "!\"").c_str());
        sendCmd((String("details.ssid.txt=\"SSID: ") + ssid + "\"").c_str());
        sendCmd((String("details.ipAddress.txt=\"IP: ") + ip.toString() + "\"").c_str());
    }

    void showTimeSyncing() {
        show("timeSync");
        sendCmd("start.timeSync.txt=\"Synchronising time...\"");
    }
    void showTimeSynced() {
        show("timeSync");
        sendCmd("start.timeSync.txt=\"Synchronised with NTP pool!\"");
    }

    void showDhtInitializing() {
        show("dhtSensor");
        sendCmd("start.dhtSensor.txt=\"Initialising DHT22 sensor...\"");
    }
    void showDhtInitialized() {
        show("dhtSensor");
        sendCmd("start.dhtSensor.txt=\"DHT22 sensor ready!\"");
    }

    void showThingsBoardSuccess() {
        // Show active upload indicator on details page
        sendCmd("details.uploadStatus.txt=\"↗ Uploading to ThingsBoard\"");
    }
    void showThingsBoardError(const String &error) {
        // Show error on details page
        sendCmd((String("details.uploadStatus.txt=\"✗ Upload failed: ") + error + "\"").c_str());
    }

    /* ---------- Main page ---------- */
    void showMain() { sendCmd("page main"); }
    void updateClock(const char *hhmmss) {
        sendCmd((String("time.txt=\"") + hhmmss + "\"").c_str());
    }
    void updateWeather(const String &line) {
        sendCmd((String("main.weather.txt=\"") + line + "\"").c_str());
    }
    void updateDhtSensor(const String &line) {
        sendCmd((String("main.dhtSensor.txt=\"") + line + "\"").c_str());
    }
    void updateCoSensor(const String &line) {
        sendCmd((String("main.coSensor.txt=\"") + line + "\"").c_str());
    }
    void updateOzoneSensor(const String &line) {
        sendCmd((String("main.ozoneSensor.txt=\"") + line + "\"").c_str());
    }
    void updateEnergyEstimate(const String &line) {
        sendCmd((String("main.energyEstimate.txt=\"") + line + "\"").c_str());
    }
    void updateAlerts(const String &line) {
        sendCmd((String("main.alerts.txt=\"") + line + "\"").c_str());
    }

    /* -------- Details page --------- */
    void showLocation(double lat, double lon) {
        sendCmd((String("details.latitude.txt=\"Latitude: ") + String(lat, 15) + "\"").c_str());
        sendCmd((String("details.longitude.txt=\"Longitude: ") + String(lon, 15) + "\"").c_str());
    }
    void updateCoDetails(float voltage, uint16_t analogReading, bool digitalReading) {
        String digitalState = digitalReading ? "HIGH" : "LOW";
        sendCmd((String("details.coDetails.txt=\"CO: ") + String(voltage, 2) + "V (ADC: " + String(analogReading) + ") (D: " + digitalState + ")\"").c_str());
    }

private:
    static void sendCmd(const char *cmd) { ::sendCommand(cmd); }
    static void show(const char *id) { sendCmd((String("vis ") + id + ",1").c_str()); }
    static void hide(const char *id) { sendCmd((String("vis ") + id + ",0").c_str()); }
};
