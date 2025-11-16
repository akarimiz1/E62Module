// E62Module.cpp (Final Stable Version)
#include "E62Module.h"
#include <string.h> 

// --- Utility Functions ---

void E62Module::clearSerialBuffer() {
    // FIX: Completely drain the serial buffer
    while (_serial.available()) _serial.read();
}

void E62Module::sendCommand3(uint8_t cmd) {
    clearSerialBuffer(); // FIX: Clear buffer before sending command block
    for (int i = 0; i < 3; i++) _serial.write(cmd);
}

// --- Constructor and Mode Switching ---

E62Module::E62Module(HardwareSerial& serial, uint8_t pinM0, uint8_t pinAUX)
  : _serial(serial), _m0Pin(pinM0), _auxPin(pinAUX) {
  pinMode(_m0Pin, OUTPUT);
  pinMode(_auxPin, INPUT);
  digitalWrite(_m0Pin, LOW);
  memset(_config, 0, sizeof(_config));
}

void E62Module::begin(uint32_t baud) {
  _serial.begin(baud);
  waitForAUX("begin()");
}

void E62Module::enterConfigMode() {
  digitalWrite(_m0Pin, HIGH);
  delay(10); // FIX: Increased M0 delay to 100ms
  waitForAUX("EnterConfigMode");
}

void E62Module::enterNormalMode() {
  digitalWrite(_m0Pin, LOW);
  delay(10); // FIX: Increased M0 delay to 100ms
  waitForAUX("EnterNormalMode");
}

void E62Module::waitForAUX(const char* label) {
  unsigned long start = millis();
  while (!digitalRead(_auxPin)) {
    if (millis() - start > 300) { // FIX: Increased AUX timeout to 1500ms
      Serial.printf("%s: ❌ Timeout waiting for AUX HIGH\n", label);
      return;
    }
  }
  Serial.printf("%s: ✅ AUX is now HIGH \n", label);
}

// --- Read Functions ---

bool E62Module::getConfig(uint8_t* buffer) {
  enterConfigMode();
  sendCommand3(0xC1);
  
  delay(10); // FIX: Critical delay of 500ms for stable C1 read on ESP32
  
  bool success = false;
  if (_serial.available() >= 6) {
    uint8_t header = _serial.read();
    if (header == 0xC0) { // FIX: Check for the correct header (0xC0)
      for (int i = 0; i < 5; i++) {
        _config[i] = _serial.read();
      }
      if (buffer) memcpy(buffer, _config, 5);
      success = true;
    } else {
      Serial.println("getConfig: ❌ Invalid header received!");
    }
  }
  
  clearSerialBuffer(); 
  enterNormalMode();
  return success;
}

bool E62Module::getVersion(uint8_t* buffer) {
  enterConfigMode();
  sendCommand3(0xC3);
  delay(10);
  
  bool success = false;
  if (_serial.available() >= 4) {
    for (int i = 0; i < 4; i++) {
      buffer[i] = _serial.read();
    }
    success = true;
  }
  
  clearSerialBuffer();
  enterNormalMode();
  return success;
}

void E62Module::resetModule() {
  enterConfigMode();
  sendCommand3(0xC4);
  delay(10); 
  enterNormalMode();
}

bool E62Module::getRSSI(uint8_t* rssi, uint8_t* noise) {
  enterConfigMode();
  sendCommand3(0xC5);
  delay(10);
  
  bool success = false;
  if (_serial.available() >= 3) {
    if (_serial.read() == 0xC5) {
      *rssi = _serial.read();
      *noise = _serial.read();
      success = true;
    }
  }
  
  clearSerialBuffer();
  enterNormalMode();
  return success;
}

// --- Setters (Unchanged) ---
void E62Module::setFHS_ID(uint8_t id) { _config[0] = id; }
void E62Module::setFHSSNums(uint8_t nums) { _config[1] = nums; }
void E62Module::setParity(uint8_t p) { _config[2] = (_config[2] & 0x3F) | ((p & 0x03) << 6); }
void E62Module::setUARTBaud(uint8_t baud) { _config[2] = (_config[2] & 0xC7) | ((baud & 0x07) << 3); }
void E62Module::setAirRate(uint8_t air) { _config[2] = (_config[2] & 0xF8) | (air & 0x07); }
void E62Module::setSPED(uint8_t val) { _config[2] = val; }
void E62Module::setChannel(uint8_t ch) { _config[3] = ch; }
void E62Module::setPower(uint8_t p) { _config[4] = (_config[4] & 0xFC) | (p & 0x03); }
void E62Module::setFEC(bool enabled) {
  if (enabled) _config[4] |= 0x04;
  else _config[4] &= ~0x04;
}
void E62Module::setIOMode(bool pushPull) {
  if (pushPull) _config[4] |= 0x40;
  else _config[4] &= ~0x40;
}
void E62Module::setOPTION(uint8_t val) { _config[4] = val; }

// --- Write Functions with Verification (Final Fix) ---

bool E62Module::applyConfig() {
  uint8_t intendedConfig[5];
  memcpy(intendedConfig, _config, 5);

  enterConfigMode();
  clearSerialBuffer();
  
  _serial.write(0xC0);
  _serial.write(intendedConfig, 5);
  
  delay(10); // FIX: 1000ms delay for writing to flash (C0)
  
  uint8_t readBackConfig[5];
  if (!getConfig(readBackConfig)) {
    Serial.println("applyConfig: ❌ Failed to read config for verification.");
    return false;
  }
  
  if (memcmp(intendedConfig, readBackConfig, 5) == 0) {
    Serial.println("applyConfig: ✅ Configuration successfully written and verified.");
    return true;
  } else {
    Serial.println("applyConfig: ⚠️ Verification failed! Read back config does NOT match intended config.");
    return false;
  }
}

bool E62Module::applyConfigVolatile() {
  uint8_t intendedConfig[5];
  memcpy(intendedConfig, _config, 5);

  enterConfigMode();
  clearSerialBuffer();
  
  _serial.write(0xC2);
  _serial.write(intendedConfig, 5);
  
  // FIX: Force a mode cycle to stabilize the module after C2's rapid processing
  enterNormalMode(); 
  delay(10); // Short delay after M0=LOW for a clean AUX transition (optional)
  
  uint8_t readBackConfig[5];
  
  // getConfig() handles re-entering Config Mode (M0=HIGH)
  if (!getConfig(readBackConfig)) {
    Serial.println("applyConfigVolatile: ❌ Failed to read config for verification.");
    return false;
  }
  
  if (memcmp(intendedConfig, readBackConfig, 5) == 0) {
    Serial.println("applyConfigVolatile: ✅ Configuration successfully written and verified.");
    return true;
  } else {
    Serial.println("applyConfigVolatile: ⚠️ Verification failed! Read back config does NOT match intended config.");
    return false;
  }
}

const uint8_t* E62Module::getConfigRaw() {
  return _config;
}

void E62Module::printConfig() {
  float freq = 425.0 + (_config[3] * 0.5);
  const char* parityStr[] = { "8N1", "8O1", "8E1", "8N1" };
  const uint32_t uartBaudTable[] = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };
  const char* airRateStr[] = { "16Kbps", "32Kbps", "64Kbps", "128Kbps"};
  const char* powerStr[] = { "30dBm", "27dBm", "24dBm", "21dBm" };

  uint8_t parity = (_config[2] >> 6) & 0x03;
  uint8_t uartBaud = (_config[2] >> 3) & 0x07;
  uint8_t airRate = _config[2] & 0x07;
  uint8_t power = _config[4] & 0x03;

  Serial.println("📟 E62 Module Config:");
  Serial.printf("  FHS ID (ADDH): %d (0x%02X) \n", _config[0], _config[0]);
  Serial.printf("  FHSS nums (ADDL): %d (0x%02X) \n", _config[1], _config[1]);
  Serial.printf("  SPED: 0x%02X \n", _config[2]);
  Serial.printf("    ├─ Parity    : %s \n", parityStr[parity]);
  Serial.printf("    ├─ UART Baud : %lu bps \n", uartBaudTable[uartBaud]);
  Serial.printf("    └─ Air Rate  : %s \n", airRateStr[airRate]);

  Serial.printf("  CHAN: %d (%.1f MHz) \n", _config[3], freq);
  Serial.printf("  OPTION: 0x%02X \n", _config[4]);
  Serial.printf("    ├─ Power     : %s \n", powerStr[power]);
  Serial.printf("    ├─ FEC       : %s \n", (_config[4] & 0x04) ? "Enabled" : "Disabled");
  Serial.printf("    ├─ IO Mode   : %s \n", (_config[4] & 0x40) ? "Push-Pull" : "Open-Drain");
  Serial.printf("    └─ Pullups   : %s \n", (_config[4] & 0x20) ? "Enabled" : "Disabled");
}