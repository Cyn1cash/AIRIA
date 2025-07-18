# AIRIA Energy Estimation Model Documentation

## Overview

This document describes the enhanced energy estimation model implemented in the AIRIA project for calculating air conditioning energy consumption based on indoor and outdoor environmental conditions.

## Model Components

### 1. Heat Load Calculation

#### Sensible Heat Load

- **Purpose**: Calculate heat transfer due to temperature difference
- **Formula**: Sensible Load = U × A × ΔT
- **Parameters**:
  - `U` = Overall heat transfer coefficient (W/m²·K)
  - `A` = Total surface area of conditioned space (m²)
  - `ΔT` = Temperature difference between indoor and outdoor (°C)

#### Latent Heat Load

- **Purpose**: Calculate energy needed to remove moisture from air
- **Formula**: Latent Load = ΔH × V × LHF × TF
- **Parameters**:
  - `ΔH` = Humidity difference (%)
  - `V` = Room air volume (m³)
  - `LHF` = Latent heat factor (W·s/m³·%RH)
  - `TF` = Temperature factor (increases with temperature)

### 2. Coefficient of Performance (COP) Calculation

#### Theoretical Foundation

- Based on Carnot cycle efficiency
- **Formula**: COP_carnot = T_cold / (T_hot - T_cold)
- Real COP = COP_carnot × AC_Carnot_Efficiency × Degradation_Factor

#### Degradation Factors

- **Temperature Difference**: Larger differences reduce efficiency
- **Operating Conditions**: High temperatures and humidity reduce COP
- **Equipment Age**: Factored into overall efficiency

### 3. Energy Efficiency Ratio (EER)

- **Formula**: EER = Heat_Load_BTU / Power_Consumption_Watts
- Higher EER indicates better efficiency
- Typical range: 8-15 BTU/Wh for residential AC units

## Configurable Parameters

### Room Characteristics

```cpp
ROOM_HEAT_TRANSFER_COEFF = 3.5   // W/m²·K - Wall insulation quality
ROOM_SURFACE_AREA = 45.0         // m² - Total room surface area
ROOM_AIR_VOLUME = 75.0           // m³ - Room volume
```

### AC Unit Specifications (Updated for R32 Refrigerant Systems)

```cpp
AC_BASE_POWER_WATTS = 1200.0     // Base power consumption (4.2kW cooling ÷ 3.5 COP)
AC_MIN_POWER_WATTS = 350.0       // Minimum power (1.2kW cooling + 38W indoor unit)
AC_MAX_POWER_WATTS = 1800.0      // Maximum power (6.3kW cooling ÷ 3.5 COP)
AC_UNIT_EFFICIENCY_FACTOR = 0.85 // Overall unit efficiency
AC_CARNOT_EFFICIENCY = 0.50      // Real vs theoretical efficiency (improved for R32)
```

### Performance Parameters (Enhanced for Modern AC Units)

```cpp
MIN_COP = 2.5                    // Minimum realistic COP (R32 systems perform better)
MAX_COP = 7.0                    // Maximum realistic COP (R32 can achieve higher efficiency)
COP_DEGRADATION_FACTOR = 0.02    // COP reduction per °C difference
MIN_COP_DEGRADATION = 0.3        // Minimum degradation factor (30%)
OPTIMAL_TEMP_DIFFERENCE = 8.0    // °C - Optimal operating difference
OPTIMAL_HUMIDITY = 60.0          // % - Optimal outdoor humidity
```

### Duty Cycle Parameters (New)

```cpp
MIN_DUTY_CYCLE = 0.2             // Minimum duty cycle (20% for standby/fan)
MAX_DUTY_CYCLE = 0.95            // Maximum duty cycle (95% - AC never runs 100%)
BASE_DUTY_CYCLE = 0.3            // Base duty cycle for mild conditions
DUTY_CYCLE_TEMP_THRESHOLD = 2.0  // °C - Temperature difference threshold
DUTY_CYCLE_LOAD_FACTOR = 0.035   // Duty cycle increase per degree
```

## Calibration Guide

### For Different Room Types

1. **Small Room (< 20 m²)**:
   - Reduce `ROOM_SURFACE_AREA` and `ROOM_AIR_VOLUME`
   - Adjust `AC_BASE_POWER_WATTS` to match AC unit rating

2. **Large Room (> 50 m²)**:
   - Increase surface area and volume parameters
   - May need higher power ratings

3. **Different Insulation Quality**:
   - **Good Insulation**: Reduce `ROOM_HEAT_TRANSFER_COEFF` to 2.0-2.5
   - **Poor Insulation**: Increase to 4.0-5.0

### For Different AC Units

1. **Window/Portable AC**:
   - Lower `AC_CARNOT_EFFICIENCY` (0.35-0.40)
   - Adjust power ratings accordingly
   - Typically lower MAX_COP (5.0-6.0)

2. **Split System with R32 Refrigerant**:
   - Higher `AC_CARNOT_EFFICIENCY` (0.50-0.55)
   - Better overall efficiency
   - Can achieve higher COP values (up to 7.0)

3. **Older Split Systems (R22/R410A)**:
   - Lower `AC_CARNOT_EFFICIENCY` (0.40-0.45)
   - Reduce MAX_COP to 5.5-6.0
   - May require higher power ratings

4. **Central AC**:
   - Highest efficiency parameters
   - Consider ductwork losses (reduce efficiency factor)
   - Account for multiple zones

## Validation Methods

### 1. Compare with Actual Bills

- Monitor actual energy consumption
- Adjust `AC_UNIT_EFFICIENCY_FACTOR` based on real data

### 2. Temperature Logging

- Log indoor/outdoor temperatures
- Verify COP calculations against manufacturer specs

### 3. Humidity Impact

- Monitor correlation between humidity and energy use
- Adjust `HUMIDITY_EFFICIENCY_PENALTY` if needed

## Accuracy Improvements

### Real-Time Calibration

- Track actual vs predicted consumption
- Implement learning algorithm for parameter adjustment

### Seasonal Adjustments

- Account for changing AC efficiency with age
- Adjust for seasonal variations in performance

### Advanced Sensors

- Add power monitoring for validation
- Include air quality sensors for comprehensive analysis

## Expected Accuracy

- **Typical Accuracy**: ±15-25% of actual consumption
- **With Calibration**: ±10-15% of actual consumption  
- **Best Case**: ±5-10% with proper calibration and validation
- **R32 Systems**: Generally more predictable due to better efficiency characteristics

## Recent Model Improvements

### Enhanced COP Calculation

- **R32 Refrigerant Support**: Better efficiency modeling for modern systems
- **Duty Cycle Integration**: More realistic power consumption patterns
- **Temperature-Based Degradation**: Improved accuracy at extreme conditions

### Alert System Integration

- **Real-Time Monitoring**: Continuous tracking of energy consumption
- **Threshold Alerts**: Configurable warnings for high consumption
- **Cost Tracking**: Daily cost estimates with alert capabilities

### Advanced Features

- **Multi-Sensor Support**: CO and ozone sensors for comprehensive monitoring
- **Visual Interface**: Nextion display integration for real-time feedback
- **Non-Intrusive Logging**: Alert system avoids Serial interference with display

## Usage Notes

- Model assumes cooling mode operation
- Heating mode would require different calculations (heat pump considerations)
- Regular calibration recommended for best accuracy
- Environmental factors (wind, solar gain) not included in base model
- R32 refrigerant systems may achieve higher efficiency than older R22/R410A systems
- Alert system provides early warning for unusual consumption patterns
