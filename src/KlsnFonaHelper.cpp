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
#include <Adafruit_FONA.h>

#include "KlsnFonaHelper.h"

FonaGsm::FonaGsm(const fona_module_t model, Adafruit_FONA * fona, Stream &fonaSS, const FonaPinout &pinout, const char* apn, const char* broker, const uint16_t port) :
    GsmBase(apn, broker, port), _model(model), _pins(pinout), _module(fona), _serial(&fonaSS) { }

void FonaGsm::start()
{
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

void FonaGsm::lowpower()
{
    digitalWrite(_pins.key, LOW);
    delay(3000);
    digitalWrite(_pins.key, HIGH);
}

void FonaGsm::wakeup()
{
    int waittime;
    switch (_model)
    {
    case FONA_80x:
    case FONA_feather:
        waittime = 3000;
        break;
    case FONA_800:
        waittime = 5000;
        break;
    }
    while (!_module->sendCheckReply(F("AT"), F("AT")))
    {
        digitalWrite(_pins.key, LOW);
        delay(waittime);
        digitalWrite(_pins.key, HIGH);
        gsm_wait(1000);
    }

    _module->sendCheckReply(F("ATE0"), F("OK"));
    _module->sendCheckReply(F("AT+CVHU=0"), F("OK"));
}

void FonaGsm::transparent(const int registered_status)
{
    // Enable exiting Data Mode with DTR pin
    if (_model == FONA_feather) _module->sendCheckReply(F("AT&D1"), F("OK"), _timeout);
    // Activate Transparent Mode
    while (!_module->sendCheckReply(F("AT+CIPMODE=1"), F("OK")))
        gsm_wait(2000);

    // Wait for Network
    while (_module->getNetworkStatus() != registered_status)
        gsm_wait(2000);
    delay(7000);

    // Set APN
    char buf[50];
    char *bufp = &buf[0];

    bufp += sprintf(bufp, "AT+CSTT=\"");
    bufp += sprintf(bufp, _apn);
    bufp += sprintf(bufp, "\"");

    _module->sendCheckReply(buf, F("OK"), _timeout);

    // Activate GPRS
    while (!_module->sendCheckReply(F("AT+CGATT=1"), F("OK"), _timeout))
        gsm_wait(1000);

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
    char *csp = &cs[0];

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
    gsm_wait(1050);

    switch (_model)
    {
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
    gsm_wait(1050);
    _module->expectReply(F("OK"), _timeout);
}

void FonaGsm::enterDataMode()
{
    gsm_wait(2050);
    _module->sendCheckReply(F("ATO"), F("CONNECT"), _timeout);
    gsm_wait(2050);
}

size_t FonaGsm::getPacket(uint8_t * buffer)
{
    size_t cnt = 0;
    while (_serial->available())
        buffer[cnt++] = _serial->read();

    return cnt;
}

void FonaGsm::serialSend(uint8_t * buffer, int len)
{
    _serial->write(buffer, len);
    _serial->flush();
    delay(500);
}