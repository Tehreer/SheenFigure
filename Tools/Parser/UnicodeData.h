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

#ifndef _SHEEN_FIGURE__PARSER__UNICODE_DATA_H
#define _SHEEN_FIGURE__PARSER__UNICODE_DATA_H

#include <cstdint>
#include <string>
#include <vector>

namespace SheenFigure {
namespace Parser {

class UnicodeData {
public:
    UnicodeData(const std::string &directory);

    uint32_t firstCodepoint() const;
    uint32_t lastCodepoint() const;

    void getCharacterName(uint32_t codepoint, std::string &characterName) const;
    void getGeneralCategory(uint32_t codepoint, std::string &generalCategory) const;
    void getCombiningClass(uint32_t codepoint, std::string &combiningClass) const;
    void getBidirectionalCategory(uint32_t codepoint, std::string &bidirectionalCategory) const;
    void getDecompositionMapping(uint32_t codepoint, std::string &decompositionMapping) const;
    void getDecimalDigitValue(uint32_t codepoint, std::string &decimalDigitValue) const;
    void getDigitValue(uint32_t codepoint, std::string &digitValue) const;
    void getNumericValue(uint32_t codepoint, std::string &numericValue) const;
    void getMirrored(uint32_t codepoint, std::string &mirrored) const;
    void getOldName(uint32_t codepoint, std::string &oldName) const;
    void getCommentField(uint32_t codepoint, std::string &commentField) const;
    void getUppercaseMapping(uint32_t codepoint, std::string &uppercaseMapping) const;
    void getLowercaseMapping(uint32_t codepoint, std::string &lowercaseMapping) const;
    void getTitlecaseMapping(uint32_t codepoint, std::string &titlecaseMapping) const;

private:
    struct _Offset {
        size_t value;

        _Offset() {
            value = SIZE_MAX;
        }
    };

    uint32_t m_firstCodepoint;
    uint32_t m_lastCodepoint;

    std::string m_data;
    std::vector<_Offset> m_offsets;

    size_t offset(uint32_t codepoint) const;
};

}
}

#endif
