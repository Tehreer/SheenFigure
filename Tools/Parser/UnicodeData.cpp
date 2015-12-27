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

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "UnicodeData.h"

using namespace std;
using namespace SheenFigure::Parser;

static const string FILE_UNICODE_DATA = "UnicodeData.txt";

static inline void getField(const string &data, size_t offset, int field, string &result) {
    result.clear();

    if (offset != SIZE_MAX) {
        size_t start = offset;
        size_t end = offset;

        for (int i = 0; i < field; i++) {
            start = end + 1;
            end = data.find(';', start);
        }

        result.append(data.c_str() + start, end - start);
    }
}

UnicodeData::UnicodeData(const string &directory) :
    m_offsets(0x110000),
    m_firstCodepoint(0),
    m_lastCodepoint(0)
{
    ifstream fileStream(directory + "/" + FILE_UNICODE_DATA, ios::binary);
    char buffer[4096];

    while (fileStream.read(buffer, sizeof(buffer))) {
        m_data.append(buffer, sizeof(buffer));
    }
    m_data.append(buffer, (size_t)fileStream.gcount());

    istringstream dataStream(m_data, ios::binary);
    bool firstCodepointSet = false;

    while (!dataStream.eof()) {
        uint32_t codepoint;

        dataStream >> hex >> setw(6) >> codepoint;
        m_offsets[codepoint].value = (size_t)dataStream.tellg();

        if (!firstCodepointSet) {
            m_firstCodepoint = codepoint;
            firstCodepointSet = true;
        }
        if (codepoint > m_lastCodepoint) {
            m_lastCodepoint = codepoint;
        }

        dataStream.ignore(1024, '\n');
    }
}

uint32_t UnicodeData::firstCodepoint() const {
    return m_firstCodepoint;
}

uint32_t UnicodeData::lastCodepoint() const {
    return m_lastCodepoint;
}

size_t UnicodeData::offset(uint32_t codepoint) const {
    if (codepoint >= m_firstCodepoint && codepoint <= m_lastCodepoint) {
        return m_offsets[codepoint].value;
    }

    return SIZE_MAX;
}

void UnicodeData::getCharacterName(uint32_t codepoint, string &characterName) const {
    getField(m_data, offset(codepoint), 1, characterName);
}

void UnicodeData::getGeneralCategory(uint32_t codepoint, string &generalCategory) const {
    getField(m_data, offset(codepoint), 2, generalCategory);
}

void UnicodeData::getCombiningClass(uint32_t codepoint, string &combiningClass) const {
    getField(m_data, offset(codepoint), 3, combiningClass);
}

void UnicodeData::getBidirectionalCategory(uint32_t codepoint, string &bidirectionalCategory) const {
    getField(m_data, offset(codepoint), 4, bidirectionalCategory);
}

void UnicodeData::getDecompositionMapping(uint32_t codepoint, string &decompositionMapping) const {
    getField(m_data, offset(codepoint), 5, decompositionMapping);
}

void UnicodeData::getDecimalDigitValue(uint32_t codepoint, string &decimalDigitValue) const {
    getField(m_data, offset(codepoint), 6, decimalDigitValue);
}

void UnicodeData::getDigitValue(uint32_t codepoint, string &digitValue) const {
    getField(m_data, offset(codepoint), 7, digitValue);
}

void UnicodeData::getNumericValue(uint32_t codepoint, string &numericValue) const {
    getField(m_data, offset(codepoint), 8, numericValue);
}

void UnicodeData::getMirrored(uint32_t codepoint, string &mirrored) const {
    getField(m_data, offset(codepoint), 9, mirrored);
}

void UnicodeData::getOldName(uint32_t codepoint, string &oldName) const {
    getField(m_data, offset(codepoint), 10, oldName);
}

void UnicodeData::getCommentField(uint32_t codepoint, string &commentField) const {
    getField(m_data, offset(codepoint), 11, commentField);
}

void UnicodeData::getUppercaseMapping(uint32_t codepoint, string &uppercaseMapping) const {
    getField(m_data, offset(codepoint), 12, uppercaseMapping);
}

void UnicodeData::getLowercaseMapping(uint32_t codepoint, string &lowercaseMapping) const {
    getField(m_data, offset(codepoint), 13, lowercaseMapping);
}

void UnicodeData::getTitlecaseMapping(uint32_t codepoint, string &titlecaseMapping) const {
    getField(m_data, offset(codepoint), 14, titlecaseMapping);
}
