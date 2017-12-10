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

#include <KiotlogSN.h>

template <class T>
KiotlogSN<T>::KiotlogSN(T &gsm, MQTTSN * mqttsn, const char *&topic, const char *&clientid, const uint32_t interval, const boolean preregistered) :
    _gsm(gsm), _sn(mqttsn, topic, clientid, interval, preregistered), _stream(_stream_buffer, BUFFER_SERIAL_BUFFER_SIZE) {}

template <class T>
KiotlogSN<T>::Sn::Sn(MQTTSN * mqttsn, const char *&topic, const char *&clientid, const uint32_t interval, const boolean pre) : _client(mqttsn), _id(clientid), _topic(topic), _interval(interval), _pre(pre) {}

template <class T>
void KiotlogSN<T>::start()
{
    Serial.println("Beginning");
    _gsm.start();
}

template <class T>
void KiotlogSN<T>::sendPayload(const uint8_t * data, size_t data_len, const uint8_t * nonce, size_t nonce_len)
{
    boolean done = false;

    while (!done)
    {
        checkForData();

        switch (_status)
        {
        case STARTING:
            Serial.println("Starting: Connect");
            connect();
            _status = CONNECTING;
            break;
        case CONNECTING:
            Serial.println("Connecting: Register Topic");
            if (registerTopic() != 0xffff){
                _status = REGISTERING;
            }
            break;
        case REGISTERING:
            Serial.println("Registering Topic: Publish");
            publish(data, data_len, nonce, nonce_len);
            _status = PUBLISHING;
            break;
        case PUBLISHING:
            Serial.println("Publishing: Disconnect");
            disconnect();
            _status = DISCONNECTING;
            break;
        case DISCONNECTING:
            Serial.println("Disconnecting: Sleep");
            if (!_sn._client->connected()) {
                _status = SLEEPING;
            }
            break;
        case SLEEPING:
            Serial.println("Sleeping: Start");
            sleep();
            _status = STARTING;
            done = true;
            break;
        }
    }

    Serial.println("Done");
}

template <class T>
void KiotlogSN<T>::checkForData()
{
    uint16_t cnt = 0;
    uint8_t buffer[512];
    uint8_t *buf = &buffer[0];

    Serial.println("Checking for data");

    while (_gsm._serial->available()) buffer[cnt++] = _gsm._serial->read();
    if (cnt > 0) _sn._client->parse_stream(buf, cnt);
    delay(2000);

    while (_sn._client->wait_for_response()) {
        while (_gsm._serial->available()) buffer[cnt++] = _gsm._serial->read();
        if (cnt > 0) _sn._client->parse_stream(buf, cnt);
    }
}

template <class T>
void KiotlogSN<T>::connect()
{
    if (!_sn._client->connected()) _sn._client->connect(_sn._flags, 10, _sn._id);
}

template <class T>
uint16_t KiotlogSN<T>::registerTopic()
{
    uint8_t index;
    if (_sn._pre)
    {
        _sn._topic_id = (uint16_t)atoi(_sn._topic);
        _sn._flags |= FLAG_TOPIC_PREDEFINED_ID;
    }
    else if (strlen(_sn._topic) == 2)
    {
        _sn._topic_id = (_sn._topic[0] << 8) + _sn._topic[1];
        _sn._flags |= FLAG_TOPIC_SHORT_NAME;
    }
    else
    {
        _sn._topic_id = (uint16_t)_sn._client->find_topic_id(_sn._topic, &index);
        if (_sn._topic_id == 0xffff)
        {
            _sn._client->register_topic(_sn._topic);
        }
        else
        {
            _sn._flags |= FLAG_TOPIC_NAME;
        }
    }
    return _sn._topic_id;
}

template <class T>
void KiotlogSN<T>::publish(const uint8_t * data, size_t data_len, const uint8_t * nonce, size_t nonce_len)
{
    msgpck_write_map_header(&_stream, 2);
    msgpck_write_string(&_stream, "nonce");
    msgpck_write_bin(&_stream, nonce, nonce_len);
    msgpck_write_string(&_stream, "data");
    msgpck_write_bin(&_stream, data, data_len);

    size_t len = _stream.available();
    const uint8_t *s = _stream;
    
    _sn._client->publish(_sn._flags, _sn._topic_id, s, len);
}

template <class T>
void KiotlogSN<T>::disconnect()
{
    _sn._client->disconnect(_sn._interval * 2UL + 60);
}

template <class T>
void KiotlogSN<T>::sleep()
{
    _gsm.sleep();
}
