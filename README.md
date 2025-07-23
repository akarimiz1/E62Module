# E62Module Arduino Library

This library allows full configuration and communication with EBYTE E62-433T30D LoRa modules using UART on ESP32.

## Features
- Configure SPED, channel, FEC, power, parity, IO mode
- Save or temporarily apply parameters
- Read version, RSSI, and module status
- AUX-aware UART handling
- Example Bluetooth-to-LoRa bridge with RSSI

## Example
```cpp
#include <E62Module.h>
E62Module lora(Serial2, 19, 18);
lora.begin(9600);
lora.setChannel(10);
lora.setFEC(true);
lora.applyConfig();
