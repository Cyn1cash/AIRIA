#pragma once
#include <Arduino.h>

namespace Config {
    // =======================================================================
    // NETWORK & CONNECTIVITY
    // =======================================================================

    /* Wi-Fi ------------------------------------------------------ */
    constexpr char SSID[] = "Casy";                 // Wi-Fi SSID
    constexpr char PASSWORD[] = "Shikanokonoko";    // Wi-Fi Password
    constexpr int WIFI_TIMEOUT_MS = 15'000;         // Wi-Fi connection timeout
    constexpr int WIFI_RETRY_DELAY_MS = 5'000;      // Wi-Fi reconnection delay
    constexpr uint32_t WIFI_CONNECT_DELAY_MS = 250; // Delay between Wi-Fi connection attempts

    /* Network Timeouts ------------------------------------------- */
    constexpr uint32_t HTTP_TIMEOUT_MS = 15'000; // HTTP request timeout
    constexpr uint16_t JSON_DOC_SIZE = 8192;     // JSON document buffer size

    /* Time / NTP ------------------------------------------------- */
    constexpr long GMT_OFFSET_SEC = 8 * 3600;        // UTC+8
    constexpr char NTP_SERVER[] = "pool.ntp.org";    // NTP server for time synchronization
    constexpr uint32_t NTP_MIN_EPOCH_TIME = 100'000; // Minimum time to wait for NTP sync
    constexpr uint32_t NTP_SYNC_DELAY_MS = 100;      // Delay between NTP sync checks

    /* Weather / Location ---------------------------------------- */
    constexpr double LATITUDE = 1.330591328881519;
    constexpr double LONGITUDE = 103.77506363783029;
    constexpr uint32_t WEATHER_REFRESH_MS = 15'000; // (15 s for demo; make 15*60*1000 in prod)

    // Weather calculation constants
    constexpr double EARTH_RADIUS_KM = 6371.0;       // Earth's radius in kilometers
    constexpr double MAX_DISTANCE_SEARCH = 999999.0; // Maximum distance for station search

    /* NEA API --------------------------------------------------- */
    constexpr char NEA_TEMP_API[] = "https://api-open.data.gov.sg/v2/real-time/api/air-temperature";
    constexpr char NEA_HUMIDITY_API[] = "https://api-open.data.gov.sg/v2/real-time/api/relative-humidity";

    /* ThingsBoard ----------------------------------------------- */
    constexpr char THINGSBOARD_HTTP_URL[] = "http://demo.thingsboard.io/api/v1/4pz51pefyj8yig0m0f4w/telemetry"; // Complete HTTP URL
    constexpr uint32_t THINGSBOARD_UPLOAD_INTERVAL_MS = 60'000;                                                 // Upload every 1 minute (reduced for rate limits)
    constexpr bool THINGSBOARD_USE_CHUNKED_UPLOAD = true;                                                       // Split data into multiple smaller uploads
    constexpr uint32_t THINGSBOARD_CHUNK_DELAY_MS = 2000;                                                       // Delay between chunks (2 seconds)

    // =======================================================================
    // HARDWARE & SENSORS
    // =======================================================================

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
    constexpr uint16_t BUZZER_BEEP_DURATION_MS = 500;   // Individual beep duration
    constexpr uint16_t BUZZER_BEEP_INTERVAL_MS = 0;     // Interval between beeps
    constexpr uint8_t BUZZER_BEEP_COUNT = 255;          // Maximum beeps per alert cycle

    /* CO Sensor (MQ-9) ------------------------------------------ */
    constexpr uint16_t MQ9_ANALOG_MIN = 0;         // Minimum analog reading (clean air)
    constexpr uint16_t MQ9_ANALOG_MAX = 1023;      // Maximum analog reading (10-bit ADC)
    constexpr float MQ9_VOLTAGE_REF = 3.3;         // Reference voltage for ESP8266
    constexpr uint32_t MQ9_WARMUP_TIME_MS = 20000; // 20 seconds warmup time for MQ-9
    constexpr float MQ9_MAX_PPM = 1000.0;          // Maximum CO reading limit

    // CO concentration calculation (these values need calibration!)
    constexpr float MQ9_CLEAN_AIR_VOLTAGE = 0.3; // Voltage in clean air (needs calibration)
    constexpr float MQ9_CO_CURVE_SLOPE = -0.45;  // Slope of CO curve (typical value, needs calibration)
    constexpr float MQ9_CO_CURVE_OFFSET = 0.9;   // Curve offset (needs calibration)

    /* Ozone Sensor (MQ-131) ------------------------------------- */
    constexpr uint32_t MQ131_WARMUP_TIME_MS = 30000; // 30 seconds warmup time (adjust as needed)

    // =======================================================================
    // APPLICATION SETTINGS
    // =======================================================================

    /* Misc / Timing ---------------------------------------------- */
    constexpr uint32_t CLOCK_REFRESH_MS = 1'000;        // Update once per second
    constexpr uint32_t COMPONENT_INIT_DELAY_MS = 3'000; // Delay after component initialization
    constexpr uint32_t MILLISECONDS_PER_DAY = 86400000; // 24 hours in milliseconds

    // =======================================================================
    // ENERGY ESTIMATION MODEL
    // =======================================================================

    /* Energy Estimation ----------------------------------------- */
    constexpr uint32_t ENERGY_CALC_REFRESH_MS = 10'000;    // Calculate energy every 10 seconds
    constexpr float TARGET_INDOOR_TEMP = 24.0;             // Target indoor temperature in Celsius (more typical for Singapore)
    constexpr float AC_BASE_POWER_WATTS = 750.0;           // Base power consumption of AC unit (3kW cooling ÷ 4.0 COP)
    constexpr float AC_MIN_POWER_WATTS = 200.0;            // Minimum power (inverter minimum + fan)
    constexpr float AC_MAX_POWER_WATTS = 1200.0;           // Maximum power consumption during peak load
    constexpr float TEMP_LOAD_FACTOR = 0.08;               // Power increase factor per degree difference (8% per °C)
    constexpr float EFFICIENCY_DEGRADATION_FACTOR = 0.015; // Efficiency loss per degree difference (1.5% per °C)
    constexpr float ELECTRICITY_RATE_PER_KWH = 0.28;       // Singapore residential rate (~$0.28/kWh)

    /* Unit Conversion Constants --------------------------------- */
    constexpr float MILLIS_TO_SECONDS = 1000.0;    // Convert milliseconds to seconds
    constexpr float SECONDS_TO_HOURS = 3600.0;     // Convert seconds to hours
    constexpr float WATTS_TO_KILOWATTS = 1000.0;   // Convert watts to kilowatts
    constexpr float HOURS_PER_DAY = 24.0;          // Hours in a day
    constexpr uint8_t PERCENTAGE_MULTIPLIER = 100; // Convert decimal to percentage
    constexpr float WATTS_TO_BTU_HR = 3.412;       // Conversion factor

    /* Room Characteristics -------------------------------------- */
    constexpr float ROOM_HEAT_TRANSFER_COEFF = 3.5; // W/m²·K - Overall heat transfer coefficient
    constexpr float ROOM_SURFACE_AREA = 40.0;       // m² - Total surface area (walls, ceiling, floor)
    constexpr float ROOM_AIR_VOLUME = 35.0;         // m³ - Room air volume

    /* Heat Load Calculation Parameters -------------------------- */
    constexpr float LATENT_HEAT_FACTOR = 0.012;     // W·s/m³·%RH - Latent heat removal factor
    constexpr float LATENT_HEAT_TEMP_FACTOR = 0.03; // Temperature effect on latent heat

    /* COP (Coefficient of Performance) Calculation ------------- */
    constexpr float AC_CARNOT_EFFICIENCY = 0.45;    // Real COP as fraction of Carnot COP (45% for typical household AC)
    constexpr float COP_DEGRADATION_FACTOR = 0.015; // COP reduction per degree temperature difference
    constexpr float MIN_COP_DEGRADATION = 0.25;     // Minimum degradation factor (25%)
    constexpr float MIN_COP = 2.8;                  // Minimum realistic COP for household inverter AC
    constexpr float MAX_COP = 5.5;                  // Maximum realistic COP for high-efficiency household AC

    /* AC Unit Specific Parameters ------------------------------- */
    constexpr float AC_UNIT_EFFICIENCY_FACTOR = 0.82; // Overall AC unit efficiency (82% for typical household unit)

    /* Operating Condition Penalties ---------------------------- */
    constexpr float OPTIMAL_TEMP_DIFFERENCE = 6.0;       // °C - Optimal temperature difference (24°C indoor, 30°C outdoor)
    constexpr float OPTIMAL_HUMIDITY = 65.0;             // % - Optimal outdoor humidity for Singapore climate
    constexpr float TEMP_EFFICIENCY_PENALTY = 0.012;     // Efficiency penalty per degree above optimal
    constexpr float HUMIDITY_EFFICIENCY_PENALTY = 0.003; // Efficiency penalty per % humidity above optimal

    /* Duty Cycle Estimation Parameters ------------------------- */
    constexpr float MIN_DUTY_CYCLE = 0.15;            // Minimum duty cycle (15% for standby/fan only)
    constexpr float MAX_DUTY_CYCLE = 0.90;            // Maximum duty cycle (90% - allows for defrost cycles)
    constexpr float BASE_DUTY_CYCLE = 0.35;           // Base duty cycle for mild conditions
    constexpr float DUTY_CYCLE_TEMP_THRESHOLD = 3.0;  // °C - Temperature difference threshold
    constexpr float DUTY_CYCLE_LOAD_FACTOR = 0.045;   // Duty cycle increase per degree
    constexpr float DUTY_CYCLE_CONTROL_FACTOR = 0.12; // Duty cycle correction per degree of indoor temp error

    /* AC State Management --------------------------------------- */
    constexpr uint32_t AC_STARTUP_TIME_MS = 3000;         // 3 seconds startup time (typical for inverter AC)
    constexpr float AC_STARTUP_POWER_MULTIPLIER = 1.5;    // Power multiplier during startup (50% more)
    constexpr float AC_FAN_ONLY_POWER_WATTS = 80.0;       // Power consumption when only fan is running
    constexpr float TEMP_DEADBAND = 0.8;                  // °C - Temperature deadband for state switching
    constexpr float TEMP_DEADBAND_TOLERANCE = 0.3;        // °C - Additional tolerance for reactivation
    constexpr float AUTO_ON_HEAT_LOAD_THRESHOLD = 350.0;  // W - Heat load threshold for automatic AC turn-on
    constexpr float AUTO_OFF_HEAT_LOAD_THRESHOLD = 175.0; // W - Heat load threshold for automatic AC turn-off
    constexpr uint32_t AUTO_OFF_MIN_TIME_MS = 180000;     // 3 minutes - Minimum time before auto turn-off

    // =======================================================================
    // ALERT THRESHOLDS
    // =======================================================================

    // Temperature thresholds
    constexpr float TEMP_HIGH_THRESHOLD = 28.0;            // °C - High temperature alert
    constexpr float TEMP_LOW_THRESHOLD = 21.0;             // °C - Low temperature alert
    constexpr float TEMP_DIFFERENCE_HIGH_THRESHOLD = 12.0; // °C - High indoor/outdoor temperature difference

    // Humidity thresholds
    constexpr float HUMIDITY_HIGH_THRESHOLD = 70.0;            // % - High humidity alert
    constexpr float HUMIDITY_LOW_THRESHOLD = 40.0;             // % - Low humidity alert
    constexpr float HUMIDITY_DIFFERENCE_HIGH_THRESHOLD = 30.0; // % - High indoor/outdoor humidity difference

    // Air quality thresholds
    constexpr float CO_HIGH_THRESHOLD = 10.0;       // ppm - CO concentration alert (WHO 8-hour avg: 30 ppm)
    constexpr bool OZONE_ALERT_ON_DETECTION = true; // Alert when ozone is detected (digital sensor)

    // Energy consumption thresholds
    constexpr float POWER_HIGH_THRESHOLD = 1000.0;   // W - High power consumption alert
    constexpr float DAILY_COST_HIGH_THRESHOLD = 6.0; // $ - High daily cost alert (~$6 SGD for 24h operation)
}