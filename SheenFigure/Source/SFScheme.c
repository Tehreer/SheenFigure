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

#include <SFConfig.h>
#include <SFLanguageTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SFCommon.h"
#include "SFPatternBuilder.h"
#include "SFPattern.h"
#include "SFShapingEngine.h"
#include "SFUnifiedEngine.h"
#include "SFScheme.h"

static SFData _SFSearchScriptInList(SFData scriptList, SFScriptTag scriptTag)
{
    SFUInt16 scriptCount = SFScriptList_ScriptCount(scriptList);
    SFUInt16 index;

    for (index = 0; index < scriptCount; index++) {
        SFData scriptRecord = SFScriptList_ScriptRecord(scriptList, index);
        SFTag scriptRecordTag = SFScriptRecord_ScriptTag(scriptRecord);

        if (scriptRecordTag == scriptTag) {
            SFOffset offset = SFScriptRecord_ScriptOffset(scriptRecord);
            return SFData_Subdata(scriptList, offset);
        }
    }

    return NULL;
}

static SFData _SFSearchLangSysInScript(SFData script, SFLanguageTag languageTag)
{
    if (languageTag == SFLanguageTagDFLT) {
        SFOffset offset = SFScript_DefaultLangSysOffset(script);
        if (offset) {
            return SFData_Subdata(script, offset);
        }
    } else {
        SFUInt16 langSysCount = SFScript_LangSysCount(script);
        SFUInt16 index;

        for (index = 0; index < langSysCount; index++) {
            SFData langSysRecord = SFScript_LangSysRecord(script, index);
            SFTag langSysTag = SFLangSysRecord_LangSysTag(langSysRecord);
            SFUInt16 offset;

            if (langSysTag == languageTag) {
                offset = SFLangSysRecord_LangSysOffset(langSysRecord);
                return SFData_Subdata(script, offset);
            }
        }
    }

    return NULL;
}

static SFData _SFSearchFeatureInLangSys(SFData langSys, SFData featureList, SFFeatureTag featureTag)
{
    SFUInt16 featureCount = SFLangSys_FeatureCount(langSys);
    SFUInt16 index;

    for (index = 0; index < featureCount; index++) {
        SFUInt16 featureIndex = SFLangSys_FeatureIndex(langSys, index);
        SFData featureRecord = SFFeatureList_FeatureRecord(featureList, featureIndex);
        SFTag featureRecordTag = SFFeatureRecord_FeatureTag(featureRecord);

        if (featureRecordTag == featureTag) {
            SFOffset offset = SFFeatureRecord_FeatureOffset(featureRecord);
            return SFData_Subdata(featureList, offset);
        }
    }

    return NULL;
}

static void _SFAddLookups(_SFSchemeStateRef state, SFData feature)
{
    SFUInt16 lookupCount = SFFeature_LookupCount(feature);
    SFUInt16 lookupIndex;

    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFUInt16 lookupListIndex = SFFeature_LookupListIndex(feature, lookupIndex);
        SFPatternBuilderAddLookup(&state->builder, lookupListIndex);
    }
}

static void _SFAddFeatureGroup(_SFSchemeStateRef state, SFUInteger index, SFUInteger count, SFBoolean simultaneous)
{
    SFUInteger limit = index + count;

    for (; index < limit; index++) {
        SFFeatureTag featureTag = state->knowledge->featureTags.array[index];
        SFData feature = _SFSearchFeatureInLangSys(state->langSys, state->featureList, featureTag);

        if (feature) {
            SFPatternBuilderAddFeature(&state->builder, featureTag);
            _SFAddLookups(state, feature);

            if (!simultaneous) {
                SFPatternBuilderMakeGroup(&state->builder);
            }
        }
    }

    if (simultaneous) {
        SFPatternBuilderMakeGroup(&state->builder);
    }
}

static void _SFAddFeatures(_SFSchemeStateRef state)
{
    SFUInteger featureCount = state->knowledge->featureTags.count;
    SFUInteger groupCount = state->knowledge->featureGroups.count;
    SFUInteger featureIndex = 0;
    SFUInteger groupIndex;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++) {
        SFRange groupRange = state->knowledge->featureGroups.array[groupIndex];

        if (groupRange.start > featureIndex) {
            _SFAddFeatureGroup(state, featureIndex, groupRange.start - featureIndex, SFFalse);
            featureIndex = groupRange.start;
        } else {
            _SFAddFeatureGroup(state, groupRange.start, groupRange.length, SFTrue);
            featureIndex += groupRange.length;
        }
    }

    _SFAddFeatureGroup(state, featureIndex, featureCount - featureIndex, SFFalse);
}

static void _SFAddHeader(_SFSchemeStateRef state, SFData header)
{
    SFSchemeRef scheme = state->scheme;
    SFOffset offset;

    /* Get script list table. */
    offset = SFHeader_ScriptListOffset(header);
    state->scriptList = SFData_Subdata(header, offset);
    /* Get feature list table. */
    offset = SFHeader_FeatureListOffset(header);
    state->featureList = SFData_Subdata(header, offset);

    /* Get script table belonging to desired tag. */
    state->script = _SFSearchScriptInList(state->scriptList, scheme->_scriptTag);

    if (state->script) {
        /* Get lang sys table belonging to desired tag. */
        state->langSys = _SFSearchLangSysInScript(state->script, scheme->_languageTag);

        if (state->langSys) {
            _SFAddFeatures(state);
        }
    }
}

SFSchemeRef SFSchemeCreate(void)
{
    SFSchemeRef scheme = malloc(sizeof(SFScheme));
    scheme->_font = NULL;
    scheme->_scriptTag = 0;
    scheme->_languageTag = 0;
    scheme->_retainCount = 1;

    return scheme;
}

void SFSchemeSetFont(SFSchemeRef scheme, SFFontRef font)
{
    scheme->_font = font;
}

void SFSchemeSetScript(SFSchemeRef scheme, SFScriptTag scriptTag)
{
    scheme->_scriptTag = scriptTag;
}

void SFSchemeSetLanguage(SFSchemeRef scheme, SFLanguageTag languageTag)
{
    scheme->_languageTag = languageTag;
}

SFPatternRef SFSchemeBuildPattern(SFSchemeRef scheme)
{
    _SFSchemeState state;
    state.scheme = scheme;
    state.knowledge = SFShapingKnowledgeSeekScript(&SFUnifiedKnowledgeInstance, scheme->_scriptTag);

    /* Check whether Sheen Figure has knowledge about this script. */
    if (state.knowledge) {
        SFData gsub = scheme->_font->cache.gsub;
        SFData gpos = scheme->_font->cache.gpos;

        if (gsub || gpos) {
            SFPatternRef pattern = SFPatternCreate();

            SFPatternBuilderInitialize(&state.builder);
            SFPatternBuilderSetFont(&state.builder, scheme->_font);
            SFPatternBuilderSetScript(&state.builder, scheme->_scriptTag);
            SFPatternBuilderSetLanguage(&state.builder, scheme->_languageTag);

            if (gsub) {
                SFPatternBuilderBeginHeader(&state.builder, SFHeaderKindGSUB);
                _SFAddHeader(&state, gsub);
            }
            if (gpos) {
                SFPatternBuilderBeginHeader(&state.builder, SFHeaderKindGPOS);
                _SFAddHeader(&state, gpos);
            }

            SFPatternBuilderBuild(&state.builder, pattern);

            return pattern;
        }
    }

    return NULL;
}

SFSchemeRef SFSchemeRetain(SFSchemeRef scheme)
{
    if (scheme) {
        scheme->_retainCount++;
    }

    return scheme;
}

void SFSchemeRelease(SFSchemeRef scheme)
{
    if (scheme && --scheme->_retainCount == 0) {
        free(scheme);
    }
}
