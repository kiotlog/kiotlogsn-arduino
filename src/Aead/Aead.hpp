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

#include "Aead.h"

template <size_t Len>
Aead<Len>::Aead(const uint8_t *key) :
    _key(key)
{
    _chachapoly.setKey(_key, KEY_SIZE);
}

template <size_t Len>
void Aead<Len>::begin(const uint8_t noise_pin)
{
    pinMode(noise_pin, INPUT);
    randomSeed(analogRead(noise_pin));
    for (int i = 0; i < NONCE_SIZE; i++)
    {
        nonce[i] = (uint8_t)random(256);
    }
}

/*
   https://github.com/jedisct1/libsodium/blob/06a523423abeb3993c2ea6f149f10230cda58e8a/src/libsodium/sodium/utils.c#L237
*/
template <size_t Len>
void Aead<Len>::increment_iv()
{
    size_t i = 0U;
    uint_fast16_t c = 1U;

    for (; i < NONCE_SIZE; i++)
    {
        c += (uint_fast16_t)nonce[i];
        nonce[i] = (unsigned char)c;
        c >>= 8;
    }
}

template <size_t Len>
void Aead<Len>::updateStatus()
{
    increment_iv();
}

template <size_t Len>
void Aead<Len>::authEncrypt(const uint8_t *payload)
{
    _chachapoly.setIV(nonce, NONCE_SIZE);
    _chachapoly.encrypt(_cipher, payload, Len);
    _chachapoly.computeTag(_tag, TAG_SIZE);
    memcpy(data, _cipher, Len);
    memcpy(data + Len, _tag, TAG_SIZE);
}
