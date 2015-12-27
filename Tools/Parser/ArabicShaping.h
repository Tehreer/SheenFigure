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

#ifndef _SHEEN_FIGURE__PARSER__ARABIC_SHAPING_H
#define _SHEEN_FIGURE__PARSER__ARABIC_SHAPING_H

#include <cstdint>
#include <string>
#include <vector>

namespace SheenFigure {
namespace Parser {

class ArabicShaping {
public:
    ArabicShaping(const std::string &directory);
    ArabicShaping();

    uint32_t firstCodePoint() const;
    uint32_t lastCodePoint() const;

    std::uint32_t codePointForCharacterName(const std::string &) const;
    std::string characterNameForCodePoint(uint32_t) const;
    char joiningTypeForCodePoint(uint32_t) const;
    std::string joiningGroupForCodePoint(uint32_t) const;

private:
    typedef struct _FieldOffset {
        size_t characterName;
        size_t joiningType;
        size_t joiningGroup;
    } FieldOffset;

    uint32_t m_firstCodePoint;
    uint32_t m_lastCodePoint;

    std::string m_data;
    std::vector<FieldOffset> m_offsets;
};

}
}

#endif
