#ifndef __MQTT_SN_HELPERS_H_
#define __MQTT_SN_HELPERS_H_

#include <Arduino.h>

extern SoftwareSerial fonaSS;
extern MQTTSN mqttsn;

void checkSerial()
{
  uint16_t cnt = 0;
  uint8_t buffer[512];
  uint8_t *buf = &buffer[0];

  while (fonaSS.available()) buffer[cnt++] = fonaSS.read();
  if (cnt > 0) mqttsn.parse_stream(buf, cnt);
}

void MQTTSN_serial_send(uint8_t* message_buffer, int length)
{
  fonaSS.write(message_buffer, length);
  fonaSS.flush();
}

void MQTTSN_publish_handler(const msg_publish * msg) {}
void MQTTSN_gwinfo_handler(const msg_gwinfo * msg) {}

#endif
