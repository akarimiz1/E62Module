#include <BluetoothSerial.h>
#include <E62Module.h>

#define E62_RX 16
#define E62_TX 17
#define E62_M0 19
#define E62_AUX 18

BluetoothSerial SerialBT;
HardwareSerial E62Serial(2);
E62Module lora(E62Serial, E62_M0, E62_AUX);

#define BUFFER_SIZE 64
uint8_t buffer[BUFFER_SIZE + 1]; // last byte for RSSI

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_LoRa_Bridge_2");

  // Initialize LoRa module
  lora.begin(9600);  // match default baud rate

  // Optional: Set default parameters (you can skip if already configured)
  lora.setFHS_ID(0x01);
  lora.setFHSSNums(0x0A);
  lora.setParity(0);            // 8N1
  lora.setUARTBaud(3);          // 9600
  lora.setAirRate(2);           // 64Kbps
  lora.setChannel(10);          // 430 MHz
  lora.setPower(0);             // 30dBm
  lora.setFEC(true);
  lora.setIOMode(true);
  lora.applyConfig();           // save permanently

  // Print current config
  lora.getConfig(nullptr);
  lora.printConfig();

  Serial.println("✅ Bluetooth LoRa Bridge ready.");
}

void loop() {
  // Bluetooth -> LoRa
  if (SerialBT.available()) {
    String msg = SerialBT.readStringUntil('\n');
    E62Serial.write((uint8_t*)msg.c_str(), msg.length());
    Serial.printf("📤 Sent: %s\n", msg.c_str());
  }

  // LoRa -> Bluetooth with RSSI
  if (E62Serial.available() > 1 && digitalRead(E62_AUX)) {
    int len = E62Serial.readBytes(buffer, E62Serial.available());
    if (len > 1) {
      uint8_t rssi_raw = buffer[len - 1];
      int rssi_dbm = -rssi_raw;

      SerialBT.printf("[RSSI:%d dBm] ", rssi_dbm);
      SerialBT.write(buffer, len - 1);
      SerialBT.println();

      // Debug
      Serial.printf("📥 Received %d bytes | RSSI: %d dBm\n", len - 1, rssi_dbm);
    }
  }
}
