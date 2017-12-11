/*
 * Copyright (C) 2017 Giampaolo Mancini, Trampoline SRL.
 *
 * This file is part of KiotlogSN for Arduino.
 *
 * KiotlogSN for Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KiotlogSN for Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KiotlogSN for Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>

#include <SoftwareSerial.h>
#include <Adafruit_SleepyDog.h>
#include <Adafruit_FONA.h>

#include "FonaHelper.h"

#define KL_DEBUG 0

GsmFona::GsmFona(const gsm_module_t model, Adafruit_FONA * fona, SoftwareSerial * fonaSS, const FonaPinout &pinout, const char* apn, const char* broker, const uint16_t port) :
    _model(model), _apn(apn), _broker(broker), _port(port), _pins(pinout), _module(fona), _serial(fonaSS) { }

void GsmFona::start()
{
    // pinMode(_pins.tx, INPUT);
    // pinMode(_pins.rx, OUTPUT);
    pinMode(_pins.key, OUTPUT);
    pinMode(_pins.rst, OUTPUT);
    pinMode(_pins.dtr, OUTPUT);

    digitalWrite(_pins.dtr, HIGH);
    digitalWrite(_pins.key, HIGH);
    delay(10);
    digitalWrite(_pins.key, LOW);

    wakeup();
    transparent();
    connect();
}

void GsmFona::reset()
{
    pinMode(_pins.rst, OUTPUT);

    digitalWrite(_pins.rst, HIGH);

#if KL_DEBUG
    delay(10);
#else
    Watchdog.sleep(10);
#endif

    digitalWrite(_pins.rst, LOW);

#if KL_DEBUG
    delay(2000);
#else
    Watchdog.sleep(2000);
#endif

    digitalWrite(_pins.rst, HIGH);

#if KL_DEBUG
    delay(1000);
#else
    Watchdog.sleep(1000);
#endif
}

void GsmFona::sleep()
{
    digitalWrite(_pins.key, LOW);
    delay(3000);
    digitalWrite(_pins.key, HIGH);
}

void GsmFona::wakeup()
{
    int wait;
    switch (_model)
    {
    case FONA_80x:
    case FONA_feather:
    case FONA_debug:
        wait = 3000;
        break;
    case FONA_800:
        wait = 5000;
        break;
    }
    while (!_module->sendCheckReply(F("AT"), F("AT")))
    {
        digitalWrite(_pins.key, LOW);
        delay(wait);
        digitalWrite(_pins.key, HIGH);
#if KL_DEBUG
        delay(1000);
#else
        Watchdog.sleep(1000);
#endif
    }

    _module->sendCheckReply(F("ATE0"), F("OK"));
    _module->sendCheckReply(F("AT+CVHU=0"), F("OK"));
}

void GsmFona::transparent(const int registered_status)
{
    // Enable exiting Data Mode with DTR pin
    if (_model == FONA_feather) _module->sendCheckReply(F("AT&D1"), F("OK"), _timeout);
    // Activate Transparent Mode
    while (!_module->sendCheckReply(F("AT+CIPMODE=1"), F("OK")))
    {
#if KL_DEBUG
        delay(2000);
#else
        Watchdog.sleep(2000);
#endif
    }

    // Wait for Network
    while (_module->getNetworkStatus() != registered_status)
    {
#if KL_DEBUG
        delay(1000);
#else
        Watchdog.sleep(1000);
#endif
    }
#if KL_DEBUG
    delay(7000);
#else
    Watchdog.sleep(7000);
#endif
    // Set APN
    char buf[50];
    char *bufp = &buf[0];

    // memset(buf, 0, 50);

    bufp += sprintf(bufp, "AT+CSTT=\"");
    bufp += sprintf(bufp, _apn);
    bufp += sprintf(bufp, "\"");

    _module->sendCheckReply(buf, F("OK"), _timeout);

    // Activate GPRS
    while (!_module->sendCheckReply(F("AT+CGATT=1"), F("OK"), _timeout))
#if KL_DEBUG
        delay(1000);
#else
        Watchdog.sleep(1000);
#endif

    // Activate Wireless
    _module->sendCheckReply(F("AT+CIICR"), F("OK"), _timeout);

    // Get IP Address
    _module->sendCheckReply(F("AT+CIFSR"), F("OK"), _timeout);
}

void GsmFona::connect()
{
    boolean status;

    char cs[100];
    char *csp = &cs[0];

    csp += sprintf(csp, "AT+CIPSTART=\"UDP\",\"");
    csp += sprintf(csp, _broker);
    csp += sprintf(csp, "\",\"");
    csp += sprintf(csp, "%d", _port);
    csp += sprintf(csp, "\"");

    // Connect UDP
    _module->sendCheckReply(cs, F("OK"), _timeout);
    _module->expectReply(F("CONNECT"), _timeout);
    _serial->flush();

}

void GsmFona::exitDataMode()
{
#if KL_DEBUG
    delay(1050);
#else
    Watchdog.sleep(1050);
#endif

    switch (_model)
    {
    case FONA_feather:
    case FONA_debug:
        digitalWrite(_pins.dtr, LOW);
#if KL_DEBUG
        delay(1050);
#else
        Watchdog.sleep(1050);
#endif
        digitalWrite(_pins.dtr, HIGH);
        break;
    case FONA_80x:
    case FONA_800:
        _serial->print("+++");
        break;
    }

#if KL_DEBUG
    delay(1050);
#else
    Watchdog.sleep(1050);
#endif

    _module->expectReply(F("OK"), _timeout);
}

void GsmFona::enterDataMode()
{
#if KL_DEBUG
    delay(2050);
#else
    Watchdog.sleep(2050);
#endif
    _module->sendCheckReply(F("ATO"), F("CONNECT"), _timeout);
#if KL_DEBUG
    delay(2050);
#else
    Watchdog.sleep(2050);
#endif
}