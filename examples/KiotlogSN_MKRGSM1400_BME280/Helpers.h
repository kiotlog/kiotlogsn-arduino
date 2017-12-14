#ifndef KLFONA_HELPERS_H
#define KLFONA_HELPERS_H

#include <ArduinoLowPower.h>

extern MkrGsm gsm;

void KLSN_sleep(uint32_t sleep_duration) {

  LowPower.sleep(sleep_duration);
  // delay(sleep_duration);
}

void deepsleep(uint32_t interval, uint32_t t0) {
  uint32_t sleep_duration = interval * 1000UL;
  uint32_t elapsed = millis() - t0;
  sleep_duration -= elapsed;
  if (sleep_duration < 0) sleep_duration = 0;
  Serial1.println("Will sleep for: " + String(sleep_duration) + "ms.");
  LowPower.sleep(sleep_duration);
  Serial1.println("Waking up.");
}

void MQTTSN_serial_send(uint8_t * buffer, int len)
{
  gsm.serialSend(buffer, len);
}
void MQTTSN_publish_handler(const msg_publish * msg) {}
void MQTTSN_gwinfo_handler(const msg_gwinfo * msg) {}

#endif
