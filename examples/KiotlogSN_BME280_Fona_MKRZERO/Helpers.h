#ifndef KLFONA_HELPERS_H
#define KLFONA_HELPERS_H

#include <ArduinoLowPower.h>

extern FonaGsm gsm;

void deepsleep(uint32_t interval, uint32_t t0) {
  uint32_t sleep_duration = interval * 1000UL;
  uint32_t elapsed = millis() - t0;
  sleep_duration -= elapsed;
  if (sleep_duration > 0)
    LowPower.sleep(sleep_duration);
}

void MQTTSN_serial_send(uint8_t* buffer, int len)
{
#if defined(DEBUG)
  SerialUSB.println("Sending " + String(len) + " bytes");
  for (int i = 0; i < len; i++){
    if (buffer[i] < 0x10) SerialUSB.print(0, HEX);
    SerialUSB.print(buffer[i], HEX);
  }
  SerialUSB.println();
#endif

  gsm.serialSend(buffer, len);
}

void MQTTSN_publish_handler(const msg_publish * msg) {}
void MQTTSN_gwinfo_handler(const msg_gwinfo * msg) {}

#endif
