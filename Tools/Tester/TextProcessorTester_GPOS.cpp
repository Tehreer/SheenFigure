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
#include <SheenFigure/Source/SFAlbum.h>
#include <SheenFigure/Source/SFAssert.h>
}

#include "OpenType/Common.h"
#include "OpenType/GPOS.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

void TextProcessorTester::testSingleAdjustment()
{
    Glyph glyphs[] = { 1 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    /* Test with format 1. */
    {
        ValueRecord record;
        record.xPlacement = -100;
        record.yPlacement = -100;

        SinglePosSubtable subtable;
        subtable.posFormat = 1;
        subtable.coverage = &coverage;
        subtable.valueFormat = ValueFormat::XPlacement | ValueFormat::YPlacement;
        subtable.format1.value = &record;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1 };
        processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

        /* Test the output offsets. */
        const SFPoint *actual = SFAlbumGetGlyphOffsetsPtr(&album);
        const SFPoint expected[] = { { -100, -100, } };

        SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expected) / sizeof(SFPoint)));
        SFAssert(memcmp(actual, expected, sizeof(expected) / sizeof(SFPoint)) == 0);
    }

    /* Test with format 2. */
    {
        ValueRecord record;
        record.xPlacement = -100;
        record.yPlacement = -100;

        SinglePosSubtable subtable;
        subtable.posFormat = 2;
        subtable.coverage = &coverage;
        subtable.valueFormat = ValueFormat::XPlacement | ValueFormat::YPlacement;
        subtable.format2.valueCount = 1;
        subtable.format2.value = &record;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1 };
        processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

        /* Test the output glyphs. */
        const SFPoint *actual = SFAlbumGetGlyphOffsetsPtr(&album);
        const SFPoint expected[] = { { -100, -100, } };

        SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expected) / sizeof(SFPoint)));
        SFAssert(memcmp(actual, expected, sizeof(expected) / sizeof(SFPoint)) == 0);
    }
}
