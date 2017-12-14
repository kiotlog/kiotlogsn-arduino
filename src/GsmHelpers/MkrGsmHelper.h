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

#ifndef MkrGsm_Helper_h
#define MkrGsm_Helper_h
#include <Arduino.h>

#include <MKRGSM.h>

#include "GsmHelper.h"

class MkrGsm : public GsmBase
{
    template<class GsmType> friend class KiotlogSN;

public:
    MkrGsm() = default;
    MkrGsm(GSM& gsmAccess, GPRS& gprsAccess, GSMUDP& client, const char* apn, const char* broker, const uint16_t port);
    ~MkrGsm() = default;

    void start() override;
    void lowpower() override;
    size_t getPacket(uint8_t *) override;
    void serialSend(uint8_t *, int) override;

    void reset()  override { };

protected:
    void wakeup() override { } ;
    void connect() override;

private:
    GSM& _mkr;
    GPRS& _gprs;
    GSMUDP& _client;
};

#endif