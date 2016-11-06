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
#include <cstring>
#include <iostream>

extern "C" {
#include <Source/SFAlbum.h>
#include <Source/SFAssert.h>
}

#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "OpenType/GPOS.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

void TextProcessorTester::testSinglePositioning()
{
    Builder builder;

    /* Test the first format. */
    {
        /* Test with unmatching glyph. */
        testPositioning(builder.createSinglePos({ 0 }, builder.createValueRecord({ -100, -200, -300, 0 })),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with empty value record. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ 0, 0, 0, 0 })),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with negative values. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ -100, -200, -300, 0 })),
                        { 1 }, { {-100, -200} }, { -300 });
        /* Test with positive values. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ 100, 200, 300, 0 })),
                        { 1 }, { {100, 200} }, { 300 });
    }

    /* Test the second format. */
    {
        /* Test with unmatching glyph. */
        testPositioning(builder.createSinglePos({ 0 },
                        { builder.createValueRecord({ -100, -200, -300, 0 }) }),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with empty value record. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ 0, 0, 0, 0 }) }),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with negative values. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ -100, -200, -300, 0 }) }),
                        { 1 }, { {-100, -200} }, { -300 });
        /* Test with positive values. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ 100, 200, 300, 0 }) }),
                        { 1 }, { {100, 200} }, { 300 });
    }
}

void TextProcessorTester::testPairPositioning()
{
    Builder builder;

    ValueRecord &empty = builder.createValueRecord({ 0, 0, 0, 0 });
    ValueRecord &positive1 = builder.createValueRecord({ 100, 200, 300, 400 });
    ValueRecord &positive2 = builder.createValueRecord({ 600, 700, 800, 900 });
    ValueRecord &negative1 = builder.createValueRecord({ -900, -800, -700, -600 });
    ValueRecord &negative2 = builder.createValueRecord({ -400, -300, -200, -100 });

    /* Test the first format. */
    {
        /* Test with first unmatching glyph. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with second unmatching glyph. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with empty values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, empty, empty }
                        }),
                        { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with positive values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 1, 2 }, { {100, 200}, {600, 700} }, { 300, 800 });
        /* Test with negative values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, negative1, negative2 }
                        }),
                        { 1, 2 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in a single pair set. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 1, 3, positive2, negative1 },
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 1, 3 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in a single pair set. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 1, 3, positive2, negative1 },
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 1, 4 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 3, 4, positive2, negative1 },
                            pair_rule { 5, 6, negative1, negative2 }
                        }),
                        { 3, 4 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 3, 4, positive2, negative1 },
                            pair_rule { 5, 6, negative1, negative2 }
                        }),
                        { 5, 6 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
    }

    /* Test the second format. */
    {
        reference_wrapper<ClassDefTable> classDefs[] = {
            builder.createClassDef(11, 3, { 1, 2, 3 }),
            builder.createClassDef(21, 3, { 4, 5, 6 }),
        };

        /* Test with first unmatching glyph. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 0, 21 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with second unmatching glyph. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 11, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with empty values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, empty, empty }
                        }),
                        { 11, 21 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with positive values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 11, 21 }, { {100, 200}, {600, 700} }, { 300, 800 });
        /* Test with negative values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 11, 21 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in a single class set. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 1, 5, positive2, negative1 },
                            pair_rule { 1, 6, negative1, negative2 }
                        }),
                        { 11, 22 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in a single class set. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 1, 5, positive2, negative1 },
                            pair_rule { 1, 6, negative1, negative2 }
                        }),
                        { 11, 23 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in multiple class sets. */
        testPositioning(builder.createPairPos({ 11, 12, 13 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 2, 5, positive2, negative1 },
                            pair_rule { 3, 6, negative1, negative2 }
                        }),
                        { 12, 22 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({ 11, 12, 13 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 2, 5, positive2, negative1 },
                            pair_rule { 3, 6, negative1, negative2 }
                        }),
                        { 13, 23 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
    }
}

void TextProcessorTester::testCursivePositioning()
{
    Builder builder;

    /* Test with first unmatching glyph. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, &builder.createAnchor(100, 200) },
                        curs_rule { 2, &builder.createAnchor(300, 400), nullptr }
                    }),
                    { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with second unmatching glyph. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, &builder.createAnchor(100, 200) },
                        curs_rule { 2, &builder.createAnchor(300, 400), nullptr }
                    }),
                    { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with no anchor. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, nullptr },
                        curs_rule { 2, nullptr, nullptr }
                    }),
                    { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with non matching anchors. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, &builder.createAnchor(100, 200), nullptr },
                        curs_rule { 2, nullptr, &builder.createAnchor(300, 400) }
                    }),
                    { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with ascending glyph order. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, &builder.createAnchor(100, 200) },
                        curs_rule { 2, &builder.createAnchor(300, 400), nullptr }
                    }),
                    { 1, 2 }, { {0, 0}, {-300, -200} }, { 100, -300 });
    /* Test with dscending glyph order. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, &builder.createAnchor(300, 400), nullptr },
                        curs_rule { 2, nullptr, &builder.createAnchor(100, 200) }
                    }),
                    { 2, 1 }, { {0, 0}, {-300, -200} }, { 100, -300 });
    /* Test with LTR cursive chain. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, &builder.createAnchor(-800, -700) },
                        curs_rule { 2, &builder.createAnchor(-600, -500), &builder.createAnchor(-400, -300) },
                        curs_rule { 3, &builder.createAnchor(-200, -100), &builder.createAnchor(0, 100) },
                        curs_rule { 4, &builder.createAnchor(200, 300), &builder.createAnchor(400, 500) },
                        curs_rule { 5, &builder.createAnchor(600, 700), nullptr },
                    }),
                    { 1, 2, 3, 4, 5 },
                    { {0, 0}, {600, -200}, {200, -400}, {-200, -600}, {-600, -800} },
                    { -800, 200, 200, 200, -600 });
    /* Test with RTL cursive chain. */
    testPositioning(builder.createCursivePos({
                        curs_rule { 1, nullptr, &builder.createAnchor(-800, -700) },
                        curs_rule { 2, &builder.createAnchor(-600, -500), &builder.createAnchor(-400, -300) },
                        curs_rule { 3, &builder.createAnchor(-200, -100), &builder.createAnchor(0, 100) },
                        curs_rule { 4, &builder.createAnchor(200, 300), &builder.createAnchor(400, 500) },
                        curs_rule { 5, &builder.createAnchor(600, 700), nullptr },
                    }),
                    { 1, 2, 3, 4, 5 },
                    { {800, -200}, {400, -400}, {0, -600}, {-400, -800}, {0, -800} },
                    { 800, -200, -200, -200, 600 }, { }, true);
}

void TextProcessorTester::testMarkToBasePositioning()
{
    Glyph markGlyphs[] = { 2 };

    CoverageTable markCoverage;
    markCoverage.coverageFormat = 1;
    markCoverage.format1.glyphCount = sizeof(markGlyphs) / sizeof(Glyph);
    markCoverage.format1.glyphArray = markGlyphs;

    Glyph baseGlyphs[] = { 1 };

    CoverageTable baseCoverage;
    baseCoverage.coverageFormat = 1;
    baseCoverage.format1.glyphCount = sizeof(baseGlyphs) / sizeof(Glyph);
    baseCoverage.format1.glyphArray = baseGlyphs;

    AnchorTable markAnchor;
    markAnchor.anchorFormat = 1;
    markAnchor.xCoordinate = 50;
    markAnchor.yCoordinate = -50;

    MarkRecord markRecord;
    markRecord.clazz = 0;
    markRecord.markAnchor = &markAnchor;

    MarkArrayTable markArray;
    markArray.markCount = 1;
    markArray.markRecord = &markRecord;

    AnchorTable baseAnchor;
    baseAnchor.anchorFormat = 1;
    baseAnchor.xCoordinate = -50;
    baseAnchor.yCoordinate = 50;

    BaseRecord baseRecord;
    baseRecord.baseAnchor = &baseAnchor;

    BaseArrayTable baseArray;
    baseArray.baseCount = 1;
    baseArray.baseRecord = &baseRecord;

    MarkToBaseAttachmentPosSubtable subtable;
    subtable.posFormat = 1;
    subtable.markCoverage = &markCoverage;
    subtable.baseCoverage = &baseCoverage;
    subtable.classCount = 1;
    subtable.markArray = &markArray;
    subtable.baseArray = &baseArray;

    SFAlbum album;
    SFAlbumInitialize(&album);

    SFCodepoint input[] = { 1, 2 };
    processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

    /* Test the output. */
    const SFPoint *actualOffsets = SFAlbumGetGlyphOffsetsPtr(&album);
    const SFAdvance *actualAdvances = SFAlbumGetGlyphAdvancesPtr(&album);
    const SFPoint expectedOffsets[] = { { 0, 0 }, { -100, 100, } };
    const SFAdvance expectedAdvances[] = { 0, 0 };

    SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expectedOffsets) / sizeof(SFPoint)));
    SFAssert(memcmp(actualOffsets, expectedOffsets, sizeof(expectedOffsets) / sizeof(SFPoint)) == 0);
    SFAssert(memcmp(actualAdvances, expectedAdvances, sizeof(expectedAdvances) / sizeof(SFAdvance)) == 0);
}

void TextProcessorTester::testMarkToLigaturePositioning()
{
    Glyph markGlyphs[] = { 2 };

    CoverageTable markCoverage;
    markCoverage.coverageFormat = 1;
    markCoverage.format1.glyphCount = sizeof(markGlyphs) / sizeof(Glyph);
    markCoverage.format1.glyphArray = markGlyphs;

    Glyph ligatureGlyphs[] = { 1 };

    CoverageTable ligatureCoverage;
    ligatureCoverage.coverageFormat = 1;
    ligatureCoverage.format1.glyphCount = sizeof(ligatureGlyphs) / sizeof(Glyph);
    ligatureCoverage.format1.glyphArray = ligatureGlyphs;

    AnchorTable markAnchor;
    markAnchor.anchorFormat = 1;
    markAnchor.xCoordinate = 50;
    markAnchor.yCoordinate = -50;

    MarkRecord markRecord;
    markRecord.clazz = 0;
    markRecord.markAnchor = &markAnchor;

    MarkArrayTable markArray;
    markArray.markCount = 1;
    markArray.markRecord = &markRecord;

    AnchorTable ligatureAnchor;
    ligatureAnchor.anchorFormat = 1;
    ligatureAnchor.xCoordinate = -50;
    ligatureAnchor.yCoordinate = 50;

    ComponentRecord componentRecord;
    componentRecord.ligatureAnchor = &ligatureAnchor;

    LigatureAttachTable ligatureAttach;
    ligatureAttach.componentCount = 1;
    ligatureAttach.componentRecord = &componentRecord;

    LigatureArrayTable ligatureArray;
    ligatureArray.ligatureCount = 1;
    ligatureArray.ligatureAttach = &ligatureAttach;

    MarkToLigatureAttachmentPosSubtable subtable;
    subtable.posFormat = 1;
    subtable.markCoverage = &markCoverage;
    subtable.ligatureCoverage = &ligatureCoverage;
    subtable.classCount = 1;
    subtable.markArray = &markArray;
    subtable.ligatureArray = &ligatureArray;

    SFAlbum album;
    SFAlbumInitialize(&album);

    SFCodepoint input[] = { 1, 2 };
    processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

    /* Test the output. */
    const SFPoint *actualOffsets = SFAlbumGetGlyphOffsetsPtr(&album);
    const SFAdvance *actualAdvances = SFAlbumGetGlyphAdvancesPtr(&album);
    const SFPoint expectedOffsets[] = { { 0, 0 }, { -100, 100, } };
    const SFAdvance expectedAdvances[] = { 0, 0 };

    SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expectedOffsets) / sizeof(SFPoint)));
    SFAssert(memcmp(actualOffsets, expectedOffsets, sizeof(expectedOffsets) / sizeof(SFPoint)) == 0);
    SFAssert(memcmp(actualAdvances, expectedAdvances, sizeof(expectedAdvances) / sizeof(SFAdvance)) == 0);
}

void TextProcessorTester::testMarkToMarkPositioning()
{
    Glyph mark1Glyphs[] = { 2 };

    CoverageTable mark1Coverage;
    mark1Coverage.coverageFormat = 1;
    mark1Coverage.format1.glyphCount = sizeof(mark1Glyphs) / sizeof(Glyph);
    mark1Coverage.format1.glyphArray = mark1Glyphs;

    Glyph mark2Glyphs[] = { 1 };

    CoverageTable mark2Coverage;
    mark2Coverage.coverageFormat = 1;
    mark2Coverage.format1.glyphCount = sizeof(mark2Glyphs) / sizeof(Glyph);
    mark2Coverage.format1.glyphArray = mark2Glyphs;

    AnchorTable mark1Anchor;
    mark1Anchor.anchorFormat = 1;
    mark1Anchor.xCoordinate = 50;
    mark1Anchor.yCoordinate = -50;

    MarkRecord mark1Record;
    mark1Record.clazz = 0;
    mark1Record.markAnchor = &mark1Anchor;

    MarkArrayTable mark1Array;
    mark1Array.markCount = 1;
    mark1Array.markRecord = &mark1Record;

    AnchorTable mark2Anchor;
    mark2Anchor.anchorFormat = 1;
    mark2Anchor.xCoordinate = -50;
    mark2Anchor.yCoordinate = 50;

    Mark2Record mark2Record;
    mark2Record.mark2Anchor = &mark2Anchor;

    Mark2ArrayTable mark2Array;
    mark2Array.mark2Count = 1;
    mark2Array.mark2Record = &mark2Record;

    MarkToMarkAttachmentPosSubtable subtable;
    subtable.posFormat = 1;
    subtable.mark1Coverage = &mark1Coverage;
    subtable.mark2Coverage = &mark2Coverage;
    subtable.classCount = 1;
    subtable.mark1Array = &mark1Array;
    subtable.mark2Array = &mark2Array;

    SFAlbum album;
    SFAlbumInitialize(&album);

    SFCodepoint input[] = { 1, 2 };
    processGPOS(&album, input, sizeof(input) / sizeof(SFCodepoint), subtable);

    /* Test the output. */
    const SFPoint *actualOffsets = SFAlbumGetGlyphOffsetsPtr(&album);
    const SFAdvance *actualAdvances = SFAlbumGetGlyphAdvancesPtr(&album);
    const SFPoint expectedOffsets[] = { { 0, 0 }, { -100, 100, } };
    const SFAdvance expectedAdvances[] = { 0, 0 };

    SFAssert(SFAlbumGetGlyphCount(&album) == (sizeof(expectedOffsets) / sizeof(SFPoint)));
    SFAssert(memcmp(actualOffsets, expectedOffsets, sizeof(expectedOffsets) / sizeof(SFPoint)) == 0);
    SFAssert(memcmp(actualAdvances, expectedAdvances, sizeof(expectedAdvances) / sizeof(SFAdvance)) == 0);
}
