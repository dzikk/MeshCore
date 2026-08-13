#include <Arduino.h>
#include "target.h"

XiaoC3Board board;

#if defined(P_LORA_SCLK)
  static SPIClass spi;
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_0, P_LORA_RESET, P_LORA_DIO_1, spi);
#else
  RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_0, P_LORA_RESET, P_LORA_DIO_1);
#endif

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

#if ENV_INCLUDE_GPS
  #include <helpers/sensors/MicroNMEALocationProvider.h>
  MicroNMEALocationProvider nmea = MicroNMEALocationProvider(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors = EnvironmentSensorManager(nmea);
#else
  EnvironmentSensorManager sensors;
#endif

static uint8_t sx127x_read_reg(uint8_t addr) {
  digitalWrite(P_LORA_NSS, LOW);
  delayMicroseconds(5);

  spi.transfer(addr & 0x7F);
  uint8_t val = spi.transfer(0x00);

  delayMicroseconds(5);
  digitalWrite(P_LORA_NSS, HIGH);

  return val;
}

bool radio_init() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== ESP32C3 SuperMini RFM95W RADIO TEST ===");
  Serial.println("radio_init() entered");

  Serial.println("Pins:");
  Serial.print("SCLK: "); Serial.println(P_LORA_SCLK);
  Serial.print("MISO: "); Serial.println(P_LORA_MISO);
  Serial.print("MOSI: "); Serial.println(P_LORA_MOSI);
  Serial.print("NSS : "); Serial.println(P_LORA_NSS);
  Serial.print("RST : "); Serial.println(P_LORA_RESET);
  Serial.print("DIO0: "); Serial.println(P_LORA_DIO_0);
  Serial.print("DIO1: "); Serial.println(P_LORA_DIO_1);

  fallback_clock.begin();
  rtc_clock.begin(Wire);

#if defined(P_LORA_SCLK)
  Serial.println("Starting SPI...");
  spi.begin(P_LORA_SCLK, P_LORA_MISO, P_LORA_MOSI);

  pinMode(P_LORA_NSS, OUTPUT);
digitalWrite(P_LORA_NSS, HIGH);

pinMode(P_LORA_RESET, OUTPUT);

digitalWrite(P_LORA_RESET, LOW);
delay(20);
digitalWrite(P_LORA_RESET, HIGH);
delay(100);

Serial.println("Reading SX127x RegVersion...");
uint8_t ver = sx127x_read_reg(0x42);

Serial.print("SX127x RegVersion = 0x");
Serial.println(ver, HEX);
`
  
  Serial.println("Calling radio.std_init(&spi)...");
  bool ok = radio.std_init(&spi);
#else
  Serial.println("Calling radio.std_init()...");
  bool ok = radio.std_init();
#endif

  Serial.print("radio.std_init result: ");
  Serial.println(ok ? "OK" : "FAILED");

  if (!ok) {
    Serial.println("RFM95W init failed. Keeping firmware alive for debug.");
    return true;
  }

  Serial.println("RFM95W init OK.");
  return true;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
