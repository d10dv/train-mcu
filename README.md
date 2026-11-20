# Train MCU - ESP32 Onboard Locomotive Controller

This project implements an onboard controller for a model train based on the ESP32 microcontroller. It connects to the `train-control` remote unit via WiFi/MQTT to receive commands and report its state.

## 🚀 Features

- **WiFi Connectivity**: Operates as a Station (STA) connecting to the `TrainControl` access point.
- **MQTT Protocol**: 
  - Receives control commands (speed, direction, lights, horn).
  - Publishes real-time state (speed, direction, battery, uptime) with retained messages.
  - Supports "Announce" on connection and Last Will and Testament (LWT) for disconnection.
- **Motor Control**:
  - H-Bridge (PWM) control with configurable acceleration and deceleration ramps.
  - Maximum speed limiting.
  - Hardware E-Stop support.
- **Peripherals**:
  - 3-channel lighting control (Head, Tail, Cabin).
  - Piezo-buzzer horn with variable signal lengths.
- **Monitoring**:
  - Battery voltage monitoring via ADC (0-100%).
  - Periodic state reporting every 2 seconds.
- **Safety**:
  - **Connectivity Watchdog**: Automatically brings the train to a stop if MQTT connection is lost for more than 5 seconds.
  - **Safe Direction Switching**: Direction changes are only allowed when the train is stationary.
- **Configuration**: Persistent storage of Train ID and motor parameters in NVS (Non-Volatile Storage).

## 🔌 Schematic & Connections

### Block Diagram
```mermaid
graph TD
    subgraph Power_Supply
        BAT[LiPo Battery 7.4V-12V] --> L298_VCC[L298N VCC]
        BAT --> REG[Voltage Regulator 3.3V]
        REG --> ESP_3V3[ESP32 3.3V]
        BAT --> DIV[Voltage Divider]
        DIV --> ESP_G36[GPIO 36 ADC]
    end

    subgraph ESP32_Controller
        ESP_G25[GPIO 25]
        ESP_G26[GPIO 26]
        ESP_G27[GPIO 27 PWM]
        ESP_G18[GPIO 18]
        ESP_G19[GPIO 19]
        ESP_G17[GPIO 17 PWM]
        ESP_G32[GPIO 32]
        ESP_G33[GPIO 33]
        ESP_G13[GPIO 13]
        ESP_G14[GPIO 14 PWM]
    end

    subgraph Drive_&_Peripherals
        ESP_G25 --> L298_IN1[L298N IN1]
        ESP_G26 --> L298_IN2[L298N IN2]
        ESP_G27 --> L298_ENA[L298N ENA]
        L298_OUT[L298N OUT1/2] --> MOTOR[Train Motor 1]

        ESP_G18 --> L298_IN3[L298N IN3]
        ESP_G19 --> L298_IN4[L298N IN4]
        ESP_G17 --> L298_ENB[L298N ENB]
        L298_OUT2[L298N OUT3/4] --> MOTOR2[Train Motor 2]

        ESP_G32 --> LED_H[Head LED]
        ESP_G33 --> LED_T[Tail LED]
        ESP_G13 --> LED_C[Cabin LED]
        ESP_G14 --> BUZZER[Piezo Horn]
    end

    GND[Common GND] --- BAT
    GND --- L298_GND[L298N GND]
    GND --- ESP_GND[ESP32 GND]
```

### Connection Table (L298N Focus)

| L298N Pin | ESP32 Pin | Function |
|:---:|:---:|:---|
| **ENA** | GPIO 27 | Motor 1 PWM Speed Control |
| **IN1** | GPIO 25 | Motor 1 Direction Logic 1 |
| **IN2** | GPIO 26 | Motor 1 Direction Logic 2 |
| **ENB** | GPIO 17 | Motor 2 PWM Speed Control |
| **IN3** | GPIO 18 | Motor 2 Direction Logic 1 |
| **IN4** | GPIO 19 | Motor 2 Direction Logic 2 |
| **VCC** | Battery (+) | Motor Power (7-12V) |
| **GND** | GND | Common Ground |

Both motors are driven synchronously: one speed/direction command applies to both L298N channels.

## 🛠 Hardware Architecture

| Component | Signal | ESP32 GPIO (Default) |
|-----------|--------|----------------------|
| **Motors (L298)** | IN1 / IN2 / ENA (PWM) | 25 / 26 / 27 |
|                  | IN3 / IN4 / ENB (PWM) | 18 / 19 / 17 |
| **Lights**| Head / Tail / Cabin | 32 / 33 / 13 |
| **Horn**  | Piezo Buzzer (PWM) | 14 |
| **Battery**| Voltage Divider (ADC) | 36 (ADC1_CH0) |

*Note: GPIO pins can be reconfigured in component headers.*

### Planned Peripherals — Proposed Pinout

> ⚠️ **Not implemented yet**: there is currently no encoder or display driver in the codebase (no `encoder`/`display` components, nothing in `PLAN.md`). The pinout below is a **proposal for future wiring** — verify before soldering.

Used GPIOs: 13, 14, 17, 18, 19, 25, 26, 27, 32, 33, 36 → free: 4, 5, 15, 16, 21, 22, 23 (input-only 34/35/39 are avoided to keep internal pull-ups available).

| Peripheral | Signal | ESP32 GPIO | Notes |
|------------|--------|:---:|:---|
| **Rotary Encoder** (KY-040) | Channel A / Channel B | 13 / 14 | With internal pull-ups; ISR-capable |
| | Push button (SW) | 15 | Active-low, internal pull-up |
| **OLED Display** (SSD1306, I2C) | SDA / SCL | 21 / 22 | Default ESP32 I2C pins, `esp_lcd` SSD1306 driver available in ESP-IDF |

## 📂 Project Structure

- `main/`: Application entry point and system initialization.
- `components/`:
  - `wifi_sta`: WiFi station management.
  - `mqtt_client`: MQTT connectivity and message routing.
  - `motor_driver`: PWM motor control with speed ramping.
  - `light_ctrl`: GPIO-based lighting management.
  - `horn`: Buzzer signal generation.
  - `battery_monitor`: ADC voltage reading.
  - `cmd_handler`: JSON command parsing (cJSON).
  - `state_reporter`: Periodic status telemetry.
  - `train_config`: NVS-based configuration management.

## ⚙️ Building and Flashing

This project uses the **ESP-IDF** (v4.4 or newer) framework.

1. **Set the target** (e.g., esp32, esp32s3, esp32c3):
   ```bash
   idf.py set-target esp32
   ```

2. **Configure the project** (SSID, Password, MQTT Broker IP):
   ```bash
   idf.py menuconfig
   ```
   Navigate to `Train MCU Configuration`.

3. **Build and Flash**:
   ```bash
   idf.py build
   idf.py -p [PORT] flash monitor
   ```

## 📡 MQTT Protocol Summary

- **Commands**: `train/<id>/cmd` (JSON)
- **State**: `train/<id>/state` (JSON, Retained)
- **Announce**: `train/announce` (JSON, LWT)

Refer to `PLAN.md` for detailed JSON schemas.
