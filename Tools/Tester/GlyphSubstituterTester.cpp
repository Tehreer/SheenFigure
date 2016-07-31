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

#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "GlyphSubstituterTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

static void loadTable(void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length)
{
    Writer *writer = reinterpret_cast<Writer *>(object);

    switch (tag) {
    case SFTagMake('G', 'S', 'U', 'B'):
        if (buffer) {
            memcpy(buffer, writer->data(), (size_t)writer->size());
        }
        if (length) {
            *length = (SFUInteger)writer->size();
        }
        break;
    }
}

static SFGlyphID getGlyphID(void *object, SFCodepoint codepoint)
{
    return (SFGlyphID)codepoint;
}

static SFAdvance getGlyphAdvance(void *object, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    return 0;
}

static void writeGSUB(Writer &writer, Table &subtable, LookupType type)
{
    /* Create the lookup table. */
    LookupTable lookup;
    lookup.lookupType = type;
    lookup.lookupFlag = (LookupFlag)0;
    lookup.subTableCount = 1;
    lookup.subtables = &subtable;
    lookup.markFilteringSet = 0;

    /* Create the lookup list table. */
    LookupListTable lookupList;
    lookupList.lookupCount = 1;
    lookupList.lookupTables = &lookup;

    UInt16 lookupIndex[1] = { 0 };

    /* Create the feature table. */
    FeatureTable testFeature;
    testFeature.featureParams = 0;
    testFeature.lookupCount = 1;
    testFeature.lookupListIndex = lookupIndex;

    /* Create the feature record. */
    FeatureRecord featureRecord[1];
    memcpy(&featureRecord[0].featureTag, "test", 4);
    featureRecord[0].feature = &testFeature;

    /* Create the feature list table. */
    FeatureListTable featureList;
    featureList.featureCount = 1;
    featureList.featureRecord = featureRecord;

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

    /* Create the gsub table. */
    GSUB gsub;
    gsub.version = 0x00010000;
    gsub.scriptList = &scriptList;
    gsub.featureList = &featureList;
    gsub.lookupList = &lookupList;

    writer.writeTable(&gsub);
}

static void processSubtable(Table &subtable, LookupType type, SFAlbumRef album, SFCodepoint *input, SFUInteger length)
{
    /* Write GSUB table for the given lookup. */
    Writer writer;
    writeGSUB(writer, subtable, type);

    /* Create font with protocol. */
    SFFontProtocol protocol = {
        .loadTable = &loadTable,
        .getGlyphIDForCodepoint = &getGlyphID,
        .getAdvanceForGlyph = &getGlyphAdvance,
    };
    SFFontRef font = SFFontCreateWithProtocol(&protocol, &writer);

    /* Create a pattern. */
    SFPatternRef pattern = SFPatternCreate();

    /* Build the pattern. */
    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);
    SFPatternBuilderSetFont(&builder, font);
    SFPatternBuilderSetScript(&builder, SFTagMake('d', 'f', 'l', 't'), SFTextDirectionLeftToRight);
    SFPatternBuilderSetLanguage(&builder, SFTagMake('d', 'f', 'l', 't'));
    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);
    SFPatternBuilderAddFeature(&builder, SFTagMake('t', 'e', 's', 't'), 0);
    SFPatternBuilderAddLookup(&builder, 0);
    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);
    SFPatternBuilderBuild(&builder);

    /* Create the codepoint sequence. */
    SBCodepointSequence sequence;
    sequence.stringEncoding = SBStringEncodingUTF32;
    sequence.stringBuffer = input;
    sequence.stringLength = length;

    /* Reset the album for given codepoints. */
    SFCodepoints codepoints;
    SFCodepointsInitialize(&codepoints, &sequence, SFFalse);
    SFAlbumReset(album, &codepoints, length);

    /* Process the album. */
    SFTextProcessor processor;
    SFTextProcessorInitialize(&processor, pattern, album, SFTextDirectionLeftToRight, SFTextModeForward);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);

    /* Release the allocated objects. */
    SFPatternRelease(pattern);
    SFFontRelease(font);
}

GlyphSubstituterTester::GlyphSubstituterTester()
{
}

void GlyphSubstituterTester::testSingleSubstitution()
{
    Glyph glyphs[] = { 1 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    /* Test with format 1. */
    {
        SingleSubstSubtable subtable;
        subtable.substFormat = 1;
        subtable.coverage = &coverage;
        subtable.format1.deltaGlyphID = -1;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1 };
        processSubtable(subtable, LookupType::sSingle, &album, input, sizeof(input) / sizeof(SFCodepoint));

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 1);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 0);
    }

    /* Test with format 2. */
    {
        Glyph substitutes[] = { 10 };

        SingleSubstSubtable subtable;
        subtable.substFormat = 2;
        subtable.coverage = &coverage;
        subtable.format2.glyphCount = sizeof(substitutes) / sizeof(Glyph);
        subtable.format2.substitute = substitutes;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1 };
        processSubtable(subtable, LookupType::sSingle, &album, input, sizeof(input) / sizeof(SFCodepoint));

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 1);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 10);
    }
}

void GlyphSubstituterTester::testMultipleSubstitution()
{
    Glyph glyphs[] = { 1 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    /* Test with format 1. */
    {
        Glyph substitutes[] = { 0, 1, 2, 3, 4 };

        SequenceTable sequence;
        sequence.glyphCount = sizeof(substitutes) / sizeof(Glyph);
        sequence.substitute = substitutes;

        MultipleSubstSubtable subtable;
        subtable.substFormat = 1;
        subtable.coverage = &coverage;
        subtable.sequenceCount = sizeof(glyphs) / sizeof(Glyph);
        subtable.sequence = &sequence;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1 };
        processSubtable(subtable, LookupType::sMultiple, &album, input, sizeof(input) / sizeof(SFCodepoint));

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 0);
        SFAssert(output[1] == 1);
        SFAssert(output[2] == 2);
        SFAssert(output[3] == 3);
        SFAssert(output[4] == 4);
    }
}

void GlyphSubstituterTester::testLigatureSubstitution()
{
    Glyph glyphs[] = { 1 };

    /* Create the coverage table. */
    CoverageTable coverage;
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = sizeof(glyphs) / sizeof(Glyph);
    coverage.format1.glyphArray = glyphs;

    /* Test with format 1. */
    {
        Glyph componenets[] = { 2, 3, 4, 5 };

        LigatureTable ligature;
        ligature.ligGlyph = 10;
        ligature.compCount = (sizeof(componenets) / sizeof(Glyph)) + 1;
        ligature.component = componenets;

        LigatureSetTable ligatureSet;
        ligatureSet.ligatureCount = 1;
        ligatureSet.ligature = &ligature;

        LigatureSubstSubtable subtable;
        subtable.substFormat = 1;
        subtable.coverage = &coverage;
        subtable.ligSetCount = 1;
        subtable.ligatureSet = &ligatureSet;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1, 2, 3, 4, 5 };
        processSubtable(subtable, LookupType::sLigature, &album, input, sizeof(input) / sizeof(SFCodepoint));

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 1);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 10);
    }
}

void GlyphSubstituterTester::test()
{
    testSingleSubstitution();
    testMultipleSubstitution();
    testLigatureSubstitution();
}
