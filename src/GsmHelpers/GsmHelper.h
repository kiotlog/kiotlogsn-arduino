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

#ifndef GsmModule_Helper_h
#define GsmModule_Helper_h

#include <Arduino.h>

typedef enum gsm_modul_enum {
    FONA_800,
    FONA_80x,
    FONA_feather,
    FONA_debug
} gsm_module_t;

class GsmBase
{
  public:
    // GsmModule(const T& module);
    GsmBase() = default;
    virtual ~GsmBase() = default;

    GsmBase(const gsm_module_t model, const char * &apn, const char * &broker, const uint16_t port):
        _model(model), _apn(apn), _broker(broker), _port(port) { };

    virtual void start() = 0;
    virtual void reset() = 0;
    virtual void sleep() = 0;
    virtual void exitDataMode() = 0;
    virtual void enterDataMode() = 0;

protected:
    gsm_module_t _model;
    const char * _apn;
    const char * _broker;
    uint16_t _port;
    uint16_t _timeout = 10000;

    virtual void wakeup() = 0;
    virtual void transparent(const int) = 0;
    virtual void connect() = 0;
};

#endif