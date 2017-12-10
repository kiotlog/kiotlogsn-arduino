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

#ifndef KiotlogAead_h
#define KiotlogAead_h

#include <Arduino.h>

#include "CryptoMini/ChaChaPoly.h"
#include "BufferStream/BufferStream.h"

#define KEY_SIZE 32
#define NONCE_SIZE 12
#define TAG_SIZE 16

template <unsigned int Len>
class Aead {
    
    template<class GsmType> friend class KiotlogSN;

public:
    Aead(const uint8_t * key);
    
    void authenc(const uint8_t * payload);
    void increment_iv();    
    
    uint8_t data[Len + TAG_SIZE];
    uint8_t nonce[NONCE_SIZE];
    uint8_t * key;

protected:
    ChaChaPoly _chachapoly;

    uint8_t _cipher[Len];
    uint8_t _tag[TAG_SIZE];
};

#include "Aead.hpp"

#endif