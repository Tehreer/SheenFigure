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
#include <cstring>

extern "C" {
#include <Source/SFPattern.h>
#include <Source/SFScheme.h>
}

#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "Utilities/SFPattern+Testing.h"
#include "SchemeTester.h"

using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

static const int CCMP = 0;
static const int CLIG = 1;
static const int CUST = 2;
static const int DIST = 3;
static const int KERN = 4;
static const int LIGA = 5;
static const int MARK = 6;
static const int MKMK = 7;
static const int TPOS = 8;
static const int TSUB = 9;

static void loadTable(void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length)
{
    if (tag == 'GSUB') {
        Builder builder;

        FeatureListTable &featureList = builder.createFeatureList({
            {'ccmp', builder.createFeature({ CCMP })},
            {'clig', builder.createFeature({ CLIG })},
            {'cust', builder.createFeature({ CUST })},
            {'liga', builder.createFeature({ LIGA })},
            {'tsub', builder.createFeature({ TSUB })},
        });
        ScriptListTable &scriptList = builder.createScriptList({
            {'latn', builder.createScript(builder.createLangSys({ 0, 1, 2, 3, 4 }), {
                                              {'ENG ', builder.createLangSys({ 3 })}
                                          })}
        });

        GSUB gsub;
        gsub.version = 0x00010000;
        gsub.scriptList = &scriptList;
        gsub.featureList = &featureList;
        gsub.lookupList = NULL;
        
        Writer writer;
        writer.write(&gsub);

        if (length) {
            *length = (SFUInteger)writer.size();
        }

        if (buffer) {
            memcpy(buffer, writer.data(), (size_t)writer.size());
        }
    } else if (tag == 'GPOS') {
        Builder builder;

        FeatureListTable &featureList = builder.createFeatureList({
            {'cust', builder.createFeature({ CUST })},
            {'dist', builder.createFeature({ DIST })},
            {'kern', builder.createFeature({ KERN })},
            {'mark', builder.createFeature({ MARK })},
            {'mkmk', builder.createFeature({ MKMK })},
            {'tpos', builder.createFeature({ TPOS })},
        });
        ScriptListTable &scriptList = builder.createScriptList({
            {'latn', builder.createScript(builder.createLangSys({ 0, 1, 2, 3, 4, 5 }), {
                                              {'ENG ', builder.createLangSys({ 2 })}
                                          })}
        });

        GPOS gpos;
        gpos.version = 0x00010000;
        gpos.scriptList = &scriptList;
        gpos.featureList = &featureList;
        gpos.lookupList = NULL;

        Writer writer;
        writer.write(&gpos);

        if (length) {
            *length = (SFUInteger)writer.size();
        }

        if (buffer) {
            memcpy(buffer, writer.data(), (size_t)writer.size());
        }
    }
}

static SFGlyphID getGlyphIDForCodepoint(void *object, SFCodepoint codepoint)
{
    return 0;
}

static SFAdvance getAdvanceForGlyph(void *object, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    return 0;
}

SchemeTester::SchemeTester()
{
}

void SchemeTester::testFeatures()
{
    /* Test unique features. */
    {
        SFSchemeRef scheme = SFSchemeCreate();

        SFTag featureTags[] = {
            'ccmp', 'liga', 'dist', 'kern'
        };
        SFUInt16 featureValues[] = {
            1, 2, 3, 4
        };
        SFUInteger featureCount = sizeof(featureValues) / sizeof(SFUInt16);

        SFSchemeSetFeatureValues(scheme, featureTags, featureValues, featureCount);

        assert(memcmp(scheme->_featureTags, featureTags, sizeof(featureTags)) == 0);
        assert(memcmp(scheme->_featureValues, featureValues, sizeof(featureValues)) == 0);
        assert(scheme->_featureCount == featureCount);

        SFSchemeRelease(scheme);
    }

    /* Test duplicated features. */
    {
        SFSchemeRef scheme = SFSchemeCreate();

        SFTag featureTags[] = {
            'ccmp', 'liga', 'dist', 'kern', 'ccmp', 'kern'
        };
        SFUInt16 featureValues[] = {
            1, 2, 3, 4, 5, 6
        };
        SFUInteger featureCount = sizeof(featureValues) / sizeof(SFUInt16);

        SFSchemeSetFeatureValues(scheme, featureTags, featureValues, featureCount);

        SFTag expectedTags[] = {
            'ccmp', 'liga', 'dist', 'kern'
        };
        SFUInt16 expectedValues[] = {
            5, 2, 3, 6
        };
        SFUInteger expectedCount = sizeof(expectedValues) / sizeof(SFUInt16);

        assert(memcmp(scheme->_featureTags, expectedTags, sizeof(expectedTags)) == 0);
        assert(memcmp(scheme->_featureValues, expectedValues, sizeof(expectedValues)) == 0);
        assert(scheme->_featureCount == expectedCount);

        SFSchemeRelease(scheme);
    }
}

void SchemeTester::testBuild()
{
    const SFFontProtocol protocol = {
        .finalize = NULL,
        .loadTable = &loadTable,
        .getGlyphIDForCodepoint = &getGlyphIDForCodepoint,
        .getAdvanceForGlyph = &getAdvanceForGlyph,
    };
    SFFontRef font = SFFontCreateWithProtocol(&protocol, NULL);

    SFSchemeRef scheme = SFSchemeCreate();
    SFSchemeSetFont(scheme, font);
    SFSchemeSetScriptTag(scheme, 'latn');

    /* Test with default language. */
    {
        SFSchemeSetLanguageTag(scheme, 'dflt');
        SFPatternRef pattern = SFSchemeBuildPattern(scheme);

        SFTag expectedTags[] = {
            'ccmp', 'liga', 'clig',
            'dist', 'kern', 'mark', 'mkmk',
        };
        SFLookupInfo expectedLookups[] = {
            {CCMP, 1}, {LIGA, 1}, {CLIG, 1},
            {DIST, 1}, {KERN, 1}, {MARK, 1}, {MKMK, 1},
        };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 1 }, { 0, 1 }, 0x00 },
            { { &expectedLookups[1], 1 }, { 1, 1 }, 0x00 },
            { { &expectedLookups[2], 1 }, { 2, 1 }, 0x00 },
            { { &expectedLookups[3], 1 }, { 3, 1 }, 0x00 },
            { { &expectedLookups[4], 1 }, { 4, 1 }, 0x00 },
            { { &expectedLookups[5], 1 }, { 5, 1 }, 0x00 },
            { { &expectedLookups[6], 1 }, { 6, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 7 },
            .featureUnits = { expectedUnits, 3, 4 },
            .scriptTag = 'latn',
            .languageTag = 'dflt',
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test with a non-default language. */
    {
        SFSchemeSetLanguageTag(scheme, 'ENG ');
        SFPatternRef pattern = SFSchemeBuildPattern(scheme);

        SFTag expectedTags[] = { 'liga', 'kern' };
        SFLookupInfo expectedLookups[] = { {LIGA, 1}, {KERN, 1} };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 1 }, { 0, 1 }, 0x00 },
            { { &expectedLookups[1], 1 }, { 1, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 2 },
            .featureUnits = { expectedUnits, 1, 1 },
            .scriptTag = 'latn',
            .languageTag = 'ENG ',
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test by trying to turn off a required feature. */
    /* Test by disabling a feature marked as on by default. */
    /* Test by enabling a feature marked as off by default. (MISSING) */
    /* Test by setting custom value of a required feature. */
    /* Test by setting custom value of a feature marked as on by default. */
    /* Test by enabling simultaneous features across multiple groups. (MISSING) */
    /* Test by enabling custom features of GSUB and GPOS. */
    /* Test by enabling a custom feature available in both GSUB and GPOS. */
    {
        SFTag featureTags[] = {
            'mark', 'liga', 'mkmk', 'clig', 'tsub', 'tpos', 'cust'
        };
        SFUInt16 featureValues[] = {
            0, 0, 2, 3, 4, 5, 6
        };

        SFSchemeSetLanguageTag(scheme, 'dflt');
        SFSchemeSetFeatureValues(scheme, featureTags, featureValues, sizeof(featureValues) / sizeof(SFUInt16));

        SFPatternRef pattern = SFSchemeBuildPattern(scheme);

        SFTag expectedTags[] = {
            'ccmp', 'clig', 'tsub', 'cust',
            'dist', 'kern', 'mark', 'mkmk', 'tpos'
        };
        SFLookupInfo expectedLookups[] = {
            {CCMP, 1}, {CLIG, 3}, {CUST, 6}, {TSUB, 4},
            {DIST, 1}, {KERN, 1}, {MARK, 1}, {MKMK, 2}, {TPOS, 5},
        };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 1 }, { 0, 1 }, 0x00 },
            { { &expectedLookups[1], 1 }, { 1, 1 }, 0x00 },
            { { &expectedLookups[2], 2 }, { 2, 2 }, 0x00 },
            { { &expectedLookups[4], 1 }, { 4, 1 }, 0x00 },
            { { &expectedLookups[5], 1 }, { 5, 1 }, 0x00 },
            { { &expectedLookups[6], 1 }, { 6, 1 }, 0x00 },
            { { &expectedLookups[7], 1 }, { 7, 1 }, 0x00 },
            { { &expectedLookups[8], 1 }, { 8, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 9 },
            .featureUnits = { expectedUnits, 3, 5 },
            .scriptTag = 'latn',
            .languageTag = 'dflt',
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    SFSchemeRelease(scheme);
    SFFontRelease(font);
}

void SchemeTester::test()
{
    testFeatures();
    testBuild();
}
