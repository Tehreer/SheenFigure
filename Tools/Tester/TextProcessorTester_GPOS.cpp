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

void TextProcessorTester::testPairPositioning()
{
    Glyph glyphs[] = { 1 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    /* Test with format 1. */
    {
        ValueRecord value2Record;
        value2Record.xPlacement = -100;
        value2Record.yPlacement = -100;

        PairValueRecord pairValueRecord;
        pairValueRecord.secondGlyph = 2;
        pairValueRecord.value1 = NULL;
        pairValueRecord.value2 = &value2Record;

        PairSetTable pairSet;
        pairSet.pairValueCount = 1;
        pairSet.pairValueRecord = &pairValueRecord;

        PairAdjustmentPosSubtable subtable;
        subtable.posFormat = 1;
        subtable.coverage = &coverage;
        subtable.valueFormat1 = ValueFormat::None;
        subtable.valueFormat2 = ValueFormat::XPlacement | ValueFormat::YPlacement;
        subtable.format1.pairSetCount = 1;
        subtable.format1.pairSetTable = &pairSet;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1, 2 };
        processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

        /* Test the output offsets. */
        const SFPoint *actual = SFAlbumGetGlyphOffsetsPtr(&album);
        const SFPoint expected[] = { {0, 0}, { -100, -100, } };

        SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expected) / sizeof(SFPoint)));
        SFAssert(memcmp(actual, expected, sizeof(expected) / sizeof(SFPoint)) == 0);
    }

    /* Test with format 2. */
    {
        UInt16 classValues1[] = { 0 };

        ClassDefTable classDef1;
        classDef1.classFormat = 1;
        classDef1.format1.startGlyph = 1;
        classDef1.format1.glyphCount = 1;
        classDef1.format1.classValueArray = classValues1;

        UInt16 classValues2[] = { 0 };

        ClassDefTable classDef2;
        classDef2.classFormat = 1;
        classDef2.format1.startGlyph = 2;
        classDef2.format1.glyphCount = 1;
        classDef2.format1.classValueArray = classValues2;

        ValueRecord value2Record;
        value2Record.xPlacement = -100;
        value2Record.yPlacement = -100;

        Class2Record class2Record;
        class2Record.value1 = NULL;
        class2Record.value2 = &value2Record;

        Class1Record class1Record;
        class1Record.class2Record = &class2Record;

        PairAdjustmentPosSubtable subtable;
        subtable.posFormat = 2;
        subtable.coverage = &coverage;
        subtable.valueFormat1 = ValueFormat::None;
        subtable.valueFormat2 = ValueFormat::XPlacement | ValueFormat::YPlacement;
        subtable.format2.classDef1 = &classDef1;
        subtable.format2.classDef2 = &classDef2;
        subtable.format2.class1Count = 1;
        subtable.format2.class2Count = 1;
        subtable.format2.class1Record = &class1Record;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1, 2 };
        processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

        /* Test the output glyphs. */
        const SFPoint *actual = SFAlbumGetGlyphOffsetsPtr(&album);
        const SFPoint expected[] = { {0, 0}, { -100, -100, } };

        SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expected) / sizeof(SFPoint)));
        SFAssert(memcmp(actual, expected, sizeof(expected) / sizeof(SFPoint)) == 0);
    }
}

void TextProcessorTester::testCursivePositioning()
{
    Glyph glyphs[] = { 1, 2, 3 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    AnchorTable firstExitAnchor;
    firstExitAnchor.anchorFormat = 1;
    firstExitAnchor.xCoordinate = 100;
    firstExitAnchor.yCoordinate = 0;

    EntryExitRecord firstRecord;
    firstRecord.entryAnchor = NULL;
    firstRecord.exitAnchor = &firstExitAnchor;

    AnchorTable secondEntryAnchor;
    secondEntryAnchor.anchorFormat = 1;
    secondEntryAnchor.xCoordinate = 10;
    secondEntryAnchor.yCoordinate = -10;

    AnchorTable secondExitAnchor;
    secondExitAnchor.anchorFormat = 1;
    secondExitAnchor.xCoordinate = 90;
    secondExitAnchor.yCoordinate = -10;

    EntryExitRecord secondRecord;
    secondRecord.entryAnchor = &secondEntryAnchor;
    secondRecord.exitAnchor = &secondExitAnchor;

    AnchorTable thirdEntryAnchor;
    thirdEntryAnchor.anchorFormat = 1;
    thirdEntryAnchor.xCoordinate = -10;
    thirdEntryAnchor.yCoordinate = 10;

    EntryExitRecord thirdRecord;
    thirdRecord.entryAnchor = &thirdEntryAnchor;
    thirdRecord.exitAnchor = NULL;

    EntryExitRecord allRecords[] = { firstRecord, secondRecord, thirdRecord };

    CursiveAttachmentPosSubtable subtable;
    subtable.posFormat = 1;
    subtable.coverage = &coverage;
    subtable.entryExitCount = sizeof(allRecords) / sizeof(EntryExitRecord);
    subtable.entryExitRecord = allRecords;

    SFAlbum album;
    SFAlbumInitialize(&album);

    SFCodepoint input[] = { 1, 2, 3 };
    processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

    /* Test the output. */
    const SFPoint *actualOffsets = SFAlbumGetGlyphOffsetsPtr(&album);
    const SFAdvance *actualAdvances = SFAlbumGetGlyphAdvancesPtr(&album);
    const SFPoint expectedOffsets[] = { {0, 0}, { -10, 10, }, { 10, -10 } };
    const SFAdvance expectedAdvances[] = { 100, 80, 113 };

    SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expectedOffsets) / sizeof(SFPoint)));
    SFAssert(memcmp(actualOffsets, expectedOffsets, sizeof(expectedOffsets) / sizeof(SFPoint)) == 0);
    SFAssert(memcmp(actualAdvances, expectedAdvances, sizeof(expectedAdvances) / sizeof(SFAdvance)) == 0);
}
