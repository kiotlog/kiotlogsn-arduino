#ifndef KLFONA_HELPERS_H
#define KLFONA_HELPERS_H

#include <Adafruit_SleepyDog.h>
#include <KlsnFonaHelper.h>

extern FonaGsm gsm;

void deepsleep(uint32_t interval, uint32_t t0) {
  uint32_t sleep_duration = interval * 1000UL;
  uint32_t elapsed = millis() - t0;
  sleep_duration -= elapsed;
  Serial.println(sleep_duration);
  if (sleep_duration > 0) {
    uint32_t sleepMS = 0;
    while ((sleepMS <= sleep_duration)) {
      sleepMS += Watchdog.sleep(sleep_duration);
    }
  }
}

void MQTTSN_serial_send(uint8_t* message_buffer, int length)
{
  gsm.serialSend(message_buffer, length);
}

void MQTTSN_publish_handler(const msg_publish * msg) {}
void MQTTSN_gwinfo_handler(const msg_gwinfo * msg) {}

#endif
