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
#include <cstring>

extern "C" {
#include <Source/SFAssert.h>
#include <Source/SFPattern.h>
#include <Source/SFScheme.h>
}

#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "Utilities/SFPattern+Testing.h"
#include "SchemeTester.h"

using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

static void loadTable(void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length)
{
    if (tag == SFTagMake('G', 'S', 'U', 'B')) {
        UInt16 lookupIndex[1];
        lookupIndex[0] = 0;

        FeatureTable kernFeature;
        kernFeature.featureParams = 0;
        kernFeature.lookupCount = 1;
        kernFeature.lookupListIndex = lookupIndex;

        FeatureRecord featureRecord[1];
        memcpy(&featureRecord[0].featureTag, "ccmp", 4);
        featureRecord[0].feature = &kernFeature;

        FeatureListTable featureList;
        featureList.featureCount = 1;
        featureList.featureRecord = featureRecord;

        UInt16 engFeatureIndex[1];
        engFeatureIndex[0] = 0;

        LangSysTable engLangSys;
        engLangSys.lookupOrder = 0;
        engLangSys.reqFeatureIndex = 0xFFFF;
        engLangSys.featureCount = 1;
        engLangSys.featureIndex = engFeatureIndex;

        LangSysRecord langSysRecord[1];
        memcpy(&langSysRecord[0].langSysTag, "ENG ", 4);
        langSysRecord[0].langSys = &engLangSys;

        UInt16 dfltFeatureIndex[1];
        dfltFeatureIndex[0] = 0;

        LangSysTable dfltLangSys;
        dfltLangSys.lookupOrder = 0;
        dfltLangSys.reqFeatureIndex = 0xFFFF;
        dfltLangSys.featureCount = 1;
        dfltLangSys.featureIndex = dfltFeatureIndex;

        ScriptTable latnScript;
        latnScript.defaultLangSys = &dfltLangSys;
        latnScript.langSysCount = 1;
        latnScript.langSysRecord = langSysRecord;

        ScriptRecord scripts[1];
        memcpy(&scripts[0].scriptTag, "latn", 4);
        scripts[0].script = &latnScript;

        ScriptListTable scriptList;
        scriptList.scriptCount = 1;
        scriptList.scriptRecord = scripts;

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

void SchemeTester::test()
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
    SFSchemeSetScriptTag(scheme, SFTagMake('l', 'a', 't', 'n'));

    /* Test with default language. */
    {
        SFSchemeSetLanguageTag(scheme, SFTagMake('d', 'f', 'l', 't'));
        SFPatternRef pattern = SFSchemeBuildPattern(scheme);

        SFTag expectedTags[] = { SFTagMake('c', 'c', 'm', 'p') };
        SFUInt16 expectedLookups[] = { 0 };
        SFFeatureUnit expectedUnits[] = {
            {
                .lookupIndexes = { expectedLookups, 1 },
                .coveredRange = { 0, 1 },
                .featureMask = 0x00,
            }
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 1 },
            .featureUnits = { expectedUnits, 1, 0 },
            .scriptTag = SFTagMake('l', 'a', 't', 'n'),
            .languageTag = SFTagMake('d', 'f', 'l', 't'),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        SFAssert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test with a non-default language. */
    {
        SFSchemeSetLanguageTag(scheme, SFTagMake('E', 'N', 'G', ' '));
        SFPatternRef pattern = SFSchemeBuildPattern(scheme);

        SFTag expectedTags[] = { SFTagMake('c', 'c', 'm', 'p') };
        SFUInt16 expectedLookups[] = { 0 };
        SFFeatureUnit expectedUnits[] = {
            {
                .lookupIndexes = { expectedLookups, 1 },
                .coveredRange = { 0, 1 },
                .featureMask = 0x00,
            }
        };
        SFPattern expectedPattern = {
            .font = font,
            .featureTags = { expectedTags, 1 },
            .featureUnits = { expectedUnits, 1, 0 },
            .scriptTag = SFTagMake('l', 'a', 't', 'n'),
            .languageTag = SFTagMake('E', 'N', 'G', ' '),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        SFAssert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    SFSchemeRelease(scheme);
    SFFontRelease(font);
}
