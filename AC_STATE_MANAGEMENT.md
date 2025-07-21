# AC State Management and Energy Estimation

## Overview

The AIRIA system now uses a discrete AC state management approach instead of continuous duty cycle estimation. This provides much more realistic energy consumption estimates that match actual residential air conditioning usage patterns.

## AC States

The system tracks four distinct AC states:

### 1. OFF

- AC is completely turned off
- Power consumption: 0 watts
- No energy usage tracking
- User must manually turn AC on

### 2. STARTING

- AC has just been turned on
- High power draw due to compressor startup
- Duration: 5 seconds (configurable)
- Power consumption: ~180% of normal running power
- Automatically transitions to RUNNING state

### 3. RUNNING  

- AC is actively cooling the space
- Full power consumption based on thermal load
- Compressor and fan both operating
- Switches to IDLE when target temperature is reached

### 4. IDLE

- AC is on but target temperature is maintained
- Only fan is running (compressor off)  
- Low power consumption (~150W)
- Returns to RUNNING if temperature drifts too far from target

## Key Features

### Realistic Usage Tracking

- Tracks actual runtime hours per day
- Accumulates real energy consumption
- Handles periods when AC is completely off
- Provides both actual and projected daily usage

### Smart State Transitions

- Temperature deadband prevents frequent cycling
- Automatic state management based on thermal conditions
- Manual on/off control for realistic user behavior

### Accurate Energy Estimates

- Zero consumption when AC is off
- Startup energy spikes included
- Fan-only power consumption during idle periods
- Real-time power consumption based on current state

## Usage

### Manual Control

```cpp
// Turn AC on (will start in STARTING state)
energyEstimator.setACOn();

// Turn AC off completely  
energyEstimator.setACOff();
```

### State Monitoring

```cpp
// Check current state
ACPowerState state = energyEstimator.getACState();

// Get today's actual runtime
float hoursToday = energyEstimator.getTodaysRuntimeHours();

// Get actual energy consumed today
float actualKWh = energyEstimator.getTodaysEnergyKWh();

// Get projected daily consumption
float projectedKWh = energyEstimator.getDailyEnergyKWh();
```

### Display Information

The display now shows:

- Current AC state (OFF/STARTING/RUNNING/IDLE)
- Current power consumption (0W when off)
- Today's actual energy consumption
- Total runtime hours today
- Duty cycle percentage (runtime vs total day)

## Configuration Parameters

Key parameters in `Config.h`:

```cpp
// State management
constexpr uint32_t AC_STARTUP_TIME_MS = 5000;       // Startup duration
constexpr float AC_STARTUP_POWER_MULTIPLIER = 1.8;  // Startup power spike
constexpr float AC_FAN_ONLY_POWER_WATTS = 150.0;    // Fan-only power
constexpr float TEMP_DEADBAND = 0.5;                // Temperature control deadband
```

## Benefits

1. **Realistic Usage Patterns**: Handles actual on/off behavior instead of assuming continuous operation
2. **Accurate Energy Tracking**: Zero consumption when off, actual consumption when on
3. **Better User Feedback**: Shows real runtime hours and energy usage
4. **Startup Modeling**: Includes energy spikes when AC starts up
5. **Daily Reset**: Automatically resets daily statistics every 24 hours

This approach provides much more accurate energy estimates that match real residential air conditioning usage patterns, where ACs are frequently turned completely off for hours or days at a time.
