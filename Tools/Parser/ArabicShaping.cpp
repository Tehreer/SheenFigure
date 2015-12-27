/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "ArabicShaping.h"

using namespace std;
using namespace SheenFigure::Parser;

static const string FILE_ARABIC_SHAPING = "ArabicShaping.txt";

static inline string getField(const string &data, size_t offset) {
    size_t length = data.find_first_of(';');
    return data.substr(offset, length);
}

ArabicShaping::ArabicShaping(const string &directory) :
    m_data(""),
    m_offsets(0x200000),
    m_firstCodePoint(0),
    m_lastCodePoint(0)
{
    ifstream stream(directory + "/" + FILE_ARABIC_SHAPING, ios::binary);
    char buffer[4096];

    while (stream.read(buffer, sizeof(buffer))) {
        m_data.append(buffer, sizeof(buffer));
    }
    m_data.append(buffer, (size_t)stream.gcount());

    istringstream datastream(m_data, ios::binary);
    string field;
    char ch;

    while (!datastream.eof()) {
        datastream >> ch;

        if (ch != '#') {
            datastream.seekg(-1, ios_base::cur);

            uint32_t codepoint;
            datastream >> hex >> setw(6) >> codepoint;
            datastream.ignore(128, ';');

            FieldOffset &offset = m_offsets[codepoint];
            offset.characterName = (size_t)datastream.tellg() + 1;
            datastream.ignore(128, ';');

            offset.joiningType = (size_t)datastream.tellg() + 1;
            datastream.ignore(128, ';');

            offset.joiningGroup = (size_t)datastream.tellg() + 1;

            if (!m_firstCodePoint) {
                m_firstCodePoint = codepoint;
            }

            if (codepoint > m_lastCodePoint) {
                m_lastCodePoint = codepoint;
            }
        }
        
        datastream.ignore(1024, '\n');
    }
}

uint32_t ArabicShaping::firstCodePoint() const {
    return m_firstCodePoint;
}

uint32_t ArabicShaping::lastCodePoint() const {
    return m_lastCodePoint;
}

std::uint32_t ArabicShaping::codePointForCharacterName(const string &characterName) const {
    auto begin = m_offsets.begin();
    auto end = m_offsets.end();

    for (auto iter = begin; iter != end; iter++) {
        const FieldOffset &offset = *iter;

        if (m_data.compare(offset.characterName, characterName.length(), characterName) == 0
            && m_data[characterName.length()] == ';') {
            return (uint32_t)(distance(begin, iter) + m_firstCodePoint);
        }
    }

    return 0;
}

std::string ArabicShaping::characterNameForCodePoint(uint32_t codepoint) const {
    if (codepoint >= m_firstCodePoint && codepoint <= m_lastCodePoint) {
        const FieldOffset &offset = m_offsets.at(codepoint);
        if (offset.characterName) {
            return getField(m_data, offset.characterName);
        }
    }
    
    return "";
}

char ArabicShaping::joiningTypeForCodePoint(uint32_t codepoint) const {
    if (codepoint >= m_firstCodePoint && codepoint <= m_lastCodePoint) {
        const FieldOffset &offset = m_offsets.at(codepoint);
        if (offset.joiningType) {
            return m_data[offset.joiningType];
        }
    }

    return '\0';
}

std::string ArabicShaping::joiningGroupForCodePoint(uint32_t codepoint) const {
    if (codepoint >= m_firstCodePoint && codepoint <= m_lastCodePoint) {
        const FieldOffset &offset = m_offsets.at(codepoint);
        if (offset.joiningGroup) {
            return getField(m_data, offset.joiningGroup);
        }
    }

    return "";
}
