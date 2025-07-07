#pragma once
#include <Arduino.h>

namespace Config {
    /* Wi-Fi ------------------------------------------------------ */
    constexpr char SSID[] = "Casy";
    constexpr char PASSWORD[] = "Shikanokonoko";
    constexpr int WIFI_TIMEOUT_MS = 15'000;
    constexpr int WIFI_RETRY_DELAY_MS = 5'000;

    /* Time / NTP ------------------------------------------------- */
    constexpr long GMT_OFFSET_SEC = 8 * 3600; // UTC+8
    constexpr char NTP_SERVER[] = "pool.ntp.org";

    /* Misc ------------------------------------------------------- */
    constexpr uint32_t CLOCK_REFRESH_MS = 1'000; // update once per second

    /* Weather / Location ---------------------------------------- */
    constexpr double LATITUDE = 1.2838617993247599; // Marina Bay Sands, Singapore
    constexpr double LONGITUDE = 103.85922276445453;
    constexpr uint32_t WEATHER_REFRESH_MS = 15'000; // (15 s for demo; make 15*60*1000 in prod)
}
