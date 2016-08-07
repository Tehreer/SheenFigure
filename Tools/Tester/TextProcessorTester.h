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

extern "C" {
#include <SheenFigure/Source/SFAlbum.h>
}

#include "OpenType/Common.h"

namespace SheenFigure {
namespace Tester {

class TextProcessorTester {
public:
    TextProcessorTester();

    void testSingleSubstitution();
    void testMultipleSubstitution();
    void testLigatureSubstitution();
    void testChainContextSubstitution();

    void testSingleAdjustment();
    void testPairPositioning();
    void testCursivePositioning();
    void testMarkToBasePositioning();
    void testMarkToLigaturePositioning();
    void testMarkToMarkPositioning();

    void test();

private:
    void processGSUB(SFAlbumRef album,
                     SFCodepoint *input, SFUInteger length, OpenType::LookupSubtable &subtable,
                     OpenType::LookupSubtable *referrals[] = NULL, SFUInteger count = 0);
    void processGPOS(SFAlbumRef album,
                     SFCodepoint *input, SFUInteger length, OpenType::LookupSubtable &subtable,
                     OpenType::LookupSubtable *referrals[] = NULL, SFUInteger count = 0);
};

}
}

#endif
