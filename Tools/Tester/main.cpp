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

#include <Parser/ArabicShaping.h>

#include "AlbumTester.h"
#include "FontTester.h"
#include "JoiningTypeLookupTester.h"
#include "ListTester.h"
#include "LocatorTester.h"
#include "PatternTester.h"
#include "SchemeTester.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Tester;

int main(int argc, const char * argv[])
{
    const char *dir = argv[1];

    ArabicShaping arabicShaping(dir);
    JoiningTypeLookupTester joiningTypeLookupTester(arabicShaping);
    ListTester listTester;
    AlbumTester albumTester;
    LocatorTester locatorTester;
    FontTester fontTester;
    PatternTester patternTester;
    SchemeTester schemeTester;
    TextProcessorTester textProcessorTester;

    albumTester.test();
    fontTester.test();
    joiningTypeLookupTester.test();
    listTester.test();
    locatorTester.test();
    patternTester.test();
    schemeTester.test();
    textProcessorTester.test();

    return 0;
}
