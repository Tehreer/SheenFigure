/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" {
#include <Source/SFAlbum.h>
#include <Source/SFBase.h>
#include <Source/SFPattern.h>
#include <Source/SFPatternBuilder.h>
#include <Source/SFTextProcessor.h>
}

#include "OpenType/Base.h"
#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "OpenType/Writer.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

struct FontObject {
    Writer &writer;
    SFTag tag;
};

static void loadTable(void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length)
{
    FontObject *fontObject = reinterpret_cast<FontObject *>(object);

    if (tag == fontObject->tag) {
        if (buffer) {
            memcpy(buffer, fontObject->writer.data(), (size_t)fontObject->writer.size());
        }
        if (length) {
            *length = (SFUInteger)fontObject->writer.size();
        }
    }
}

static SFGlyphID getGlyphID(void *, SFCodepoint codepoint)
{
    return (SFGlyphID)codepoint;
}

static void writeTable(Writer &writer,
    LookupSubtable &subtable, LookupSubtable **referrals, SFUInteger count, LookupFlag lookupFlag)
{
    Builder builder;

    vector<reference_wrapper<LookupTable>> lookups;
    lookups.push_back(builder.createLookup({&subtable, 1}, lookupFlag));
    for (size_t i = 0; i < count; i++) {
        lookups.push_back(builder.createLookup({referrals[i], 1}, lookupFlag));
    }

    LookupListTable &lookupList = builder.createLookupList(lookups);
    FeatureListTable &featureList = builder.createFeatureList({
        {'test', builder.createFeature({ 0 })},
    });
    ScriptListTable &scriptList = builder.createScriptList({
        {'dflt', builder.createScript(builder.createLangSys({ 0 }))}
    });

    /* Create the container table. */
    GSUB gsub;
    gsub.version = 0x00010000;
    gsub.scriptList = &scriptList;
    gsub.featureList = &featureList;
    gsub.lookupList = &lookupList;

    writer.write(&gsub);
}

static void processSubtable(SFAlbumRef album,
    const SFCodepoint *input, SFUInteger length, SFBoolean positioning,
    LookupSubtable &subtable, LookupSubtable **referrals, SFUInteger count,
    SFBoolean isRTL = SFFalse)
{
    /* Write the table for the given lookup. */
    Writer writer;
    writeTable(writer, subtable, referrals, count, isRTL ? LookupFlag::RightToLeft : (LookupFlag)0);

    /* Create a font object containing writer and tag. */
    FontObject object = {
        .writer = writer,
        .tag = (positioning ? SFTagMake('G', 'P', 'O', 'S') : SFTagMake('G', 'S', 'U', 'B')),
    };

    /* Create the font with protocol. */
    SFFontProtocol protocol = {
        .finalize = NULL,
        .loadTable = &loadTable,
        .getGlyphIDForCodepoint = &getGlyphID,
        .getAdvanceForGlyph = NULL,
    };
    SFFontRef font = SFFontCreateWithProtocol(&protocol, &object);
    SFTextDirection direction = isRTL ? SFTextDirectionRightToLeft : SFTextDirectionLeftToRight;

    /* Create a pattern. */
    SFPatternRef pattern = SFPatternCreate();

    /* Build the pattern. */
    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);
    SFPatternBuilderSetFont(&builder, font);
    SFPatternBuilderSetScript(&builder, SFTagMake('d', 'f', 'l', 't'), direction);
    SFPatternBuilderSetLanguage(&builder, SFTagMake('d', 'f', 'l', 't'));
    SFPatternBuilderBeginFeatures(&builder, positioning ? SFFeatureKindPositioning : SFFeatureKindSubstitution);
    SFPatternBuilderAddFeature(&builder, SFTagMake('t', 'e', 's', 't'), 1, 0);
    SFPatternBuilderAddLookup(&builder, 0);
    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);
    SFPatternBuilderBuild(&builder);

    /* Create the codepoint sequence. */
    SBCodepointSequence sequence;
    sequence.stringEncoding = SBStringEncodingUTF32;
    sequence.stringBuffer = (void *)input;
    sequence.stringLength = length;

    /* Reset the album for given codepoints. */
    SFCodepoints codepoints;
    SFCodepointsInitialize(&codepoints, &sequence, SFFalse);
    SFAlbumReset(album, &codepoints);

    /* Process the album. */
    SFTextProcessor processor;
    SFTextProcessorInitialize(&processor, pattern, album, direction, SFTextModeForward, SFFalse);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);
    
    /* Release the allocated objects. */
    SFPatternRelease(pattern);
    SFFontRelease(font);
}

TextProcessorTester::TextProcessorTester()
{
}

void TextProcessorTester::testSubstitution(LookupSubtable &subtable,
    const vector<uint32_t> codepoints,
    const vector<Glyph> glyphs,
    const vector<LookupSubtable *> referrals)
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    processSubtable(&album, &codepoints[0], codepoints.size(), SFFalse, subtable,
                    (LookupSubtable **)referrals.data(), referrals.size());

    assert(SFAlbumGetGlyphCount(&album) == glyphs.size());
    assert(memcmp(SFAlbumGetGlyphIDsPtr(&album), glyphs.data(), sizeof(SFGlyphID) * glyphs.size()) == 0);
}

void TextProcessorTester::testPositioning(LookupSubtable &subtable,
    const vector<uint32_t> codepoints,
    const vector<pair<int32_t, int32_t>> offsets,
    const vector<int32_t> advances,
    const vector<LookupSubtable *> referrals,
    bool isRTL)
{
    assert(offsets.size() == advances.size());

    SFAlbum album;
    SFAlbumInitialize(&album);
    processSubtable(&album, &codepoints[0], codepoints.size(), SFTrue, subtable,
                    (LookupSubtable **)referrals.data(), referrals.size(), isRTL);

    assert(SFAlbumGetGlyphCount(&album) == offsets.size());
    assert(memcmp(SFAlbumGetGlyphOffsetsPtr(&album), offsets.data(), sizeof(SFPoint) * offsets.size()) == 0);
    assert(memcmp(SFAlbumGetGlyphAdvancesPtr(&album), advances.data(), sizeof(SFInt32) * advances.size()) == 0);
}

void TextProcessorTester::test()
{
    testSingleSubstitution();
    testMultipleSubstitution();
    testAlternateSubstitution();
    testLigatureSubstitution();
    testReverseChainContextSubstitution();
    testSinglePositioning();
    testPairPositioning();
    testCursivePositioning();
    testMarkToBasePositioning();
    testMarkToLigaturePositioning();
    testMarkToMarkPositioning();
    testContextSubtable();
    testChainContextSubtable();
    testExtensionSubtable();
}
