# ESP32 sACN to DMX Converter

Ein Projekt zum Empfang von sACN (E1.31) über Ethernet und Ausgabe über DMX512 per UART.

## Features
- sACN Multicast Empfang (Universe konfigurierbar)
- DMX-Ausgabe über UART2 (Pin 4)
- OTA-Updates via ArduinoOTA
- Webinterface zur Universe-Einstellung & Statusanzeige
- Statische IP-Konfiguration

## Hardware
- WT32-ETH01 oder ESP32 mit LAN8720