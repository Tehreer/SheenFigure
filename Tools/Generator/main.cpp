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

#include <cstdio>
#include <iostream>
#include <string>

#include <Parser/ArabicShaping.h>
#include <Parser/UnicodeData.h>

#include "JoiningTypeLookupGenerator.h"
#include "GeneralCategoryLookupGenerator.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Generator;

int main(int argc, const char * argv[])
{
    ArabicShaping arabicShaping("/Users/mta/Desktop");
    UnicodeData unicodeData("/Users/mta/Desktop");

    JoiningTypeLookupGenerator joiningTypeLookup(arabicShaping);
    joiningTypeLookup.displayGaps();
    joiningTypeLookup.setAllowedGap(90);
    joiningTypeLookup.generateFile("/Users/mta/Desktop");

    GeneralCategoryLookupGenerator generator(unicodeData);
    generator.setMainSegmentSize(256);
    generator.setBranchSegmentSize(32);
    generator.generateFile("/Users/mta/Desktop");

    for (int i = arabicShaping.firstCodePoint(); i < arabicShaping.lastCodePoint(); i++) {
        char joiningType = arabicShaping.joiningTypeForCodePoint(i);
        if (joiningType == '\0') {
            joiningType = 'X';
        }
        printf("%c ", joiningType);
    }

    getchar();

    return 0;
}
