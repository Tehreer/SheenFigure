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
#include <vector>

extern "C" {
#include <Source/SFPattern.h>
#include <Source/SFScheme.h>
#include <Source/SFShapingKnowledge.h>
}

#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "Utilities/General.h"
#include "Utilities/SFPattern+Testing.h"
#include "SchemeTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;
using namespace SheenFigure::Tester::Utilities;

class TestKnowledge {
public:
    static SFShapingKnowledgeRef instance() { return &object->knowledge.shape; }

private:
    static TestKnowledge *object;

    struct {
        vector<SFFeatureInfo> subst;
        vector<SFFeatureInfo> pos;
    } features;
    struct {
        SFScriptKnowledge script;
        SFShapingKnowledge shape;
    } knowledge;

    static SFScriptKnowledgeRef seekScript(const void *, SFTag scriptTag)
    {
        if (scriptTag == tag("test")) {
            return &object->knowledge.script;
        }

        return NULL;
    }

    TestKnowledge() {
        features.subst = {
            /* First Group. */
            { 1, tag("srqi"), REQUIRED,       INDIVIDUAL,   0x00 },
            { 1, tag("srq1"), REQUIRED,       SIMULTANEOUS, 0x00 },
            { 1, tag("son1"), ON_BY_DEFAULT,  SIMULTANEOUS, 0x00 },
            { 1, tag("sof1"), OFF_BY_DEFAULT, SIMULTANEOUS, 0x00 },
            /* Second Group. */
            { 2, tag("sim1"), REQUIRED,       SIMULTANEOUS, 0x00 },
            { 2, tag("sim2"), REQUIRED,       SIMULTANEOUS, 0x00 },
        };
        features.pos = {
            /* Third Group. */
            { 3, tag("pof1"), OFF_BY_DEFAULT, SIMULTANEOUS, 0x00 },
            { 3, tag("pon1"), ON_BY_DEFAULT,  SIMULTANEOUS, 0x00 },
            { 3, tag("prq1"), REQUIRED,       SIMULTANEOUS, 0x00 },
            { 3, tag("prqi"), REQUIRED,       INDIVIDUAL,   0x00 },
        };
        knowledge.script = {
            SFTextDirectionLeftToRight,
            { features.subst.data(), features.subst.size() },
            { features.pos.data(), features.pos.size() }
        };
        knowledge.shape = {
            &seekScript
        };
    }
};
TestKnowledge *TestKnowledge::object = new TestKnowledge();

static const UInt16 CUST = 0;
static const UInt16 PCST = 1;
static const UInt16 POF1 = 2;
static const UInt16 PON1 = 3;
static const UInt16 PRQ1 = 4;
static const UInt16 PRQI = 5;
static const UInt16 SCST = 6;
static const UInt16 SIM1 = 7;
static const UInt16 SIM2 = 8;
static const UInt16 SOF1 = 9;
static const UInt16 SON1 = 10;
static const UInt16 SRQ1 = 11;
static const UInt16 SRQI = 12;

static void loadTable(void *, SFTag tableTag, SFUInt8 *buffer, SFUInteger *length)
{
    if (tableTag == tag("GSUB")) {
        Builder builder;

        FeatureListTable &featureList = builder.createFeatureList({
            {tag("cust"), builder.createFeature({ CUST })},
            {tag("scst"), builder.createFeature({ SCST })},
            {tag("sim1"), builder.createFeature({ SIM1 })},
            {tag("sim2"), builder.createFeature({ SIM2 })},
            {tag("sof1"), builder.createFeature({ SOF1 })},
            {tag("son1"), builder.createFeature({ SON1 })},
            {tag("srq1"), builder.createFeature({ SRQ1 })},
            {tag("srqi"), builder.createFeature({ SRQI })},
        });
        ScriptListTable &scriptList = builder.createScriptList({
            {tag("test"), builder.createScript(builder.createLangSys({ 0, 1, 2, 3, 4, 5, 6, 7 }), {
                                                   {tag("LNG "), builder.createLangSys({ 7 })}
                                               })}
        });
        GSUB &gsub = builder.createGSUB(&scriptList, &featureList, NULL);

        Writer writer;
        writer.write(&gsub);

        if (length) {
            *length = (SFUInteger)writer.size();
        }
        if (buffer) {
            memcpy(buffer, writer.data(), (size_t)writer.size());
        }
    } else if (tableTag == tag("GPOS")) {
        Builder builder;

        FeatureListTable &featureList = builder.createFeatureList({
            {tag("cust"), builder.createFeature({ CUST })},
            {tag("pcst"), builder.createFeature({ PCST })},
            {tag("pof1"), builder.createFeature({ POF1 })},
            {tag("pon1"), builder.createFeature({ PON1 })},
            {tag("prq1"), builder.createFeature({ PRQ1 })},
            {tag("prqi"), builder.createFeature({ PRQI })},
        });
        ScriptListTable &scriptList = builder.createScriptList({
            {tag("test"), builder.createScript(builder.createLangSys({ 0, 1, 2, 3, 4, 5 }), {
                                                   {tag("LNG "), builder.createLangSys({ 2 })}
                                               })}
        });
        GPOS &gpos = builder.createGPOS(&scriptList, &featureList, NULL);

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

static SFGlyphID getGlyphIDForCodepoint(void *, SFCodepoint)
{
    return 0;
}

static SFAdvance getAdvanceForGlyph(void *, SFFontLayout, SFGlyphID)
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
            tag("ccmp"), tag("liga"), tag("dist"), tag("kern")
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
            tag("ccmp"), tag("liga"), tag("dist"), tag("kern"), tag("ccmp"), tag("kern")
        };
        SFUInt16 featureValues[] = {
            1, 2, 3, 4, 5, 6
        };
        SFUInteger featureCount = sizeof(featureValues) / sizeof(SFUInt16);

        SFSchemeSetFeatureValues(scheme, featureTags, featureValues, featureCount);

        SFTag expectedTags[] = {
            tag("ccmp"), tag("liga"), tag("dist"), tag("kern")
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

    SFScheme scheme;
    SFSchemeInitialize(&scheme, TestKnowledge::instance());
    SFSchemeSetFont(&scheme, font);
    SFSchemeSetScriptTag(&scheme, tag("test"));

    /* Test with default language. */
    {
        SFSchemeSetLanguageTag(&scheme, tag("dflt"));
        SFPatternRef pattern = SFSchemeBuildPattern(&scheme);

        SFTag expectedTags[] = {
            tag("srqi"), tag("srq1"), tag("son1"), tag("sim1"), tag("sim2"),
            tag("pon1"), tag("prq1"), tag("prqi")
        };
        SFLookupInfo expectedLookups[] = {
            {SRQI, 1}, {SON1, 1}, {SRQ1, 1}, {SIM1, 1}, {SIM2, 1},
            {PON1, 1}, {PRQ1, 1}, {PRQI, 1}
        };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 1 }, { 0, 1 }, 0x00 },
            { { &expectedLookups[1], 2 }, { 1, 2 }, 0x00 },
            { { &expectedLookups[3], 2 }, { 3, 2 }, 0x00 },
            { { &expectedLookups[5], 2 }, { 5, 2 }, 0x00 },
            { { &expectedLookups[7], 1 }, { 7, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 8 },
            .featureUnits = { expectedUnits, 3, 2 },
            .scriptTag = tag("test"),
            .languageTag = tag("dflt"),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test with a non-default language. */
    {
        SFSchemeSetLanguageTag(&scheme, tag("LNG "));
        SFPatternRef pattern = SFSchemeBuildPattern(&scheme);

        SFTag expectedTags[] = { tag("srqi") };
        SFLookupInfo expectedLookups[] = { {SRQI, 1} };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 1 }, { 0, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 1 },
            .featureUnits = { expectedUnits, 1, 0 },
            .scriptTag = tag("test"),
            .languageTag = tag("LNG "),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test by trying to turn off required features. */
    /* Test by disabling a feature marked as on by default. */
    /* Test by enabling a feature marked as off by default. */
    /* Test by setting custom value of a required feature. */
    /* Test by setting custom value of a feature marked as on by default. */
    /* Test by setting custom value of a feature marked as off by default. */
    /* Test by enabling simultaneous features across multiple groups. */
    /* Test by enabling custom features of GSUB and GPOS. */
    /* Test by enabling a custom feature available in both GSUB and GPOS. */
    {
        SFTag featureTags[] = {
            tag("srqi"), tag("prq1"), tag("son1"), tag("sof1"), tag("prqi"),
            tag("pon1"), tag("pof1"), tag("scst"), tag("pcst"), tag("cust")
        };
        SFUInt16 featureValues[] = {
            0, 0, 0, 1, 2, 3, 4, 5, 6, 7
        };

        SFSchemeSetLanguageTag(&scheme, tag("dflt"));
        SFSchemeSetFeatureValues(&scheme, featureTags, featureValues, sizeof(featureValues) / sizeof(SFUInt16));

        SFPatternRef pattern = SFSchemeBuildPattern(&scheme);

        SFTag expectedTags[] = {
            tag("srqi"), tag("srq1"), tag("sof1"), tag("sim1"), tag("sim2"), tag("scst"), tag("cust"),
            tag("pof1"), tag("pon1"), tag("prq1"), tag("prqi"), tag("pcst")
        };
        SFLookupInfo expectedLookups[] = {
            {SRQI, 1}, {SOF1, 1}, {SRQ1, 1}, {SIM1, 1}, {SIM2, 1}, {CUST, 7}, {SCST, 5},
            {POF1, 4}, {PON1, 3}, {PRQ1, 1}, {PRQI, 2}, {PCST, 6}
        };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0],  1 }, { 0,  1 }, 0x00 },
            { { &expectedLookups[1],  2 }, { 1,  2 }, 0x00 },
            { { &expectedLookups[3],  2 }, { 3,  2 }, 0x00 },
            { { &expectedLookups[5],  2 }, { 5,  2 }, 0x00 },
            { { &expectedLookups[7],  3 }, { 7,  3 }, 0x00 },
            { { &expectedLookups[10], 1 }, { 10, 1 }, 0x00 },
            { { &expectedLookups[11], 1 }, { 11, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 12 },
            .featureUnits = { expectedUnits, 4, 3 },
            .scriptTag = tag("test"),
            .languageTag = tag("dflt"),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    SFSchemeFinalize(&scheme);
    SFFontRelease(font);
}

void SchemeTester::test()
{
    testFeatures();
    testBuild();
}
