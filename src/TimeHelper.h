#pragma once
#include "Config.h"
#include "DisplayManager.h"
#include <time.h>

class TimeHelper {
public:
    explicit TimeHelper(DisplayManager &disp) : _disp(disp) {}

    void begin() {
        _disp.showTimeSyncing();

        configTime(Config::GMT_OFFSET_SEC, 0, Config::NTP_SERVER);
        while (time(nullptr) < 100'000)
            delay(100);

        _disp.showTimeSynced();
        _lastClockUpdate = millis();
    }

    void poll() {
        if (millis() - _lastClockUpdate >= Config::CLOCK_REFRESH_MS) {
            _lastClockUpdate = millis();
            char buf[12]; // HH:MM:SS AM
            formatTime(buf, sizeof(buf));
            _disp.updateClock(buf);
        }
    }

private:
    static void formatTime(char *out, size_t len) {
        time_t now = time(nullptr);
        struct tm *tm = localtime(&now);
        int hour = tm->tm_hour;
        const char *ampm = "AM";
        if (hour >= 12) {
            ampm = "PM";
            if (hour > 12)
                hour -= 12;
        } else if (hour == 0)
            hour = 12;
        snprintf(out, len, "%02d:%02d:%02d %s", hour, tm->tm_min, tm->tm_sec, ampm);
    }

    DisplayManager &_disp;
    uint32_t _lastClockUpdate = 0;
};
