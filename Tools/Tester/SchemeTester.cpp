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

#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "SchemeTester.h"

extern "C" {
#include <SheenFigure/Source/SFAssert.h>
#include <SheenFigure/Source/SFPattern.h>
#include <SheenFigure/Source/SFScheme.h>
}

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
        writer.writeTable(&gsub);

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
    SFSchemeSetLanguageTag(scheme, SFTagMake('d', 'f', 'l', 't'));

    SFPatternRef pattern1 = SFSchemeBuildPattern(scheme);
    SFAssert(pattern1->font == font);
    SFAssert(pattern1->featureTags.items != NULL);
    SFAssert(pattern1->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
    SFAssert(pattern1->featureTags.count == 1);
    SFAssert(pattern1->featureUnits.items != NULL);
    SFAssert(pattern1->featureUnits.items[0].lookupIndexes.items != NULL);
    SFAssert(pattern1->featureUnits.items[0].lookupIndexes.items[0] == 0);
    SFAssert(pattern1->featureUnits.items[0].lookupIndexes.count == 1);
    SFAssert(pattern1->featureUnits.items[0].coveredRange.start == 0);
    SFAssert(pattern1->featureUnits.items[0].coveredRange.length == 1);
    SFAssert(pattern1->featureUnits.gsub == 1);
    SFAssert(pattern1->featureUnits.gpos == 0);
    SFAssert(pattern1->scriptTag == SFTagMake('l', 'a', 't', 'n'));
    SFAssert(pattern1->languageTag == SFTagMake('d', 'f', 'l', 't'));
    SFPatternRelease(pattern1);

    SFSchemeSetLanguageTag(scheme, SFTagMake('E', 'N', 'G', ' '));

    SFPatternRef pattern2 = SFSchemeBuildPattern(scheme);
    SFAssert(pattern2->font == font);
    SFAssert(pattern2->featureTags.items != NULL);
    SFAssert(pattern2->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
    SFAssert(pattern2->featureTags.count == 1);
    SFAssert(pattern2->featureUnits.items != NULL);
    SFAssert(pattern2->featureUnits.items[0].lookupIndexes.items != NULL);
    SFAssert(pattern2->featureUnits.items[0].lookupIndexes.items[0] == 0);
    SFAssert(pattern2->featureUnits.items[0].lookupIndexes.count == 1);
    SFAssert(pattern2->featureUnits.items[0].coveredRange.start == 0);
    SFAssert(pattern2->featureUnits.items[0].coveredRange.length == 1);
    SFAssert(pattern2->featureUnits.gsub == 1);
    SFAssert(pattern2->featureUnits.gpos == 0);
    SFAssert(pattern2->scriptTag == SFTagMake('l', 'a', 't', 'n'));
    SFAssert(pattern2->languageTag == SFTagMake('E', 'N', 'G', ' '));
    SFPatternRelease(pattern2);

    SFSchemeRelease(scheme);
    SFFontRelease(font);
}
