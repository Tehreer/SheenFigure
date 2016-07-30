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
#include <SheenFigure/Source/SFPattern.h>
#include <SheenFigure/Source/SFPatternBuilder.h>
#include <SheenFigure/Source/SFTextProcessor.h>
}

#include "GlyphDiscovererTester.h"

using namespace std;
using namespace SheenFigure::Tester;

static const SFCodepoint CODEPOINT_ARRAY[] = { 'G', 'L', 'Y', 'P', 'H' };
static const SFUInteger CODEPOINT_COUNT = sizeof(CODEPOINT_ARRAY) / sizeof(SFCodepoint);

static const SBCodepointSequence CODEPOINT_SEQUENCE = {
    SBStringEncodingUTF32,
    (void *)CODEPOINT_ARRAY,
    CODEPOINT_COUNT
};

static void loadTable(void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length)
{
    if (length) {
        *length = 0;
    }
}

static SFGlyphID getGlyphIDForCodepoint(void *object, SFCodepoint codepoint)
{
    return (SFGlyphID)((codepoint >> 16) ^ (codepoint & 0xFFFF));
}

static SFAdvance getAdvanceForGlyph(void *object, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    switch (fontLayout) {
    case SFFontLayoutHorizontal:
        return glyphID * 10;

    case SFFontLayoutVertical:
        return glyphID * 11;

    default:
        return -1;
    }
}

GlyphDiscovererTester::GlyphDiscovererTester()
{
}

void GlyphDiscovererTester::test()
{
    const SFFontProtocol protocol = {
        .loadTable = &loadTable,
        .getGlyphIDForCodepoint = &getGlyphIDForCodepoint,
        .getAdvanceForGlyph = &getAdvanceForGlyph,
    };
    SFFontRef font = SFFontCreateWithProtocol(&protocol, NULL);

    SFPatternRef pattern = SFPatternCreate();

    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);
    SFPatternBuilderSetFont(&builder, font);
    SFPatternBuilderBuild(&builder);

    SFCodepoints codepoints;
    SFCodepointsInitialize(&codepoints, &CODEPOINT_SEQUENCE, SFFalse);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &codepoints, CODEPOINT_COUNT);

    SFTextProcessor processor;
    SFTextProcessorInitialize(&processor, pattern, &album, SFTextDirectionLeftToRight, SFTextModeForward);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);

    /* Test the glyph count. */
    SFAssert(SFAlbumGetGlyphCount(&album) == 5);

    /* Test the output glyphs. */
    const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
    SFAssert(glyphs[0] == getGlyphIDForCodepoint(NULL, CODEPOINT_ARRAY[0]));
    SFAssert(glyphs[1] == getGlyphIDForCodepoint(NULL, CODEPOINT_ARRAY[1]));
    SFAssert(glyphs[2] == getGlyphIDForCodepoint(NULL, CODEPOINT_ARRAY[2]));
    SFAssert(glyphs[3] == getGlyphIDForCodepoint(NULL, CODEPOINT_ARRAY[3]));
    SFAssert(glyphs[4] == getGlyphIDForCodepoint(NULL, CODEPOINT_ARRAY[4]));

    /* Test the output advances. */
    const SFAdvance *advances = SFAlbumGetGlyphAdvancesPtr(&album);
    SFAssert(advances[0] == getAdvanceForGlyph(NULL, SFFontLayoutHorizontal, glyphs[0]));
    SFAssert(advances[1] == getAdvanceForGlyph(NULL, SFFontLayoutHorizontal, glyphs[1]));
    SFAssert(advances[2] == getAdvanceForGlyph(NULL, SFFontLayoutHorizontal, glyphs[2]));
    SFAssert(advances[3] == getAdvanceForGlyph(NULL, SFFontLayoutHorizontal, glyphs[3]));
    SFAssert(advances[4] == getAdvanceForGlyph(NULL, SFFontLayoutHorizontal, glyphs[4]));

    SFPatternRelease(pattern);
    SFFontRelease(font);
}
