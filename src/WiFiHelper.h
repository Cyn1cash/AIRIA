#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <ESP8266WiFi.h>

class WiFiHelper {
public:
    explicit WiFiHelper(DisplayManager &disp) : _disp(disp) {}

    void begin() {
        connect();
    }

    bool poll() {
        if (WiFi.status() != WL_CONNECTED) {
            if (_everConnected) {
                _everConnected = false;
                _disp.begin();
                connect();
            } else if (!_connecting) {
                connect();
            }
            return false;
        }
        if (!_everConnected) {
            _everConnected = true;
            return true;
        }
        return false;
    }

private:
    void connect() {
        _connecting = true;

        WiFi.mode(WIFI_STA);
        WiFi.begin(Config::SSID, Config::PASSWORD);
        _disp.showWifiConnecting();

        const uint32_t t0 = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t0 < Config::WIFI_TIMEOUT_MS) {
            delay(250);
        }
        _connecting = false;

        if (WiFi.status() == WL_CONNECTED) {
            _disp.showWifiConnected(WiFi.SSID().c_str(), WiFi.localIP());
        } else {
            ESP.restart();
        }
    }

    DisplayManager &_disp;
    bool _everConnected = false;
    bool _connecting = false;
};
