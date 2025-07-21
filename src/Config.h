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
    constexpr char THINGSBOARD_MQTT_SERVER[] = "demo.thingsboard.io";
    constexpr int THINGSBOARD_MQTT_PORT = 1883;
    constexpr char THINGSBOARD_ACCESS_TOKEN[] = "AYWEm8zZNwkdV9RppN8j";
    constexpr char THINGSBOARD_CLIENT_ID[] = "ESP8266Client";
    constexpr char THINGSBOARD_TELEMETRY_TOPIC[] = "v1/devices/me/telemetry";
    constexpr uint8_t THINGSBOARD_QOS = 1;                      // QoS level for MQTT messages
    constexpr uint32_t THINGSBOARD_UPLOAD_INTERVAL_MS = 30'000; // Upload every 30 seconds
    constexpr uint32_t MQTT_RECONNECT_DELAY_MS = 5'000;         // MQTT reconnection delay

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
    constexpr uint16_t BUZZER_BEEP_DURATION_MS = 200;   // Individual beep duration (shorter for consistency)
    constexpr uint16_t BUZZER_BEEP_INTERVAL_MS = 300;   // Interval between beeps (shorter for better timing)
    constexpr uint8_t BUZZER_BEEP_COUNT = 3;            // Number of beeps per alert cycle

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
    constexpr uint32_t ENERGY_CALC_REFRESH_MS = 10'000;   // Calculate energy every 10 seconds
    constexpr float TARGET_INDOOR_TEMP = 24.0;            // Target indoor temperature in Celsius
    constexpr float AC_BASE_POWER_WATTS = 1200.0;         // Base power consumption of AC unit (4.2kW cooling ÷ 3.5 COP)
    constexpr float AC_MIN_POWER_WATTS = 350.0;           // Minimum power (1.2kW cooling + 38W indoor unit)
    constexpr float AC_MAX_POWER_WATTS = 1800.0;          // Maximum power consumption (6.3kW cooling ÷ 3.5 COP)
    constexpr float TEMP_LOAD_FACTOR = 0.05;              // Power increase factor per degree difference
    constexpr float EFFICIENCY_DEGRADATION_FACTOR = 0.02; // Efficiency loss per degree difference
    constexpr float ELECTRICITY_RATE_PER_KWH = 0.25;      // Electricity rate in $/kWh (adjust for your region)

    /* Unit Conversion Constants --------------------------------- */
    constexpr float MILLIS_TO_SECONDS = 1000.0;    // Convert milliseconds to seconds
    constexpr float SECONDS_TO_HOURS = 3600.0;     // Convert seconds to hours
    constexpr float WATTS_TO_KILOWATTS = 1000.0;   // Convert watts to kilowatts
    constexpr float HOURS_PER_DAY = 24.0;          // Hours in a day
    constexpr uint8_t PERCENTAGE_MULTIPLIER = 100; // Convert decimal to percentage
    constexpr float WATTS_TO_BTU_HR = 3.412;       // Conversion factor

    /* Room Characteristics -------------------------------------- */
    constexpr float ROOM_HEAT_TRANSFER_COEFF = 3.5; // W/m²·K - Overall heat transfer coefficient
    constexpr float ROOM_SURFACE_AREA = 45.0;       // m² - Total surface area (walls, ceiling, floor)
    constexpr float ROOM_AIR_VOLUME = 75.0;         // m³ - Room air volume

    /* Heat Load Calculation Parameters -------------------------- */
    constexpr float LATENT_HEAT_FACTOR = 0.012;     // W·s/m³·%RH - Latent heat removal factor
    constexpr float LATENT_HEAT_TEMP_FACTOR = 0.03; // Temperature effect on latent heat

    /* COP (Coefficient of Performance) Calculation ------------- */
    constexpr float AC_CARNOT_EFFICIENCY = 0.50;   // Real COP as fraction of Carnot COP (R32 systems are more efficient)
    constexpr float COP_DEGRADATION_FACTOR = 0.02; // COP reduction per degree temperature difference
    constexpr float MIN_COP_DEGRADATION = 0.3;     // Minimum degradation factor (30%)
    constexpr float MIN_COP = 2.5;                 // Minimum realistic COP (R32 systems perform better)
    constexpr float MAX_COP = 7.0;                 // Maximum realistic COP (R32 can achieve higher efficiency)

    /* AC Unit Specific Parameters ------------------------------- */
    constexpr float AC_UNIT_EFFICIENCY_FACTOR = 0.85; // Overall AC unit efficiency (85%)

    /* Operating Condition Penalties ---------------------------- */
    constexpr float OPTIMAL_TEMP_DIFFERENCE = 8.0;       // °C - Optimal temperature difference
    constexpr float OPTIMAL_HUMIDITY = 60.0;             // % - Optimal outdoor humidity
    constexpr float TEMP_EFFICIENCY_PENALTY = 0.015;     // Efficiency penalty per degree above optimal
    constexpr float HUMIDITY_EFFICIENCY_PENALTY = 0.002; // Efficiency penalty per % humidity above optimal

    /* Duty Cycle Estimation Parameters ------------------------- */
    constexpr float MIN_DUTY_CYCLE = 0.2;            // Minimum duty cycle (20% for standby/fan)
    constexpr float MAX_DUTY_CYCLE = 0.95;           // Maximum duty cycle (95% - AC never runs 100%)
    constexpr float BASE_DUTY_CYCLE = 0.3;           // Base duty cycle for mild conditions
    constexpr float DUTY_CYCLE_TEMP_THRESHOLD = 2.0; // °C - Temperature difference threshold for increased duty
    constexpr float DUTY_CYCLE_LOAD_FACTOR = 0.035;  // Duty cycle increase per degree of outdoor temp difference
    constexpr float DUTY_CYCLE_CONTROL_FACTOR = 0.1; // Duty cycle correction per degree of indoor temp error

    /* AC State Management --------------------------------------- */
    constexpr uint32_t AC_STARTUP_TIME_MS = 5000;      // 5 seconds startup time
    constexpr float AC_STARTUP_POWER_MULTIPLIER = 1.8; // Power multiplier during startup (80% more power)
    constexpr float AC_FAN_ONLY_POWER_WATTS = 150.0;   // Power consumption when only fan is running
    constexpr float TEMP_DEADBAND = 0.5;               // °C - Temperature deadband for state switching
    constexpr float TEMP_DEADBAND_TOLERANCE = 0.5;     // °C - Additional tolerance for reactivation

    // =======================================================================
    // ALERT THRESHOLDS
    // =======================================================================

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