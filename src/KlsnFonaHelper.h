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

#include "KlsnGsmHelper.h"

typedef enum fona_modul_enum {
    FONA_800,
    FONA_80x,
    FONA_feather,
    FONA_debug
} fona_module_t;

struct FonaPinout {
        FonaPinout(uint8_t key, uint8_t rst, uint8_t dtr):
            key(key), rst(rst), dtr(dtr) {};
        uint8_t key;
        uint8_t rst;
        uint8_t dtr;
};

class FonaGsm : public GsmBase
{
    template<class GsmType> friend class KiotlogSN;

public:
    FonaGsm() = default;
    FonaGsm(const fona_module_t model, Adafruit_FONA * fona, Stream &fonaSS, const FonaPinout& pinout, const char* apn, const char* broker, const uint16_t port);
    ~FonaGsm() = default;

    void start() override;
    void reset() override;
    void lowpower() override;
    size_t getPacket(uint8_t * buffer) override;
    void serialSend(uint8_t *, int) override;
    void enterDataMode();
    void exitDataMode();

protected:
    void wakeup() override;
    void connect() override;
    void transparent(const int registered_status = 5);

private:
    FonaPinout _pins;
    Adafruit_FONA * _module;
    Stream * _serial;
    fona_module_t _model;
};

#endif