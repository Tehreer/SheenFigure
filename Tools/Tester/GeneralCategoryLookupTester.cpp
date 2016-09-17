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
#include <Source/SFGeneralCategoryLookup.h>
}

#include <Parser/UnicodeData.h>

#include "Utilities/Convert.h"
#include "Utilities/Unicode.h"

#include "GeneralCategoryLookupTester.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::Utilities;

static const string &DEFAULT_GENERAL_CATEGORY = "Cn";

GeneralCategoryLookupTester::GeneralCategoryLookupTester(const UnicodeData &unicodeData) :
    m_unicodeData(unicodeData)
{
}

void GeneralCategoryLookupTester::test()
{
    cout << "Running general category lookup tester." << endl;

    size_t failCounter = 0;
    string uniGeneralCategory;

    for (uint32_t codepoint = 0; codepoint < Unicode::MaxCodepoint; codepoint++) {
        SFGeneralCategory numGeneralCategory = SFGeneralCategoryDetermine(codepoint);
        const string &actual = Convert::toString(numGeneralCategory);

        m_unicodeData.getGeneralCategory(codepoint, uniGeneralCategory);
        const string &expected = (uniGeneralCategory.length() ? uniGeneralCategory : DEFAULT_GENERAL_CATEGORY);

        if (actual != expected) {
                cout << "Invalid general category found: " << endl
                     << "  Code Point: " << codepoint << endl
                     << "  Generated General Category: " << actual << endl
                     << "  Expected General Category: " << expected << endl;

            failCounter++;
        }
    }

    cout << failCounter << " error/s." << endl;
    cout << endl;
}
