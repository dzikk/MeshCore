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

bool radio_init() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== ESP32C3 SuperMini RFM95W BOOT TEST ===");
  Serial.println("radio_init() entered");
  Serial.println("Skipping RFM95W init for boot test");

  fallback_clock.begin();
  rtc_clock.begin(Wire);

  return true;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
