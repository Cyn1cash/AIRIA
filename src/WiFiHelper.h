#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <ESP8266WiFi.h>

class WiFiHelper {
public:
    explicit WiFiHelper(DisplayManager &disp) : _disp(disp) {}

    void begin() {
        _retryCount = 1;
        connect();
    }

    bool poll() {
        if (WiFi.status() != WL_CONNECTED) {
            if (_everConnected) {
                _everConnected = false;
                _disp.begin();
                _retryCount = 1;
                connect();
            } else if (!_connecting && millis() - _lastAttempt >= Config::WIFI_RETRY_DELAY_MS) {
                connect();
            }
            return false;
        }
        if (!_everConnected) {
            _everConnected = true;
            _retryCount = 1;
            return true;
        }
        return false;
    }

private:
    void connect() {
        _connecting = true;
        _lastAttempt = millis();
        _disp.showWifiConnecting(_retryCount);

        WiFi.mode(WIFI_STA);
        WiFi.begin(Config::SSID, Config::PASSWORD);

        const uint32_t t0 = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - t0 < Config::WIFI_TIMEOUT_MS) {
            delay(250);
        }
        _connecting = false;

        if (WiFi.status() == WL_CONNECTED) {
            _disp.showWifiConnected(WiFi.SSID().c_str(), WiFi.localIP());
        } else {
            ++_retryCount;
        }
    }

    DisplayManager &_disp;
    bool _everConnected = false;
    bool _connecting = false;
    uint16_t _retryCount = 1;
    uint32_t _lastAttempt = 0;
};
