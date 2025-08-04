# AIRIA Configuration Guide

## Quick Setup for Your Room

### Step 1: Measure Your Room

1. **Room dimensions**: Length × Width × Height (meters)
2. **Calculate volume**: Length × Width × Height = ___ m³
3. **Calculate surface area**: 2×(L×W + L×H + W×H) = ___ m²

### Step 2: Check Your AC Unit

1. **Power rating**: Look for BTU/hr or kW rating on AC unit
2. **Type**: Window unit, split system, or central AC
3. **Age**: Newer units are typically more efficient

### Step 3: Update Config.h Parameters

```cpp
// Room characteristics (update these with your measurements)
constexpr float ROOM_SURFACE_AREA = 45.0;  // Your calculated surface area
constexpr float ROOM_AIR_VOLUME = 75.0;    // Your calculated volume

// AC unit specifications (update based on your unit)
constexpr float AC_BASE_POWER_WATTS = 1200.0;  // Your AC's rated power (updated for 4.2kW cooling ÷ 3.5 COP)
constexpr float AC_MIN_POWER_WATTS = 350.0;    // Minimum power (1.2kW cooling + 38W indoor unit)
constexpr float AC_MAX_POWER_WATTS = 1800.0;   // Maximum power (6.3kW cooling ÷ 3.5 COP)

// Local electricity rate (update for your area)
constexpr float ELECTRICITY_RATE_PER_KWH = 0.25;  // Your local rate
```

### Step 4: Sensor Pin Configuration

```cpp
// Sensor pins (adjust if using different GPIO pins)
constexpr uint8_t DHT22_PIN = 5;              // GPIO 5 (D1 on NodeMCU)
constexpr uint8_t MQ9_ANALOG_PIN = A0;        // Analog pin for MQ-9 CO sensor
constexpr uint8_t MQ131_DIGITAL_PIN = 4;      // GPIO 4 (D2 on NodeMCU) for ozone sensor
constexpr uint8_t BUZZER_PIN = 12;            // GPIO 12 (D6 on NodeMCU) for alert buzzer
```

### Step 5: Alert System Configuration

```cpp
// Alert thresholds (adjust based on your preferences)
constexpr float TEMP_HIGH_THRESHOLD = 30.0;       // °C - High temperature alert
constexpr float TEMP_LOW_THRESHOLD = 15.0;        // °C - Low temperature alert
constexpr float HUMIDITY_HIGH_THRESHOLD = 80.0;   // % - High humidity alert
constexpr float HUMIDITY_LOW_THRESHOLD = 30.0;    // % - Low humidity alert
constexpr float CO_HIGH_THRESHOLD = 30.0;         // ppm - CO alert (WHO 8-hour avg)
constexpr float POWER_HIGH_THRESHOLD = 1500.0;    // W - High power consumption alert
constexpr float DAILY_COST_HIGH_THRESHOLD = 8.0;  // $ - High daily cost alert

// Buzzer behavior (adjust timing as needed)
constexpr uint16_t BUZZER_BEEP_DURATION_MS = 200;   // Individual beep duration
constexpr uint16_t BUZZER_BEEP_INTERVAL_MS = 800;   // Interval between beeps
constexpr uint8_t BUZZER_BEEP_COUNT = 3;            // Number of beeps per alert cycle
```

### Step 6: Calibration Based on Room Type

#### Small Room (< 20 m²)

- Set `ROOM_HEAT_TRANSFER_COEFF = 3.0`
- Reduce power values by 20-30%

#### Medium Room (20-40 m²)

- Use default values as starting point
- `ROOM_HEAT_TRANSFER_COEFF = 3.5`

#### Large Room (> 40 m²)

- Set `ROOM_HEAT_TRANSFER_COEFF = 4.0`
- Increase power values by 20-30%

### Step 7: Insulation Quality

#### Good Insulation (new building, double-pane windows)

- Set `ROOM_HEAT_TRANSFER_COEFF = 2.5`

#### Average Insulation (typical residential)

- Set `ROOM_HEAT_TRANSFER_COEFF = 3.5`

#### Poor Insulation (old building, single-pane windows)

- Set `ROOM_HEAT_TRANSFER_COEFF = 5.0`

### Step 8: Test and Validate

1. **Run the system for 1 week**
2. **Compare with actual electricity bill**
3. **Adjust `AC_UNIT_EFFICIENCY_FACTOR`**:
   - If estimate is too high: Reduce to 0.7-0.8
   - If estimate is too low: Increase to 0.9-1.0
4. **Test alert system**:
   - Verify buzzer works when thresholds are exceeded
   - Check display shows alerts correctly

## Hardware Setup

### Required Components

- **ESP8266 NodeMCU** (or compatible)
- **DHT22** temperature/humidity sensor
- **MQ-9** CO sensor (optional)
- **MQ-131** ozone sensor (optional)
- **Active buzzer** (5V tolerant)
- **Nextion display** (for visual interface)

### Wiring Connections

```
DHT22:
- VCC → 3.3V
- GND → GND
- Data → GPIO 5 (D1)

MQ-9 CO Sensor:
- VCC → 5V (if available) or 3.3V
- GND → GND
- Analog → A0

MQ-131 Ozone Sensor:
- VCC → 3.3V
- GND → GND
- Digital → GPIO 4 (D2)

Buzzer:
- Positive → GPIO 12 (D6)
- Negative → GND

Nextion Display:
- VCC → 5V
- GND → GND
- TX → RX (GPIO 3)
- RX → TX (GPIO 1)
```

## Alert System Features

### Monitored Parameters

1. **Temperature**: High (>30°C) and Low (<15°C) alerts
2. **Humidity**: High (>80%) and Low (<30%) alerts
3. **Air Quality**: CO concentration (>30 ppm) and ozone detection
4. **Energy**: High power consumption (>1500W) and daily cost (>$8.00)

### Alert Behavior

- **Visual**: Alerts displayed on Nextion screen
- **Audio**: Buzzer produces 3 beeps with pauses when alerts are active
- **Automatic**: Alerts clear when values return to normal ranges

### Customizing Alert Thresholds

Adjust thresholds in `Config.h` based on your specific needs:

- **Climate**: Tropical regions may need higher humidity thresholds
- **Health**: Lower CO thresholds for sensitive individuals
- **Budget**: Adjust cost thresholds based on electricity rates

## Expected Results

### Typical Power Consumption

- **Small room**: 600-1000W (updated for higher efficiency R32 systems)
- **Medium room**: 1000-1400W  
- **Large room**: 1400-1800W

### Typical Daily Energy

- **Small room**: 14-24 kWh/day
- **Medium room**: 24-34 kWh/day
- **Large room**: 34-43 kWh/day

### Factors That Increase Energy Use

- High outdoor temperature (> 30°C)
- High humidity (> 70%)
- Poor insulation
- Large temperature difference (> 10°C)
- Older AC units

## Troubleshooting

### Estimates Too High?

- Check if `AC_MAX_POWER_WATTS` is set too high (default: 1800W)
- Reduce `AC_UNIT_EFFICIENCY_FACTOR` to 0.7-0.8
- Verify room measurements are correct

### Estimates Too Low?

- Check if `AC_MIN_POWER_WATTS` is set too low (default: 350W)
- Increase `ROOM_HEAT_TRANSFER_COEFF` for poor insulation
- Verify AC unit power rating

### Sensor Issues?

- Check DHT22 wiring (VCC, GND, Data to GPIO 5)
- Verify sensor is not in direct sunlight
- Check for stable readings (avoid using Serial.println with Nextion)

### Alert System Issues?

- **Buzzer not working**: Check GPIO 12 (D6) connection and buzzer polarity
- **No alerts showing**: Verify Nextion display has `main.alerts.txt` object
- **False alerts**: Adjust thresholds in Config.h for your environment
- **Alerts not clearing**: Check sensor readings are stable and within normal ranges

### Gas Sensor Issues?

- **MQ-9 (CO sensor)**: Allow 20-second warmup time, uses only analog reading on A0 for accurate PPM measurement
- **MQ-131 (Ozone sensor)**: Allow 30-second warmup time, digital reading on GPIO 4
- **Inaccurate readings**: Sensors may need calibration in clean air environment
