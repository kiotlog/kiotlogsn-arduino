#include <KiotlogSN.h>

#include <MKRGSM.h>
#include <Adafruit_SleepyDog.h> // You will also need Adafruit_ASFcore library
#include <Adafruit_BME280.h>

#include "Helpers.h"
#include "Battery.h"

GSM gsmAccess; // GSM gsmAccess(true);
GPRS gprsAccess;
GSMUDP client;

#define TIMEOUT 16 * 1000

//#define apn "hologram"
#define apn "tm"
#define broker "iottone.ostriot.io"
#define port 2883
#define interval 2 * 60

#define topic "/klsn/trmpln/mkr-mnx"
#define clientid "mkr-mnx"
const uint8_t key[] = {
  0xbd, 0xe8, 0x96, 0x7a, 0xf1, 0xc6, 0x3b, 0x38,
  0x21, 0x69, 0xec, 0x6c, 0x72, 0x64, 0xa0, 0x1e,
  0x75, 0xf3, 0x1f, 0xcc, 0x7b, 0x5c, 0xaa, 0xbe,
  0xa5, 0xe7, 0xfd, 0xbe, 0xd2, 0x58, 0x49, 0x31
};

typedef struct __attribute__ ((packed)) payload_struct {
  int16_t temperature;
  uint16_t pressure;
  uint16_t humidity;
  uint16_t battery;
} Payload;

Payload payload;

MkrGsm gsm(gsmAccess, gprsAccess, client, apn, broker, port);
Aead<sizeof(Payload)> aead(key);
KiotlogSN<MkrGsm> klsn(gsm, topic, clientid, interval);

Adafruit_BME280 bme;

void setup() {

  // Serial.begin(115200);
  // while (!Serial) {}

  Serial1.begin(115200);
  Serial1.println("Starting.");

  analogReadResolution(ADC_RESOLUTION);

  bme_init();
  aead.begin(A5);
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);
}

volatile int alarm_source = 0;
void alarmEvent0 () {
  alarm_source = 0;
}

void loop() {
  uint32_t t0 = millis();

  int timeout = Watchdog.enable(TIMEOUT);
  Serial1.println("Will reset in " + String(timeout) + "ms.");

  Serial1.println("Starting Cellular and Connecting.");
  klsn.start();
  Watchdog.reset();

  Serial1.println("Reading Sensors and Preparing Data.");
  preparePayload();
  Watchdog.reset();

  Serial1.println("Authenticating and Encrypting.");
  aead.authEncrypt((uint8_t *)&payload);
  Watchdog.reset();

  Serial1.println("Sending Message.");
  klsn.sendPayload(
    (uint8_t *)&aead.data, aead.data_len,
    (uint8_t *)&aead.nonce, aead.nonce_len);
  Watchdog.reset();

  Serial1.println("Updating AEAD status.");
  aead.updateStatus();
  Watchdog.disable();

  Serial1.println("Going to Sleep.");
  deepsleep(interval, t0);
}

inline void preparePayload()
{
  bme.takeForcedMeasurement();
  payload.temperature = bme.readTemperature() * 100;
  payload.pressure = bme.readPressure() / 10;
  payload.humidity = bme.readHumidity() * 100;
  payload.battery = readBattery() * 1000;
}

void bme_init() {
  bool status;

  status = bme.begin();
  if (!status) {
    Serial1.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1, // temperature
                  Adafruit_BME280::SAMPLING_X1, // pressure
                  Adafruit_BME280::SAMPLING_X1, // humidity
                  Adafruit_BME280::FILTER_OFF   );
}
