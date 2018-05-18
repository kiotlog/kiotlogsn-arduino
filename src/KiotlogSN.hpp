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

// #define KL_DEBUG
// #define Serial SerialUSB

template <class T>
KiotlogSN<T>::KiotlogSN(
    T& gsm, const char* topic, const char* clientid, const uint32_t interval, const boolean preregistered)
    : _gsm(gsm)
    , _sn(topic, clientid, interval, preregistered)
    , _stream(_stream_buffer, BUFFER_SERIAL_BUFFER_SIZE)
{
}

template <class T>
KiotlogSN<T>::Sn::Sn(const char* topic, const char* clientid, const uint32_t interval, const boolean pre)
    : _id(clientid)
    , _topic(topic)
    , _interval(interval)
    , _pre(pre)
{
}

template <class T> void KiotlogSN<T>::start(boolean start_gsm)
{
    if (start_gsm)
        _gsm.start();
}

template <class T>
void KiotlogSN<T>::sendPayload(const uint8_t* data, size_t data_len, const uint8_t* nonce, size_t nonce_len)
{
    boolean done = false;

#if defined(KL_DEBUG)
    Serial.println("Sending Loop");
#endif

    while (!done) {
#if defined(KL_DEBUG)
        Serial.println("Checking for data");
#endif
        checkForData();
        switch (_status) {
        case STARTING:
#if defined(KL_DEBUG)
            Serial.println("CONNECTING");
#endif
            if (connect())
                _status = CONNECTED;
            break;
        case CONNECTED:
#if defined(KL_DEBUG)
            Serial.println("REGISTERING");
#endif
            if (registerTopic() != 0xffff)
                _status = REGISTERED;
            break;
        case REGISTERED:
#if defined(KL_DEBUG)
            Serial.println("PUBLISHING");
#endif
            publish(data, data_len, nonce, nonce_len);
            _status = PUBLISHED;
            break;
        case PUBLISHED:
#if defined(KL_DEBUG)
            Serial.println("DISCONNECTING");
#endif
            if (disconnect())
                _status = DISCONNECTED;
            break;
        case DISCONNECTED:
#if defined(KL_DEBUG)
            Serial.println("SLEEPING");
#endif
            lowpower();
            _status = STARTING;
            done = true;
            break;
        }
    }
}

template <class T> void KiotlogSN<T>::checkForData()
{
    uint16_t cnt = 0;
    uint8_t buffer[512];
    uint8_t* buf = &buffer[0];

    while (_sn._client.wait_for_response()) {
        delay(500);
        cnt = _gsm.getPacket(buf);
        if (cnt > 0)
            _sn._client.parse_stream(buf, cnt);
    }
}

template <class T> boolean KiotlogSN<T>::connect()
{
    boolean connected;

    connected = _sn._client.connected();
    if (!connected)
        _sn._client.connect(_sn._flags, (uint16_t)10, _sn._id);
    return connected;
}

template <class T> uint16_t KiotlogSN<T>::registerTopic()
{
    uint8_t index;
    if (_sn._pre) {
        _sn._topic_id = (uint16_t)atoi(_sn._topic);
        _sn._flags |= FLAG_TOPIC_PREDEFINED_ID;
    } else if (strlen(_sn._topic) == 2) {
        _sn._topic_id = (_sn._topic[0] << 8) + _sn._topic[1];
        _sn._flags |= FLAG_TOPIC_SHORT_NAME;
    } else {
        _sn._topic_id = (uint16_t)_sn._client.find_topic_id(_sn._topic, &index);
        if (_sn._topic_id == 0xffff) {
            _sn._client.register_topic(_sn._topic);
        } else {
            _sn._flags |= FLAG_TOPIC_NAME;
        }
    }
    return _sn._topic_id;
}

template <class T>
void KiotlogSN<T>::publish(const uint8_t* data, size_t data_len, const uint8_t* nonce, size_t nonce_len)
{
    msgpck_write_map_header(&_stream, 2);
    msgpck_write_string(&_stream, "nonce");
    msgpck_write_bin(&_stream, (byte*)nonce, nonce_len);
    msgpck_write_string(&_stream, "data");
    msgpck_write_bin(&_stream, (byte*)data, data_len);

    size_t len = _stream.available();
    const uint8_t* s = _stream;

    _sn._client.publish(_sn._flags, _sn._topic_id, s, len);
    _stream.flush();
}

template <class T> boolean KiotlogSN<T>::disconnect()
{
    boolean disconnected;
    disconnected = !_sn._client.connected();

    if (!disconnected)
        _sn._client.disconnect(_sn._interval * 2UL + 60);
    return disconnected;
}

template <class T> void KiotlogSN<T>::lowpower() { _gsm.lowpower(); }
