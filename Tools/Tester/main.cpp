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

#include <Parser/ArabicShaping.h>
#include <Parser/UnicodeData.h>

#include "AlbumTester.h"
#include "FontTester.h"
#include "GeneralCategoryLookupTester.h"
#include "GlyphDiscovererTester.h"
#include "GlyphSubstituterTester.h"
#include "JoiningTypeLookupTester.h"
#include "ListTester.h"
#include "PatternTester.h"
#include "SchemeTester.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Tester;

int main(int argc, const char * argv[])
{
    const char *dir = "/path/to/unidata";

    ArabicShaping arabicShaping(dir);
    UnicodeData unicodeData(dir);

    AlbumTester albumTester;
    JoiningTypeLookupTester joiningTypeLookuptester(arabicShaping);
    GeneralCategoryLookupTester generalCategoryLookupTester(unicodeData);
    ListTester listTester;
    FontTester fontTester;
    PatternTester patternTester;
    SchemeTester schemeTester;
    GlyphDiscovererTester glyphDiscovererTester;
    GlyphSubstituterTester glyphSubstituterTester;

    albumTester.test();
    joiningTypeLookuptester.test();
    generalCategoryLookupTester.test();
    listTester.test();
    fontTester.test();
    patternTester.test();
    schemeTester.test();
    glyphDiscovererTester.test();
    glyphSubstituterTester.test();

    getchar();

    return 0;
}
