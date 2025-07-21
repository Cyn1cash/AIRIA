#pragma once
#include <Arduino.h>

namespace Config {
    /* Wi-Fi ------------------------------------------------------ */
    constexpr char SSID[] = "tkkr.dev";        // Wi-Fi SSID
    constexpr char PASSWORD[] = "tkkr.dev";    // Wi-Fi Password
    constexpr int WIFI_TIMEOUT_MS = 15'000;    // Wi-Fi connection timeout
    constexpr int WIFI_RETRY_DELAY_MS = 5'000; // Wi-Fi reconnection delay

    /* Time / NTP ------------------------------------------------- */
    constexpr long GMT_OFFSET_SEC = 8 * 3600;     // UTC+8
    constexpr char NTP_SERVER[] = "pool.ntp.org"; // NTP server for time synchronization

    /* Misc ------------------------------------------------------- */
    constexpr uint32_t CLOCK_REFRESH_MS = 1'000; // Update once per second

    /* Weather / Location ---------------------------------------- */
    constexpr double LATITUDE = 1.330591328881519; // Ngee Ann Polytechnic
    constexpr double LONGITUDE = 103.77506363783029;
    constexpr uint32_t WEATHER_REFRESH_MS = 15'000; // (15 s for demo; make 15*60*1000 in prod)

    /* NEA API --------------------------------------------------- */
    constexpr char NEA_TEMP_API[] = "https://api-open.data.gov.sg/v2/real-time/api/air-temperature";
    constexpr char NEA_HUMIDITY_API[] = "https://api-open.data.gov.sg/v2/real-time/api/relative-humidity";

    /* ThingsBoard ----------------------------------------------- */
    constexpr char THINGSBOARD_URL[] = "http://demo.thingsboard.io/api/v1/11d1ij5644804f100pcj/telemetry";
    constexpr uint32_t THINGSBOARD_UPLOAD_INTERVAL_MS = 30'000; // Upload every 30 seconds

    /* Sensors --------------------------------------------------- */
    constexpr uint8_t DHT22_PIN = 5;              // GPIO 5 (D1 on NodeMCU)
    constexpr uint8_t MQ9_ANALOG_PIN = A0;        // Analog pin for MQ-9 CO sensor
    constexpr uint8_t MQ9_DIGITAL_PIN = 14;       // GPIO 14 (D5 on NodeMCU) for MQ-9 digital output
    constexpr uint8_t MQ131_DIGITAL_PIN = 4;      // GPIO 4 (D2 on NodeMCU) for MQ-131 ozone sensor
    constexpr uint32_t SENSOR_REFRESH_MS = 5'000; // Read sensors every 5 seconds
    constexpr uint8_t MAX_SENSOR_FAILURES = 5;    // Max consecutive failed readings before reinit

    /* Buzzer / Alerts ------------------------------------------- */
    constexpr bool BUZZER_ENABLED = false;              // Enable/disable buzzer functionality
    constexpr uint8_t BUZZER_PIN = 12;                  // GPIO 12 (D6 on NodeMCU) for active-high buzzer
    constexpr uint32_t ALERT_CHECK_INTERVAL_MS = 2'000; // Check for alerts every 2 seconds
    constexpr uint16_t BUZZER_BEEP_DURATION_MS = 200;   // Individual beep duration (shorter for consistency)
    constexpr uint16_t BUZZER_BEEP_INTERVAL_MS = 300;   // Interval between beeps (shorter for better timing)
    constexpr uint8_t BUZZER_BEEP_COUNT = 3;            // Number of beeps per alert cycle

    /* CO Sensor (MQ-9) ------------------------------------------ */
    constexpr uint16_t MQ9_ANALOG_MIN = 0;         // Minimum analog reading (clean air)
    constexpr uint16_t MQ9_ANALOG_MAX = 1023;      // Maximum analog reading (10-bit ADC)
    constexpr float MQ9_VOLTAGE_REF = 3.3;         // Reference voltage for ESP8266
    constexpr uint32_t MQ9_WARMUP_TIME_MS = 20000; // 20 seconds warmup time for MQ-9

    // CO concentration calculation (these values need calibration!)
    constexpr float MQ9_CLEAN_AIR_VOLTAGE = 0.3; // Voltage in clean air (needs calibration)
    constexpr float MQ9_CO_CURVE_SLOPE = -0.45;  // Slope of CO curve (typical value, needs calibration)
    constexpr float MQ9_CO_CURVE_OFFSET = 0.9;   // Curve offset (needs calibration)

    /* Ozone Sensor (MQ-131) ------------------------------------- */
    constexpr uint32_t MQ131_WARMUP_TIME_MS = 30000; // 30 seconds warmup time (adjust as needed)

    /* Energy Estimation ----------------------------------------- */
    constexpr uint32_t ENERGY_CALC_REFRESH_MS = 10'000;   // Calculate energy every 10 seconds
    constexpr float TARGET_INDOOR_TEMP = 24.0;            // Target indoor temperature in Celsius
    constexpr float AC_BASE_POWER_WATTS = 1200.0;         // Base power consumption of AC unit (4.2kW cooling ÷ 3.5 COP)
    constexpr float AC_MIN_POWER_WATTS = 350.0;           // Minimum power (1.2kW cooling + 38W indoor unit)
    constexpr float AC_MAX_POWER_WATTS = 1800.0;          // Maximum power consumption (6.3kW cooling ÷ 3.5 COP)
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
    constexpr float AC_CARNOT_EFFICIENCY = 0.50;   // Real COP as fraction of Carnot COP (R32 systems are more efficient)
    constexpr float COP_DEGRADATION_FACTOR = 0.02; // COP reduction per degree temperature difference
    constexpr float MIN_COP_DEGRADATION = 0.3;     // Minimum degradation factor (30%)
    constexpr float MIN_COP = 2.5;                 // Minimum realistic COP (R32 systems perform better)
    constexpr float MAX_COP = 7.0;                 // Maximum realistic COP (R32 can achieve higher efficiency)

    // AC unit specific parameters
    constexpr float AC_UNIT_EFFICIENCY_FACTOR = 0.85; // Overall AC unit efficiency (85%)
    constexpr float WATTS_TO_BTU_HR = 3.412;          // Conversion factor

    // Operating condition penalties
    constexpr float OPTIMAL_TEMP_DIFFERENCE = 8.0;       // °C - Optimal temperature difference
    constexpr float OPTIMAL_HUMIDITY = 60.0;             // % - Optimal outdoor humidity
    constexpr float TEMP_EFFICIENCY_PENALTY = 0.015;     // Efficiency penalty per degree above optimal
    constexpr float HUMIDITY_EFFICIENCY_PENALTY = 0.002; // Efficiency penalty per % humidity above optimal

    // Duty cycle estimation parameters
    constexpr float MIN_DUTY_CYCLE = 0.2;            // Minimum duty cycle (20% for standby/fan)
    constexpr float MAX_DUTY_CYCLE = 0.95;           // Maximum duty cycle (95% - AC never runs 100%)
    constexpr float BASE_DUTY_CYCLE = 0.3;           // Base duty cycle for mild conditions
    constexpr float DUTY_CYCLE_TEMP_THRESHOLD = 2.0; // °C - Temperature difference threshold for increased duty
    constexpr float DUTY_CYCLE_LOAD_FACTOR = 0.035;  // Duty cycle increase per degree of outdoor temp difference
    constexpr float DUTY_CYCLE_CONTROL_FACTOR = 0.1; // Duty cycle correction per degree of indoor temp error

    /* Alert Thresholds ------------------------------------------ */
    // Temperature thresholds
    constexpr float TEMP_HIGH_THRESHOLD = 30.0;            // °C - High temperature alert
    constexpr float TEMP_LOW_THRESHOLD = 15.0;             // °C - Low temperature alert
    constexpr float TEMP_DIFFERENCE_HIGH_THRESHOLD = 12.0; // °C - High indoor/outdoor temperature difference alert

    // Humidity thresholds
    constexpr float HUMIDITY_HIGH_THRESHOLD = 80.0;            // % - High humidity alert
    constexpr float HUMIDITY_LOW_THRESHOLD = 30.0;             // % - Low humidity alert
    constexpr float HUMIDITY_DIFFERENCE_HIGH_THRESHOLD = 25.0; // % - High indoor/outdoor humidity difference alert

    // Air quality thresholds
    constexpr float CO_HIGH_THRESHOLD = 30.0;       // ppm - CO concentration alert (WHO 8-hour avg: 30 ppm)
    constexpr bool OZONE_ALERT_ON_DETECTION = true; // Alert when ozone is detected (digital sensor)

    // Energy consumption thresholds
    constexpr float POWER_HIGH_THRESHOLD = 1500.0;   // W - High power consumption alert
    constexpr float DAILY_COST_HIGH_THRESHOLD = 8.0; // $ - High daily cost alert
}