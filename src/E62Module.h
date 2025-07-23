// E62Module.h
#ifndef E62MODULE_H
#define E62MODULE_H

#include <Arduino.h>

class E62Module {
  public:
    E62Module(HardwareSerial& serial, uint8_t pinM0, uint8_t pinAUX);

    void begin(uint32_t baud);
    void enterConfigMode();
    void enterNormalMode();
    void waitForAUX(const char* label);

    bool getConfig(uint8_t* buffer);
    bool getVersion(uint8_t* buffer);
    void printConfig();

    void sendCommand3(uint8_t cmd);
    void resetModule();
    bool getRSSI(uint8_t* rssi, uint8_t* noise);

    // New setters based on updated datasheet
    void setFHS_ID(uint8_t id);          // Byte 1: Frequency-hopping sequence ID (default 0x01)
    void setFHSSNums(uint8_t nums);      // Byte 2: Number of frequency hopping channels (default 0x0A)

    void setParity(uint8_t p);           // Bits 7-6: 00=8N1, 01=8O1, 10=8E1, 11=8N1
    void setUARTBaud(uint8_t baud);      // Bits 5-3: 000=1200bps to 111=115200bps
    void setAirRate(uint8_t air);        // Bits 1-0: 00=16Kbps to 11=128Kbps

    void setSPED(uint8_t val);           // Set full SPED byte directly
    void setChannel(uint8_t ch);         // Byte 4: CHAN (0x00 to 0x33 → 425–450.5MHz)

    void setPower(uint8_t power);        // Bits 1-0: 00=30dBm to 11=21dBm
    void setFEC(bool enabled);           // Bit 2: FEC on/off
    void setIOMode(bool pushPull);       // Bit 6: Push-pull vs open-drain
    void setOPTION(uint8_t val);         // Set full OPTION byte directly

    void applyConfig();                  // Write config to flash (C0)
    void applyConfigVolatile();          // Write config temporarily (C2)
    const uint8_t* getConfigRaw();

  private:
    HardwareSerial& _serial;
    uint8_t _m0Pin;
    uint8_t _auxPin;
    uint8_t _config[5]; // ADDH, ADDL, SPED, CHAN, OPTION

    void waitForAUXInternal(const char* label);  // Used internally if needed
};

#endif // E62MODULE_H