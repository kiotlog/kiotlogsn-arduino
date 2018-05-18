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

#include <Adafruit_FONA.h>
#include <Arduino.h>

#include "KlsnFonaHelper.h"

FonaGsm::FonaGsm(const fona_module_t model, Adafruit_FONA* fona, Stream& fonaSS, const FonaPinout& pinout,
    const char* apn, const char* broker, const uint16_t port)
    : GsmBase(apn, broker, port)
    , _model(model)
    , _pins(pinout)
    , _module(fona)
    , _serial(&fonaSS)
{
}

void FonaGsm::start()
{
    pinMode(_pins.rst, OUTPUT);
    pinMode(_pins.dtr, OUTPUT);

    digitalWrite(_pins.dtr, HIGH);

    wakeup();
    transparent();
    connect();
}

void FonaGsm::reset()
{
    pinMode(_pins.rst, OUTPUT);

    digitalWrite(_pins.rst, HIGH);
    gsm_wait(100);
    digitalWrite(_pins.rst, LOW);
    gsm_wait(2000);
    digitalWrite(_pins.rst, HIGH);
    gsm_wait(1000);
}

void FonaGsm::powerdown()
{
    pinMode(_pins.ps, INPUT);
    pinMode(_pins.key, OUTPUT);
    for (int i = 0; i < 2; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(35);
        digitalWrite(LED_BUILTIN, LOW);
        delay(25);
    }

    while (digitalRead(_pins.ps)) {
        digitalWrite(_pins.key, LOW);
        delay(3000);
        digitalWrite(_pins.key, HIGH);
        delay(1000);
    }
}

void FonaGsm::lowpower() { powerdown(); }

void FonaGsm::powerup()
{
    int waittime;
    pinMode(_pins.ps, INPUT);
    pinMode(_pins.key, OUTPUT);

    switch (_model) {
    case FONA_80x:
    case FONA_feather:
        waittime = 3000;
        break;
    case FONA_800:
        waittime = 5000;
        break;
    }

    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(35);
        digitalWrite(LED_BUILTIN, LOW);
        delay(25);
    }

    while (!digitalRead(_pins.ps)) {
        digitalWrite(_pins.key, LOW);
        delay(waittime);
        digitalWrite(_pins.key, HIGH);
        delay(1000);
    }
}
void FonaGsm::wakeup()
{
    powerup();
    _module->sendCheckReply(F("ATE0"), F("OK"));
    _module->sendCheckReply(F("AT+CVHU=0"), F("OK"));
}

void FonaGsm::transparent(const int registered_status)
{
    // Enable exiting Data Mode with DTR pin
    if (_model == FONA_feather)
        _module->sendCheckReply(F("AT&D1"), F("OK"), _timeout);

    // Activate Transparent Mode
    while (!_module->sendCheckReply(F("AT+CIPMODE=1"), F("OK")))
        delay(500);

    // Shutdown all sockets
    _module->sendCheckReply(F("AT+CIPSHUT"), F("SHUT OK"), 20000);

    // Wait for Network
    while (_module->getNetworkStatus() != registered_status)
        delay(500);

    // Set GPRS Context
    while (!_module->sendCheckReply(F("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""), F("OK"), _timeout))
        delay(500);

    // Set APN
    String buf = "AT+SAPBR=3,1,\"APN\",\"";
    buf += _apn;
    buf += "\"";
    while (!_module->sendCheckReply((char*)buf.c_str(), F("OK"), _timeout))
        delay(500);

    buf = "AT+CSTT=\"";
    buf += _apn;
    buf += "\"";
    while (!_module->sendCheckReply((char*)buf.c_str(), F("OK"), _timeout))
        delay(500);

    // Check for GPRS Context
    while (!_module->sendCheckReply(F("AT+SAPBR=1,1"), F("OK"), 30000))
        delay(500);

    // Activate GPRS
    while (!_module->sendCheckReply(F("AT+CGATT=1"), F("OK"), _timeout))
        gsm_wait(500);

    // Activate Wireless
    _module->sendCheckReply(F("AT+CIICR"), F("OK"), _timeout);

    // Get IP Address
    _module->sendCheckReply(F("AT+CIFSR"), F("OK"), _timeout);

    // DTR
    // _module->sendCheckReply(F("AT+IFC=2,2"), F("OK"));
}

void FonaGsm::connect()
{
    boolean status;

    char cs[100];
    char* csp = &cs[0];

    csp += sprintf(csp, "AT+CIPSTART=\"UDP\",\"");
    csp += sprintf(csp, _broker);
    csp += sprintf(csp, "\",\"");
    csp += sprintf(csp, "%d", _port);
    csp += sprintf(csp, "\"");

    // Connect UDP
    _module->sendCheckReply(cs, F("OK"), _timeout);
    _module->expectReply(F("CONNECT"), _timeout);
}

void FonaGsm::exitDataMode()
{
    delay(1050);

    switch (_model) {
    case FONA_feather:
        digitalWrite(_pins.dtr, LOW);
        gsm_wait(1050);
        digitalWrite(_pins.dtr, HIGH);
        break;
    case FONA_80x:
    case FONA_800:
        _serial->print("+++");
        break;
    }
    delay(1050);
    _module->expectReply(F("OK"), _timeout);
}

void FonaGsm::enterDataMode()
{
    delay(2050);
    _module->sendCheckReply(F("ATO"), F("CONNECT"), _timeout);
    delay(2050);
}

size_t FonaGsm::getPacket(uint8_t* buffer)
{
    size_t cnt = 0;
    while (_serial->available())
        buffer[cnt++] = _serial->read();

    return cnt;
}

void FonaGsm::serialSend(uint8_t* buffer, int len)
{
    _serial->write(buffer, len);
    _serial->flush();
    delay(500);
}
