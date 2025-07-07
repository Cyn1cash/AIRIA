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
    }

    void showWifiConnecting(uint16_t attempt = 0) {
        show("wifiConn");
        String msg = "Waiting for Wi-Fi connection...";
        if (attempt) msg += " [" + String(attempt) + "]";
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

    /* ---------- Main page ---------- */
    void showMain() { sendCmd("page main"); }
    void updateClock(const char *hhmmss) {
        sendCmd((String("time.txt=\"") + hhmmss + "\"").c_str());
    }
    void updateWeather(const String &line) {
        sendCmd((String("main.weather.txt=\"") + line + "\"").c_str());
    }

    /* -------- Details page --------- */
    void showLocation(double lat, double lon) {
        sendCmd((String("details.latitude.txt=\"Latitude: ") + String(lat, 15) + "\"").c_str());
        sendCmd((String("details.longitude.txt=\"Longitude: ") + String(lon, 15) + "\"").c_str());
    }

private:
    static void sendCmd(const char *cmd) { ::sendCommand(cmd); }
    static void show(const char *id) { sendCmd((String("vis ") + id + ",1").c_str()); }
    static void hide(const char *id) { sendCmd((String("vis ") + id + ",0").c_str()); }
};
