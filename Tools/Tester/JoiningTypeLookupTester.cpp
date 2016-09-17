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

#include <cstdint>
#include <iostream>

extern "C" {
#include <Source/SFAssert.h>
#include <Source/SFJoiningTypeLookup.h>
}

#include <Parser/ArabicShaping.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "JoiningTypeLookupTester.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::Utilities;

JoiningTypeLookupTester::JoiningTypeLookupTester(const ArabicShaping &arabicShaping) :
    m_arabicShaping(arabicShaping)
{
}

void JoiningTypeLookupTester::test()
{
    cout << "Running joining type lookup tester." << endl;

    size_t failCounter = 0;

    for (uint32_t codepoint = 0; codepoint < Unicode::MaxCodepoint; codepoint++) {
        char actual = Convert::toChar(SFJoiningTypeDetermine(codepoint));
        char expected = m_arabicShaping.joiningTypeForCodePoint(codepoint);

        if (expected != actual) {
                cout << "Invalid joining type found: " << endl
                     << "  Code Point: " << codepoint << endl
                     << "  Generated Joining Type: " << actual << endl
                     << "  Expected Joining Type: " << expected << endl;

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
}
