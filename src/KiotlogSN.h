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

#ifndef KiotlogSN_h
#define KiotlogSN_h

#include <Arduino.h>
#include <mqttsn-messages.h>
#include <msgpck.h>
#include "Aead/Aead.h"

template <typename GsmType>
class KiotlogSN
{

public:
    KiotlogSN() = default;
    KiotlogSN(GsmType &gsm, const char* topic, const char* clientid, const uint32_t interval, const boolean preregistered = false);
    ~KiotlogSN() = default;

    void start(boolean start_gsm = true);
    void sendPayload(const uint8_t *, const size_t, const uint8_t *, const size_t);

    class Sn {
        template <typename G> friend class KiotlogSN;
    public:
        Sn(const char* topic, const char* clientid, const uint32_t interval, const boolean pre);
    protected:
        MQTTSN _client;
        const char* _id;
        const char* _topic;
        const uint32_t _interval;
        const boolean _pre;
        uint16_t _topic_id = 0;
        uint8_t _flags = FLAG_CLEAN;
    };

private:
    GsmType _gsm;
    Sn _sn;

    typedef enum {
        STARTING,
        CONNECTED,
        REGISTERED,
        PUBLISHED,
        DISCONNECTED
    } statuses;

    statuses _status = STARTING;

    const static size_t BUFFER_SERIAL_BUFFER_SIZE = 100;
    uint8_t _stream_buffer[BUFFER_SERIAL_BUFFER_SIZE];
    BufferStream _stream;

    void checkForData();
    boolean connect();
    uint16_t registerTopic();
    void publish(const uint8_t*, const size_t, const uint8_t*, const size_t);
    boolean disconnect();
    void lowpower();
};

#include <KiotlogSN.hpp>

#endif
