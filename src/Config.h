#pragma once
#include <Arduino.h>

namespace Config {
    /* Wi-Fi ------------------------------------------------------ */
    constexpr char SSID[] = "tk";
    constexpr char PASSWORD[] = "fuckthisworld";
    constexpr int WIFI_TIMEOUT_MS = 15'000;
    constexpr int WIFI_RETRY_DELAY_MS = 5'000;

    /* Time / NTP ------------------------------------------------- */
    constexpr long GMT_OFFSET_SEC = 8 * 3600; // UTC+8
    constexpr char NTP_SERVER[] = "pool.ntp.org";

    /* Misc ------------------------------------------------------- */
    constexpr uint32_t CLOCK_REFRESH_MS = 1'000; // update once per second

    /* Weather / Location ---------------------------------------- */
    constexpr double LATITUDE = 1.330591328881519; // Ngee Ann Polytechnic
    constexpr double LONGITUDE = 103.77506363783029;
    constexpr uint32_t WEATHER_REFRESH_MS = 15'000; // (15 s for demo; make 15*60*1000 in prod)

    /* Sensors --------------------------------------------------- */
    constexpr uint8_t DHT22_PIN = 4;              // GPIO 4 (D2 on NodeMCU)
    constexpr uint32_t SENSOR_REFRESH_MS = 5'000; // Read sensors every 5 seconds
    constexpr uint8_t MAX_SENSOR_FAILURES = 5;    // Max consecutive failed readings before reinit

    /* Energy Estimation ----------------------------------------- */
    constexpr uint32_t ENERGY_CALC_REFRESH_MS = 10'000;   // Calculate energy every 10 seconds
    constexpr float TARGET_INDOOR_TEMP = 24.0;            // Target indoor temperature in Celsius
    constexpr float AC_BASE_POWER_WATTS = 800.0;          // Base power consumption of AC unit
    constexpr float AC_MIN_POWER_WATTS = 200.0;           // Minimum power (standby/fan only)
    constexpr float AC_MAX_POWER_WATTS = 2000.0;          // Maximum power consumption
    constexpr float TEMP_LOAD_FACTOR = 0.05;              // Power increase factor per degree difference
    constexpr float EFFICIENCY_DEGRADATION_FACTOR = 0.02; // Efficiency loss per degree difference
    constexpr float ELECTRICITY_RATE_PER_KWH = 0.25;      // Electricity rate in $/kWh (adjust for your region)

    // Room characteristics for heat load calculation
    constexpr float ROOM_HEAT_TRANSFER_COEFF = 3.5; // W/m²·K - Overall heat transfer coefficient
    constexpr float ROOM_SURFACE_AREA = 45.0;       // m² - Total surface area (walls, ceiling, floor)
    constexpr float ROOM_AIR_VOLUME = 75.0;         // m³ - Room air volume

    // Latent heat calculation parameters
    constexpr float LATENT_HEAT_FACTOR = 0.012;     // W·s/m³·%RH - Latent heat removal factor
    constexpr float LATENT_HEAT_TEMP_FACTOR = 0.03; // Temperature effect on latent heat

    // COP (Coefficient of Performance) calculation
    constexpr float AC_CARNOT_EFFICIENCY = 0.45;   // Real COP as fraction of Carnot COP (typical 40-50%)
    constexpr float COP_DEGRADATION_FACTOR = 0.02; // COP reduction per degree temperature difference
    constexpr float MIN_COP_DEGRADATION = 0.3;     // Minimum degradation factor (30%)
    constexpr float MIN_COP = 2.0;                 // Minimum realistic COP
    constexpr float MAX_COP = 6.0;                 // Maximum realistic COP

    // AC unit specific parameters
    constexpr float AC_UNIT_EFFICIENCY_FACTOR = 0.85; // Overall AC unit efficiency (85%)
    constexpr float WATTS_TO_BTU_HR = 3.412;          // Conversion factor

    // Operating condition penalties
    constexpr float OPTIMAL_TEMP_DIFFERENCE = 8.0;       // °C - Optimal temperature difference
    constexpr float OPTIMAL_HUMIDITY = 60.0;             // % - Optimal outdoor humidity
    constexpr float TEMP_EFFICIENCY_PENALTY = 0.015;     // Efficiency penalty per degree above optimal
    constexpr float HUMIDITY_EFFICIENCY_PENALTY = 0.002; // Efficiency penalty per % humidity above optimal
}