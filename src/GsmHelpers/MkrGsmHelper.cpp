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

#include "MkrGsmHelper.h"

 MkrGsm::MkrGsm(GSM& gsmAccess, GPRS& gprsAccess, GSMUDP * client, const char* apn, const char* broker, const uint16_t port) :
    GsmBase(apn, broker, port), _mkr(gsmAccess), _gprs(gprsAccess), _client(client) { }

void MkrGsm::start()
{
    connect();
}

void MkrGsm::connect() {
    _mkr.begin();
    _gprs.attachGPRS(_apn, "", "");
    _client->begin(_port);
}

void MkrGsm::lowpower() {
    _client->stop();
    _gprs.detachGPRS();
    _mkr.shutdown();
}

size_t MkrGsm::getPacket(uint8_t * buffer)
{
    size_t bytes, cnt = 0;
    // _client->beginPacket(_broker, _port);
    if (_client->parsePacket())
        while (_client->available())
            buffer[cnt++] = _client->read();
    // _client->endPacket();
    return cnt;
}