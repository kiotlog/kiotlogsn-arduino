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

#ifndef Fona_Helper_h
#define Fona_Helper_h
#include <Arduino.h>

#include <Adafruit_FONA.h>

#include "GsmHelper.h"

struct FonaPinout {
        FonaPinout(uint8_t tx, uint8_t rx, uint8_t key, uint8_t rst, uint8_t dtr):
            tx(tx), rx(rx), key(key), rst(rst), dtr(dtr) {};

        uint8_t tx;
        uint8_t rx;
        uint8_t key;
        uint8_t rst;
        uint8_t dtr;
};


class GsmFona : public GsmBase
{
    template<class GsmType> friend class KiotlogSN;

public:
    GsmFona() = default;
    GsmFona(const gsm_module_t model, Adafruit_FONA * fona, Stream &fonaSS, const FonaPinout& pinout, const char* apn, const char* broker, const uint16_t port);
    ~GsmFona() = default;

    void start() override;
    void reset() override;
    void lowpower() override;
    void exitDataMode() override;
    void enterDataMode() override;

protected:
    void wakeup() override;
    void transparent(const int registered_status = 5) override;
    void connect() override;

private:
    FonaPinout _pins;
    Adafruit_FONA * _module;
    Stream * _serial;
};
#endif