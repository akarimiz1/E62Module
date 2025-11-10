// E62Module.h (Final Stable Version)
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

    void resetModule();
    bool getRSSI(uint8_t* rssi, uint8_t* noise);

    // Setters
    void setFHS_ID(uint8_t id);
    void setFHSSNums(uint8_t nums);
    void setParity(uint8_t p);
    void setUARTBaud(uint8_t baud);
    void setAirRate(uint8_t air);
    void setSPED(uint8_t val);
    void setChannel(uint8_t ch);
    void setPower(uint8_t power);
    void setFEC(bool enabled);
    void setIOMode(bool pushPull);
    void setOPTION(uint8_t val);

    bool applyConfig();                  // Write config to flash (C0) and verify
    bool applyConfigVolatile();          // Write config temporarily (C2) and verify
    const uint8_t* getConfigRaw();

  private:
    HardwareSerial& _serial;
    uint8_t _m0Pin;
    uint8_t _auxPin;
    uint8_t _config[5]; // ADDH, ADDL, SPED, CHAN, OPTION

    void clearSerialBuffer(); // FIX: Utility for communication stability
    void sendCommand3(uint8_t cmd); // FIX: Utility to send commands
};

#endif // E62MODULE_H