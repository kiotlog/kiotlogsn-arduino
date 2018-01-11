# KiotlogSN for Arduino

This library implements KiotlogSN LPWAN Protocol for Arduino based IoT devices.

KiotlogSN protocol uses 2G/3G networks to transimit secure, authenticated, bandwidth-restricted data to a [Kiotlog](https://github.com/kiotlog/kiotlog)-based IoT platform.

## Supported Hardware

1. Arduino MKRZERO + Adafruit Fona Mini Breakout
2. Adafruit M0 express + Adafruit Fona Mini Breakout
3. Sparkfun SAMD21G Mini Breakout + Adafruit Fona Mini Breakout
4. Arduino MKRGSM1400

Bundles 1 and 2 are best suited for low power deployments (<50 uA in deep sleep mode).

## Library Requirements

* [MQTT-SN for Arduino](https://github.com/kiotlog/mqttsn-arduino)
* [arduino_msgpack](https://github.com/HEADS-project/arduino_msgpack)

KiotlogSN for Arduino uses parts of Arduino Libs' [Crypto](http://rweather.github.io/arduinolibs/crypto.html) library.
