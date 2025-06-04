# ESP32 sACN to DMX Interface

This project provides an ESP32-based solution to receive sACN (E1.31) over Ethernet and output standard DMX512 using a UART port.

## Features

- sACN (E1.31) multicast reception
- DMX output via UART (RS485 via external transceiver)
- Configurable universe (saved in flash)
- Web interface for configuration and live status
- OTA updates (ArduinoOTA)
- Static IP configuration
- Compatible with PlatformIO

## Hardware

- WT32-ETH01 (ESP32 with LAN8720)
- RS485 transceiver (e.g., MAX485 or SN75176)
- Optional: Debug LED

## Pin Configuration

- **ETH Power Pin:** GPIO16
- **ETH MDC:** GPIO23
- **ETH MDIO:** GPIO18
- **DMX TX Pin:** GPIO4 (UART2)

## Web Interface

    Default-Static-IP: 10.0.0.10

Access the ESP32 via its IP address in a browser to:

- View current DMX universe
- Set a new universe
- Monitor link and DMX status
- Check device version

<img src="https://github.com/jpsybru/esp32-sacn-dmx/blob/master/webserver01.png">

## Flashing

Use [PlatformIO](https://platformio.org/) or Arduino IDE to flash the project. Ensure you select the correct board (ESP32 Dev Module or WT32-ETH01).

## License

MIT License

## Author

by pJ BursT
