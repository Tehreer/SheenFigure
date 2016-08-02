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

static void writeGSUB(Writer &writer, LookupSubtable &subtable, LookupSubtable *referrals[] = NULL, SFUInteger count = 0)
{
    UInt16 lookupCount = (UInt16)(count + 1);

    /* Create the lookup tables. */
    LookupTable *lookups = new LookupTable[lookupCount];
    lookups[0].lookupType = subtable.lookupType();
    lookups[0].lookupFlag = (LookupFlag)0;
    lookups[0].subTableCount = 1;
    lookups[0].subtables = &subtable;
    lookups[0].markFilteringSet = 0;

    for (SFUInteger i = 1; i < lookupCount; i++) {
        LookupSubtable *other = referrals[i - 1];
        lookups[i].lookupType = other->lookupType();
        lookups[i].lookupFlag = (LookupFlag)0;
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

    delete [] lookups;
}

static void processSubtable(LookupSubtable &subtable, SFAlbumRef album,
    SFCodepoint *input, SFUInteger length, LookupSubtable *referrals[] = NULL, SFUInteger count = 0)
{
    /* Write GSUB table for the given lookup. */
    Writer writer;
    writeGSUB(writer, subtable, referrals, count);

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
        processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint));

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
        processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint));

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
        processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint));

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

    Glyph componenets[] = { 2, 3, 4, 5 };

    /* Create the ligature table. */
    LigatureTable ligature;
    ligature.ligGlyph = 10;
    ligature.compCount = (sizeof(componenets) / sizeof(Glyph)) + 1;
    ligature.component = componenets;

    /* Create the ligature set table. */
    LigatureSetTable ligatureSet;
    ligatureSet.ligatureCount = 1;
    ligatureSet.ligature = &ligature;

    /* Create the ligature substitution table. */
    LigatureSubstSubtable subtable;
    subtable.substFormat = 1;
    subtable.coverage = &coverage;
    subtable.ligSetCount = 1;
    subtable.ligatureSet = &ligatureSet;

    /* Create the album. */
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Process the subtable. */
    SFCodepoint input[] = { 1, 2, 3, 4, 5 };
    processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint));

    /* Test the glyph count. */
    SFAssert(SFAlbumGetGlyphCount(&album) == 1);

    /* Test the output glyphs. */
    const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
    SFAssert(output[0] == 10);
}

void GlyphSubstituterTester::testChainContextSubstitution()
{
    Glyph inputGlyphs[] = { 1, 2, 3 };
    Glyph backtrackGlyphs[] = { 1 };
    Glyph lookaheadGlyphs[] = { 3 };

    /* Create the input coverage table. */
    CoverageTable inputCoverage;
    inputCoverage.coverageFormat = 1;
    inputCoverage.format1.glyphCount = sizeof(inputGlyphs) / sizeof(Glyph);
    inputCoverage.format1.glyphArray = inputGlyphs;

    /* Create an array for input coverage tables. */
    CoverageTable inputArray[] = {
        inputCoverage,
        inputCoverage,
        inputCoverage,
    };

    /* Create the backtrack coverage table. */
    CoverageTable backtrackCoverage;
    backtrackCoverage.coverageFormat = 1;
    backtrackCoverage.format1.glyphCount = sizeof(backtrackGlyphs) / sizeof(Glyph);
    backtrackCoverage.format1.glyphArray = backtrackGlyphs;

    /* Create an array for backtrack coverage tables. */
    CoverageTable backtrackArray[] = {
        backtrackCoverage,
        backtrackCoverage,
        backtrackCoverage,
    };

    /* Create the lookahead coverage table. */
    CoverageTable lookaheadCoverage;
    lookaheadCoverage.coverageFormat = 1;
    lookaheadCoverage.format1.glyphCount = sizeof(lookaheadGlyphs) / sizeof(Glyph);
    lookaheadCoverage.format1.glyphArray = lookaheadGlyphs;

    /* Create an array for lookahead coverage tables. */
    CoverageTable lookaheadArray[] = {
        lookaheadCoverage,
        lookaheadCoverage,
        lookaheadCoverage,
    };

    /* Create the chain context subtable. */
    ChainContextSubtable subtable;
    subtable.format = 3;
    subtable.format3.backtrackGlyphCount = sizeof(backtrackArray) / sizeof(CoverageTable);
    subtable.format3.backtrackGlyphCoverage = backtrackArray;
    subtable.format3.inputGlyphCount = sizeof(inputArray) / sizeof(CoverageTable);
    subtable.format3.inputGlyphCoverage = inputArray;
    subtable.format3.lookaheadGlyphCount = sizeof(lookaheadArray) / sizeof(CoverageTable);
    subtable.format3.lookaheadGlyphCoverage = lookaheadArray;

    /* Test with simple substitutions. */
    {
        /* Create the inner substitution subtable. */
        SingleSubstSubtable inner;
        inner.substFormat = 1;
        inner.coverage = &inputCoverage;
        inner.format1.deltaGlyphID = 1;

        LookupSubtable *referrals[] = { &inner };

        /* Create the lookup record. */
        LookupRecord lookupRecord;
        lookupRecord.sequenceIndex = 1;
        lookupRecord.lookupListIndex = 1;

        /* Update the chain context subtable. */
        subtable.format3.recordCount = 1;
        subtable.format3.lookupRecord = &lookupRecord;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1, 1, 1, 1, 2, 3, 3, 3, 3 };
        processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint), referrals, 1);

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 9);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 1);
        SFAssert(output[1] == 1);
        SFAssert(output[2] == 1);
        SFAssert(output[3] == 1);
        SFAssert(output[4] == 3);
        SFAssert(output[5] == 3);
        SFAssert(output[6] == 3);
        SFAssert(output[7] == 3);
        SFAssert(output[8] == 3);
    }

    /* Test with complex substitutions. */
    {
        Glyph singleGlyphs[] = { 1, 2, 3, 4, 5, 6 };

        /* Create the coverage table for single substitution. */
        CoverageTable singleCoverage;
        singleCoverage.coverageFormat = 1;
        singleCoverage.format1.glyphCount = sizeof(singleGlyphs) / sizeof(Glyph);
        singleCoverage.format1.glyphArray = singleGlyphs;

        /* Create the single substitution subtable. */
        SingleSubstSubtable singleSubst;
        singleSubst.substFormat = 1;
        singleSubst.coverage = &singleCoverage;
        singleSubst.format1.deltaGlyphID = 1;

        Glyph multipleGlyphs[] = { 2 };

        /* Create the coverage table for multiple substitution. */
        CoverageTable multipleCoverage;
        multipleCoverage.coverageFormat = 1;
        multipleCoverage.format1.glyphCount = sizeof(multipleGlyphs) / sizeof(Glyph);
        multipleCoverage.format1.glyphArray = multipleGlyphs;

        Glyph substitutes[] = { 4, 5, 6 };

        /* Create the sequence table. */
        SequenceTable sequence;
        sequence.glyphCount = sizeof(substitutes) / sizeof(Glyph);
        sequence.substitute = substitutes;

        /* Create the multiple substitution subtable. */
        MultipleSubstSubtable multipleSubst;
        multipleSubst.substFormat = 1;
        multipleSubst.coverage = &multipleCoverage;
        multipleSubst.sequenceCount = sizeof(multipleGlyphs) / sizeof(Glyph);
        multipleSubst.sequence = &sequence;

        Glyph ligatureGlyphs[] = { 1, 6 };

        /* Create the coverage table for ligature substitution. */
        CoverageTable ligatureCoverage;
        ligatureCoverage.coverageFormat = 1;
        ligatureCoverage.format1.glyphCount = sizeof(ligatureGlyphs) / sizeof(Glyph);
        ligatureCoverage.format1.glyphArray = ligatureGlyphs;

        Glyph componenets[] = { 4 };

        /* Create the ligature1 table. */
        LigatureTable ligature1;
        ligature1.ligGlyph = 10;
        ligature1.compCount = (sizeof(componenets) / sizeof(Glyph)) + 1;
        ligature1.component = componenets;

        /* Create the ligature2 table. */
        LigatureTable ligature2;
        ligature2.ligGlyph = 20;
        ligature2.compCount = (sizeof(componenets) / sizeof(Glyph)) + 1;
        ligature2.component = componenets;

        /* Create the ligature set table. */
        LigatureSetTable ligatureSet[2];
        ligatureSet[0].ligatureCount = 1;
        ligatureSet[0].ligature = &ligature1;
        ligatureSet[1].ligatureCount = 1;
        ligatureSet[1].ligature = &ligature2;

        /* Create the ligature substitution subtable. */
        LigatureSubstSubtable ligatureSubst;
        ligatureSubst.substFormat = 1;
        ligatureSubst.coverage = &ligatureCoverage;
        ligatureSubst.ligSetCount = sizeof(ligatureSet) / sizeof(LigatureSetTable);
        ligatureSubst.ligatureSet = ligatureSet;

        /* Create the lookup record. */
        LookupRecord lookupRecord[5];
        lookupRecord[0].sequenceIndex = 2;
        lookupRecord[0].lookupListIndex = 1;
        lookupRecord[1].sequenceIndex = 1;
        lookupRecord[1].lookupListIndex = 2;
        lookupRecord[2].sequenceIndex = 3;
        lookupRecord[2].lookupListIndex = 3;
        lookupRecord[3].sequenceIndex = 0;
        lookupRecord[3].lookupListIndex = 3;
        lookupRecord[4].sequenceIndex = 1;
        lookupRecord[4].lookupListIndex = 1;

        LookupSubtable *referrals[] = { &singleSubst, &multipleSubst, &ligatureSubst };

        /* Update the chain context subtable. */
        subtable.format3.recordCount = sizeof(lookupRecord) / sizeof(LookupRecord);
        subtable.format3.lookupRecord = lookupRecord;

        SFAlbum album;
        SFAlbumInitialize(&album);

        SFCodepoint input[] = { 1, 1, 1, 1, 2, 3, 3, 3, 3 };
        processSubtable(subtable, &album, input, sizeof(input) / sizeof(SFCodepoint), referrals, 3);

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 9);

        /* Test the output glyphs. */
        const SFGlyphID *output = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(output[0] == 1);
        SFAssert(output[1] == 1);
        SFAssert(output[2] == 1);
        SFAssert(output[3] == 10);
        SFAssert(output[4] == 6);
        SFAssert(output[5] == 20);
        SFAssert(output[6] == 3);
        SFAssert(output[7] == 3);
        SFAssert(output[8] == 3);
    }
}

void GlyphSubstituterTester::test()
{
    testSingleSubstitution();
    testMultipleSubstitution();
    testLigatureSubstitution();
    testChainContextSubstitution();
}
