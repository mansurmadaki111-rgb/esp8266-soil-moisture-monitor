# ESP8266 Soil Moisture Monitor

A lightweight IoT prototype that reads live soil moisture status from a sensor connected to an ESP8266 and sends it as JSON to a remote server over HTTP. Built as a test bed to validate a full sensor-to-cloud communication pipeline (WiFi, JSON serialization, HTTP POST) before scaling the same pattern into a larger IoT project.

## Overview

The ESP8266 continuously reads an analog soil moisture sensor, converts the raw reading into a percentage and a human-readable status (`dry`, `moist`, `wet`), packages it into a JSON payload, and POSTs it to a configurable server endpoint at a set interval.

This project was used to confirm:
- A microcontroller can reliably read and interpret analog sensor data
- Sensor data can be structured as JSON on-device
- The device can successfully reach a public server over the internet (not just a local network)
- The communication pattern is portable — the same payload structure and POST logic can point at any compatible backend with a one-line change

## Features

- Reads analog soil moisture data and classifies it into `dry` / `moist` / `wet`
- Sends structured JSON over HTTP POST at a configurable interval
- Minimal, dependency-light Arduino sketch — no unnecessary libraries
- Server endpoint is a single configurable constant, making it easy to swap between test and production backends

## Hardware Used

- ESP8266 (NodeMCU)
- soil moisture sensor module
- USB cable for power/programming
- WiFi network with internet access

## Wiring

| Sensor Pin | ESP8266 Pin |
|---|---|
| VCC | 3.3V (check your module's spec — some require 5V like mine) |
| GND | GND |
| AO (analog out) | A0 |

## JSON Payload Format

```json
{
  "device_id": "soil-sensor-1",
  "raw_value": 512,
  "moisture_percent": 45,
  "status": "moist"
}
```

| Field | Type | Description |
|---|---|---|
| `device_id` | string | Identifier for the specific sensor unit |
| `raw_value` | integer | Raw analog reading (0–1023) |
| `moisture_percent` | integer | Calculated percentage, 0–100 |
| `status` | string | One of `dry`, `moist`, `wet` |

## Setup & Installation

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and the ESP8266 board package.
2. Install the required library via Library Manager:
   - `ArduinoJson` (v6 or later)
3. Open the `.ino` file in this repo(esp8266_soil_moisture_https).
4. Update the configuration constants at the top of the file:
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   const char* SERVER_URL    = "https://your-endpoint-here";
   ```
5. Calibrate the sensor thresholds for your specific hardware:
   ```cpp
   const int DRY_VALUE = 800;  // raw reading in dry air
   const int WET_VALUE = 300;  // raw reading fully in water
   ```
   Take real readings with your sensor in dry air and in water, then update these two constants accordingly.
6. Select your board and port under **Tools** in the Arduino IDE.
7. Upload the sketch.
8. Open the Serial Monitor at **115200 baud** to view live readings and HTTP responses.

## How It Works

1. The device connects to WiFi on boot.
2. Every `SEND_INTERVAL_MS` (default: 10 seconds), it reads the analog sensor.
3. The raw reading is converted into a percentage and status label.
4. The data is serialized into a JSON payload using `ArduinoJson`.
5. The payload is sent via HTTP POST with a `Content-Type: application/json` header.
6. The server's response code and body are printed to Serial for confirmation.

## Testing Without a Live Backend

During development, this project was tested against:
- A local Python HTTP server (built with the standard library, no dependencies) — used to confirm the POST request and JSON structure locally.
- [webhook.site](https://webhook.site) — used to confirm the device could reach a real public endpoint over the open internet.

This made it possible to validate the entire pipeline before the production backend was available.

## License

MIT — feel free to use or adapt this for your own prototypes.

## Author

**Mansur Isah**
Exploring embedded systems and IoT - connecting microcontrollers to real-world sensors and cloud services.
