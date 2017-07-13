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

#ifndef __SHEENFIGURE_TESTER__TEXT_PROCESSOR_TESTER_H
#define __SHEENFIGURE_TESTER__TEXT_PROCESSOR_TESTER_H

#include <cstdint>
#include <utility>
#include <vector>

#include "OpenType/Base.h"
#include "OpenType/Common.h"

namespace SheenFigure {
namespace Tester {

class TextProcessorTester {
public:
    TextProcessorTester();

    void testSingleSubstitution();
    void testMultipleSubstitution();
    void testAlternateSubstitution();
    void testLigatureSubstitution();

    void testSinglePositioning();
    void testPairPositioning();
    void testCursivePositioning();
    void testMarkToBasePositioning();
    void testMarkToLigaturePositioning();
    void testMarkToMarkPositioning();

    void testContextSubtable();
    void testChainContextSubtable();
    void testExtensionSubtable();

    void test();

private:
    void testSubstitution(OpenType::LookupSubtable &subtable,
                          const std::vector<uint32_t> codepoints,
                          const std::vector<OpenType::Glyph> glyphs,
                          const std::vector<OpenType::LookupSubtable *> referrals = { });
    void testPositioning(OpenType::LookupSubtable &subtable,
                         const std::vector<uint32_t> codepoints,
                         const std::vector<std::pair<int32_t, int32_t>> offsets,
                         const std::vector<int32_t> advances,
                         const std::vector<OpenType::LookupSubtable *> referrals = { },
                         bool isRTL = false);
};

}
}

#endif
