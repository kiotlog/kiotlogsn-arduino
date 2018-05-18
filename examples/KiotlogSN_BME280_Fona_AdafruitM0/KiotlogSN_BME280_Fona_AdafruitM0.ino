#include <Adafruit_FONA.h>
#include <Adafruit_SleepyDog.h>

#include "Helpers.h"
#include <KiotlogSN.h>
#include <KlsnFonaHelper.h>

#include <Adafruit_BME280.h>
Adafruit_BME280 bme;

#define LED_BUILTIN 13

// Change with your APN
#define apn "tm"
#define broker "iottone.ostriot.io"
#define port 2883
#define interval 15 * 60

// Change with your device and client id
#define topic "/klsn/my-app/my-dev-id"
#define clientid "my-cli-id"

// Insert a valid key
const uint8_t key[] = { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
    0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x0a };

// key, rst, dtr, ps
FonaPinout pins(6, 5, 0, 9);

Adafruit_FONA fona(pins.rst);

#define fonaSS Serial1
HardwareSerial* fonaSerial = &fonaSS;

typedef struct __attribute__((packed)) payload_struct {
    int16_t temperature;
    uint16_t pressure;
    uint16_t humidity;
    uint16_t battery;
} Payload;

Payload payload;

// Choose among:
// - FONA_800
// - FONA_80x
FonaGsm gsm(FONA_800, &fona, fonaSS, pins, apn, broker, port);

Aead<sizeof(Payload)> aead(key);
KiotlogSN<FonaGsm> klsn(gsm, topic, clientid, interval);

void setup()
{

    //  while (!Serial) {}
    Serial.begin(115200);
    Serial.println("Starting");

    bme_init();

    // Initialize encryption stuff with noise from analog pin.
    aead.begin(A5);

    // Power cycle Fona
    gsm.powerdown();
    delay(1000);
    gsm.powerup();
    delay(1000);

    // Initialize Fona
    fonaSerial->begin(9600);
    fona.begin(*fonaSerial);

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(50);
    }
}

void loop()
{
    for (int i = 0; i < 10; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(25);
        digitalWrite(LED_BUILTIN, LOW);
        delay(25);
    }
    uint32_t t0 = millis();

    // Will reset after 16 seconds if something hangs.
    Watchdog.enable(16000);

    // Read data in Payload
    preparePayload();
    Watchdog.reset();

    // Power up Fona
    gsm.wakeup();
    Watchdog.reset();

    // Create transparent-mode UDP connection
    gsm.transparent();
    Watchdog.reset();

    // Connect to MQTT-SN broker
    gsm.connect();
    Watchdog.reset();

    // Initialize KiotlogSN
    // Set to true if you want to let KiotlogSN initialize the GSM/GPRS connection.
    klsn.start(false);
    Watchdog.reset();

    // Encrypt and Tag data
    aead.authEncrypt((uint8_t*)&payload);
    Watchdog.reset();

    // Do MQTT-SN publish cycle
    klsn.sendPayload((uint8_t*)&aead.data, aead.data_len, (uint8_t*)&aead.nonce, aead.nonce_len);
    Watchdog.reset();

    // Update encryption status
    aead.updateStatus();

    Watchdog.disable();

    deepsleep(interval, t0);
}

void preparePayload()
{
    uint16_t vbat = 0;

    // In order to get battery status from Fona module
    // you need to exit from transparent mode.
    gsm.exitDataMode();
    fona.getBattVoltage(&vbat);
    gsm.enterDataMode();

    payload.battery = vbat;

    bme.takeForcedMeasurement();
    payload.temperature = bme.readTemperature() * 100;
    payload.pressure = bme.readPressure() / 10;
    payload.humidity = bme.readHumidity() * 100;
}

void bme_init()
{
    bool status;

    status = bme.begin();
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1)
            ;
    }

    bme.setSampling(Adafruit_BME280::MODE_FORCED,
        Adafruit_BME280::SAMPLING_X1, // temperature
        Adafruit_BME280::SAMPLING_X1, // pressure
        Adafruit_BME280::SAMPLING_X1, // humidity
        Adafruit_BME280::FILTER_OFF);
}
