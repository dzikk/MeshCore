#include <Arduino.h>
#include "target.h"

XiaoC3Board board;

/*
 * The Adafruit RFM95W breakout uses a 74HC4050 input buffer.
 * Start with a conservative SPI clock matching our successful
 * standalone RegVersion test.
 */
#if defined(P_LORA_SCLK)
  static SPIClass spi(VSPI);
  static SPISettings radio_spi_settings(
    100000,       // 100 kHz
    MSBFIRST,
    SPI_MODE0
  );

  RADIO_CLASS radio = new Module(
    P_LORA_NSS,
    P_LORA_DIO_0,
    P_LORA_RESET,
    P_LORA_DIO_1,
    spi,
    radio_spi_settings
  );
#else
  RADIO_CLASS radio = new Module(
    P_LORA_NSS,
    P_LORA_DIO_0,
    P_LORA_RESET,
    P_LORA_DIO_1
  );
#endif

WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);

#if ENV_INCLUDE_GPS
  #include <helpers/sensors/MicroNMEALocationProvider.h>
  MicroNMEALocationProvider nmea =
    MicroNMEALocationProvider(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors =
    EnvironmentSensorManager(nmea);
#else
  EnvironmentSensorManager sensors;
#endif

bool radio_init() {
  Serial.begin(115200);
  delay(2000);

  Serial.println();
  Serial.println("=== ESP32-C3 SUPERMINI RFM95W INIT ===");

  Serial.print("SCK:  ");
  Serial.println(P_LORA_SCLK);
  Serial.print("MISO: ");
  Serial.println(P_LORA_MISO);
  Serial.print("MOSI: ");
  Serial.println(P_LORA_MOSI);
  Serial.print("NSS:  ");
  Serial.println(P_LORA_NSS);
  Serial.print("RST:  ");
  Serial.println(P_LORA_RESET);
  Serial.print("DIO0: ");
  Serial.println(P_LORA_DIO_0);
  Serial.print("DIO1: ");
  Serial.println(P_LORA_DIO_1);

  fallback_clock.begin();
  rtc_clock.begin(Wire);

#if defined(P_LORA_SCLK)
  Serial.println("Starting custom SPI bus...");
  spi.begin(
    P_LORA_SCLK,
    P_LORA_MISO,
    P_LORA_MOSI,
    P_LORA_NSS
  );
  Serial.println("Custom SPI bus started");
  Serial.println("Initializing SX1276 at 100 kHz, SPI mode 0...");
  bool success = radio.std_init(&spi);
#else
  Serial.println("Initializing SX1276...");
  bool success = radio.std_init();
#endif

  Serial.print("RFM95W initialization: ");
  Serial.println(success ? "SUCCESS" : "FAILED");

  return success;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
