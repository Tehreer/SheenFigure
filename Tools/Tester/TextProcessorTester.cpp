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

static SFGlyphID getGlyphID(void *object, SFCodepoint codepoint)
{
    return (SFGlyphID)codepoint;
}

static void writeTable(Writer &writer,
    LookupSubtable &subtable, LookupSubtable **referrals, SFUInteger count, LookupFlag lookupFlag)
{
    UInt16 lookupCount = (UInt16)(count + 1);

    /* Create the lookup tables. */
    LookupTable *lookups = new LookupTable[lookupCount];
    lookups[0].lookupType = subtable.lookupType();
    lookups[0].lookupFlag = lookupFlag;
    lookups[0].subTableCount = 1;
    lookups[0].subtables = &subtable;
    lookups[0].markFilteringSet = 0;

    for (SFUInteger i = 1; i < lookupCount; i++) {
        LookupSubtable *other = referrals[i - 1];
        lookups[i].lookupType = other->lookupType();
        lookups[i].lookupFlag = lookupFlag;
        lookups[i].subTableCount = 1;
        lookups[i].subtables = other;
        lookups[i].markFilteringSet = 0;
    }

    /* Create the lookup list table. */
    LookupListTable lookupList;
    lookupList.lookupCount = lookupCount;
    lookupList.lookupTables = lookups;

    UInt16 lookupIndex[1] = { 0 };

    /* Create the feature table. */
    FeatureTable testFeature;
    testFeature.featureParams = 0;
    testFeature.lookupCount = 1;
    testFeature.lookupListIndex = lookupIndex;

    /* Create the feature record. */
    FeatureRecord featureRecord;
    memcpy(&featureRecord.featureTag, "test", 4);
    featureRecord.feature = &testFeature;

    /* Create the feature list table. */
    FeatureListTable featureList;
    featureList.featureCount = 1;
    featureList.featureRecord = &featureRecord;

    UInt16 dfltFeatureIndex[] = { 0 };

    /* Create the language system table. */
    LangSysTable dfltLangSys;
    dfltLangSys.lookupOrder = 0;
    dfltLangSys.reqFeatureIndex = 0xFFFF;
    dfltLangSys.featureCount = 1;
    dfltLangSys.featureIndex = dfltFeatureIndex;

    /* Create the script table. */
    ScriptTable dfltScript;
    dfltScript.defaultLangSys = &dfltLangSys;
    dfltScript.langSysCount = 0;
    dfltScript.langSysRecord = NULL;

    /* Create the script record. */
    ScriptRecord scripts[1];
    memcpy(&scripts[0].scriptTag, "dflt", 4);
    scripts[0].script = &dfltScript;

    /* Create the script list table */
    ScriptListTable scriptList;
    scriptList.scriptCount = 1;
    scriptList.scriptRecord = scripts;

    /* Create the container table. */
    GSUB gsub;
    gsub.version = 0x00010000;
    gsub.scriptList = &scriptList;
    gsub.featureList = &featureList;
    gsub.lookupList = &lookupList;

    writer.write(&gsub);

    delete [] lookups;
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
    SFPatternBuilderAddFeature(&builder, SFTagMake('t', 'e', 's', 't'), 0);
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
    SFAlbumReset(album, &codepoints, length);

    /* Process the album. */
    SFTextProcessor processor;
    SFTextProcessorInitialize(&processor, pattern, album, direction, SFTextModeForward);
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

    SFAssert(SFAlbumGetGlyphCount(&album) == glyphs.size());
    SFAssert(memcmp(SFAlbumGetGlyphIDsPtr(&album), glyphs.data(), sizeof(SFGlyphID) * glyphs.size()) == 0);
}

void TextProcessorTester::testPositioning(LookupSubtable &subtable,
    const vector<uint32_t> codepoints,
    const vector<pair<int32_t, int32_t>> offsets,
    const vector<int32_t> advances,
    const vector<LookupSubtable *> referrals,
    bool isRTL)
{
    SFAssert(offsets.size() == advances.size());

    SFAlbum album;
    SFAlbumInitialize(&album);
    processSubtable(&album, &codepoints[0], codepoints.size(), SFTrue, subtable,
                    (LookupSubtable **)referrals.data(), referrals.size(), isRTL);

    SFAssert(SFAlbumGetGlyphCount(&album) == offsets.size());
    SFAssert(memcmp(SFAlbumGetGlyphOffsetsPtr(&album), offsets.data(), sizeof(SFPoint) * offsets.size()) == 0);
    SFAssert(memcmp(SFAlbumGetGlyphAdvancesPtr(&album), advances.data(), sizeof(SFInt32) * advances.size()) == 0);
}

void TextProcessorTester::test()
{
    testSingleSubstitution();
    testMultipleSubstitution();
    testLigatureSubstitution();
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
