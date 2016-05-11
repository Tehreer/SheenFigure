/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdlib>
#include "DataTypes.h"
#include "Writer.h"

using namespace SheenFigure::Tester::OpenType;

Writer::Writer()
{
    m_data = (UInt8 *)malloc(128);
    m_capacity = 128;
    m_size = 0;
    m_size = 0;
}

Writer::~Writer()
{
    free((void *)m_data);
}

void Writer::increaseSize(int size)
{
    m_size += size;

    if (m_capacity < m_size) {
        m_capacity *= 2;
        if (m_capacity < m_size) {
            m_capacity = m_size * 2;
        }

        m_data = (UInt8 *)realloc(m_data, (size_t)m_capacity);
    }
}

void Writer::enter()
{
    m_enteries.push(m_size);
}

void Writer::exit()
{
    m_enteries.pop();
}

int Writer::reserveOffset()
{
    int offset = m_size;
    write((UInt16)0);

    return offset;
}

int Writer::reserveLong()
{
    int reference = m_size;
    write((UInt32)0);

    return reference;
}

void Writer::writeTable(Table *table, int reference, bool isLong)
{
    Offset value = 0;

    if (table) {
        if (reference > -1) {
            value = (Offset)(m_size - m_enteries.top());
        }

        table->write(*this);
    }

    if (reference > -1) {
        if (isLong) {
            m_data[reference + 0] = (value >> 24) & 0xFF;
            m_data[reference + 1] = (value >> 16) & 0xFF;
            m_data[reference + 2] = (value >>  8) & 0xFF;
            m_data[reference + 3] = (value >>  0) & 0xFF;
        } else {
            m_data[reference + 0] = (value >> 8) & 0xFF;
            m_data[reference + 1] = (value >> 0) & 0xFF;
        }
    }
}

void Writer::write(UInt8 value)
{
    increaseSize(1);
    m_data[m_size - 1] = value;
}

void Writer::write(UInt16 value)
{
    increaseSize(2);
    m_data[m_size - 2] = (value >> 8) & 0xFF;
    m_data[m_size - 1] = (value >> 0) & 0xFF;
}

void Writer::write(UInt32 value)
{
    increaseSize(4);
    m_data[m_size - 4] = (value >> 24) & 0xFF;
    m_data[m_size - 3] = (value >> 16) & 0xFF;
    m_data[m_size - 2] = (value >>  8) & 0xFF;
    m_data[m_size - 1] = (value >>  0) & 0xFF;
}

void Writer::write(Table *array, int count)
{
    for (int i = 0; i < count; i++) {
        writeTable(&array[i]);
    }
}

void Writer::write(UInt8 *array, int count)
{
    for (int i = 0; i < count; i++) {
        write(array[i]);
    }
}

void Writer::write(UInt16 *array, int count)
{
    for (int i = 0; i < count; i++) {
        write(array[i]);
    }
}

void Writer::write(UInt32 *array, int count)
{
    for (int i = 0; i < count; i++) {
        write(array[i]);
    }
}
