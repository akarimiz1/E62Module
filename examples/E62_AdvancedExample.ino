#include "E62Module.h"

// --- تعریف پین‌های شما برای ESP32 ---
// ما از Serial2 برای ارتباط با ماژول استفاده می‌کنیم تا تداخل با Serial0 (مانیتور) نداشته باشد.
#define MODULE_SERIAL_PORT Serial2
#define E62_RX 16   // متصل به TX ماژول E62
#define E62_TX 17   // متصل به RX ماژول E62

// پین‌های کنترلی
#define E62_M0 19   
#define E62_AUX 18  

// --- نمونه‌سازی ماژول ---
E62Module e62(MODULE_SERIAL_PORT, E62_M0, E62_AUX);

// آرایه‌های موقت برای نگهداری داده‌های خوانده شده
uint8_t versionInfo[4];
uint8_t currentConfig[5];

void setup() {
  // 1. تنظیم مانیتور سریال (Serial0)
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("=================================================");
  Serial.println("  E62 Module Library Comprehensive Test Start");
  Serial.println("=================================================");
  
  // 2. تنظیم پورت سریال ماژول (Serial2) با پین‌های اختصاصی و 9600 bps
  MODULE_SERIAL_PORT.begin(9600, SERIAL_8N1, E62_RX, E62_TX);
  e62.begin(9600); 
  
  // =========================================================
  // تست ۱: خواندن اطلاعات پایه (Version و Config اولیه)
  // =========================================================
  Serial.println("\n--- [TEST 1] Basic Read Test (Version & Initial Config) ---");
  
  // خواندن ورژن ماژول (C3 C3 C3)
  if (e62.getVersion(versionInfo)) {
    Serial.printf("✅ Version Read: 0x%02X 0x%02X 0x%02X 0x%02X\n", 
                  versionInfo[0], versionInfo[1], versionInfo[2], versionInfo[3]);
  } else {
    Serial.println("❌ Version Read Failed (C3 C3 C3).");
  }

  // خواندن تنظیمات فعلی ماژول (C1 C1 C1)
  if (e62.getConfig(currentConfig)) {
    Serial.println("✅ Initial Config Read Success:");
    e62.printConfig();
  } else {
    Serial.println("❌ Initial Config Read Failed (C1 C1 C1).");
  }
  delay(2000);

  // =========================================================
  // تست ۲: خواندن قدرت سیگنال (RSSI)
  // =========================================================
  Serial.println("\n--- [TEST 2] RSSI Read Test (C5 C5 C5) ---");
  uint8_t rssi, noise;
  if (e62.getRSSI(&rssi, &noise)) {
    // مقدار RSSI و Noise به‌صورت دو بایت علامت‌دار (signed) برگردانده می‌شوند.
    Serial.printf("✅ RSSI Read Success! RSSI: %d dBm (0x%02X), Noise: %d dBm (0x%02X)\n", 
                  (int8_t)rssi, rssi, (int8_t)noise, noise);
  } else {
    Serial.println("❌ RSSI Read Failed (C5 C5 C5).");
  }
  delay(2000);
  
  // =========================================================
  // تست ۳: نوشتن دائمی تنظیمات (C0) و تأیید
  // =========================================================
  Serial.println("\n--- [TEST 3] Permanent Write Test (C0) ---");
  
  // ست کردن تنظیمات جدید و متمایز
  e62.setChannel(0x1A);    // کانال 26 (438 MHz)
  e62.setPower(2);         // توان متوسط (24dBm)
  e62.setUARTBaud(7);      // بالاترین Baud Rate (115200 bps) - فقط در حالت عادی اعمال می‌شود
  e62.setAirRate(3);       // بالاترین Air Rate (128Kbps)
  
  Serial.println("Intended Config (C0):");
  e62.printConfig(); 
  
  if (e62.applyConfig()) { // نوشتن دائمی و تأیید
    Serial.println("✅ Permanent Write Success & Verified.");
  } else {
    Serial.println("❌ Permanent Write FAILED or verification mismatch.");
  }
  delay(2000);

  // =========================================================
  // تست ۴: نوشتن موقت تنظیمات (C2) و تأیید
  // =========================================================
  Serial.println("\n--- [TEST 4] Volatile Write Test (C2) ---");
  
  // ست کردن تنظیمات موقت و متمایز (باید بعد از ریست پاک شود)
  e62.setChannel(0x05);    // کانال 5 (427.5 MHz)
  e62.setPower(0);         // بیشترین توان (30dBm)
  
  Serial.println("Intended Volatile Config (C2):");
  e62.printConfig(); 
  
  if (e62.applyConfigVolatile()) { // نوشتن موقت و تأیید
    Serial.println("✅ Volatile Write Success & Verified (Expected Failure if delay was not fixed).");
  } else {
    Serial.println("❌ Volatile Write FAILED or verification mismatch.");
  }
  delay(2000);
  
  // =========================================================
  // تست ۵: ریست ماژول و تأیید بازگشت به C0 (تنظیمات دائمی)
  // =========================================================
  Serial.println("\n--- [TEST 5] Reset Test & Final C0 Verification ---");
  e62.resetModule(); // ریست ماژول (C4 C4 C4)
  delay(3000); // صبر برای بوت مجدد ماژول

  Serial.println("Final Config Read after Reset:");
  if (e62.getConfig(nullptr)) {
    e62.printConfig();
  } else {
    Serial.println("❌ Final read failed after reset.");
  }
  
  // انتظار: تمام مقادیر باید مطابق با گام ۳ (C0: کانال 26، توان 24dBm) باشند، نه گام ۴ (C2).
  Serial.println("=================================================");
  Serial.println("  TEST COMPLETE. All functions checked.");
  Serial.println("=================================================");
}

void loop() {
  // این حلقه خالی می‌ماند.
}