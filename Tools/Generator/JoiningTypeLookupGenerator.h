/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#ifndef _SHEEN_FIGURE__GENERATOR__JOINING_TYPE_LOOKUP_GENERATOR_H
#define _SHEEN_FIGURE__GENERATOR__JOINING_TYPE_LOOKUP_GENERATOR_H

#include <cstddef>
#include <string>

#include <Parser/ArabicShaping.h>

namespace SheenFigure {
namespace Generator {

class JoiningTypeLookupGenerator {
public:
    JoiningTypeLookupGenerator(const Parser::ArabicShaping &arabicShaping);

    void setAllowedGap(size_t);
    void displayGaps();

    void generateFile(const std::string &directory);

private:
    const Parser::ArabicShaping &m_arabicShaping;
    size_t m_allowedGap;
};

}
}

#endif
