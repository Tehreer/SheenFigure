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
    const string in = "/path/to/input";
    const string out = "/path/to/output";

    ArabicShaping arabicShaping(in);
    UnicodeData unicodeData(in);

    JoiningTypeLookupGenerator joiningTypeLookup(arabicShaping);
    joiningTypeLookup.setAllowedGap(75);
    joiningTypeLookup.generateFile(out);

    GeneralCategoryLookupGenerator generator(unicodeData);
    generator.setMainSegmentSize(16);
    generator.setBranchSegmentSize(64);
    generator.generateFile(out);

    return 0;
}
