/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SFBase.h"
#include "SFCommon.h"
#include "SFFont.h"
#include "SFPatternBuilder.h"
#include "SFPattern.h"
#include "SFUnifiedEngine.h"
#include "SFScheme.h"

static SFData _SFSearchScriptTable(SFData scriptListTable, SFTag scriptTag)
{
    SFData scriptTable = NULL;
    SFUInt16 scriptCount;
    SFUInt16 index;

    scriptCount = SFScriptList_ScriptCount(scriptListTable);

    for (index = 0; index < scriptCount; index++) {
        SFData scriptRecord = SFScriptList_ScriptRecord(scriptListTable, index);
        SFTag scriptRecTag = SFScriptRecord_ScriptTag(scriptRecord);
        SFOffset scriptOffset;

        if (scriptRecTag == scriptTag) {
            scriptOffset = SFScriptRecord_ScriptOffset(scriptRecord);
            scriptTable = SFData_Subdata(scriptListTable, scriptOffset);
            break;
        }
    }

    return scriptTable;
}

static SFData _SFSearchLangSysTable(SFData scriptTable, SFTag languageTag)
{
    SFData langSysTable = NULL;

    if (languageTag == SFTagMake('d', 'f', 'l', 't')) {
        SFOffset langSysOffset = SFScript_DefaultLangSysOffset(scriptTable);

        if (langSysOffset) {
            langSysTable = SFData_Subdata(scriptTable, langSysOffset);
        }
    } else {
        SFUInt16 langSysCount = SFScript_LangSysCount(scriptTable);
        SFUInt16 index;

        for (index = 0; index < langSysCount; index++) {
            SFData langSysRecord = SFScript_LangSysRecord(scriptTable, index);
            SFTag langSysTag = SFLangSysRecord_LangSysTag(langSysRecord);
            SFOffset langSysOffset;

            if (langSysTag == languageTag) {
                langSysOffset = SFLangSysRecord_LangSysOffset(langSysRecord);
                langSysTable = SFData_Subdata(scriptTable, langSysOffset);
                break;
            }
        }
    }

    return langSysTable;
}

static SFData _SFSearchFeatureTable(SFData langSysTable, SFData featureListTable, SFTag featureTag)
{
    SFData featureTable = NULL;
    SFUInt16 featureCount;
    SFUInt16 index;

    featureCount = SFLangSys_FeatureCount(langSysTable);

    for (index = 0; index < featureCount; index++) {
        SFUInt16 featureIndex = SFLangSys_FeatureIndex(langSysTable, index);
        SFData featureRecord = SFFeatureList_FeatureRecord(featureListTable, featureIndex);
        SFTag featureRecTag = SFFeatureRecord_FeatureTag(featureRecord);
        SFOffset featureOffset;

        if (featureRecTag == featureTag) {
            featureOffset = SFFeatureRecord_FeatureOffset(featureRecord);
            featureTable = SFData_Subdata(featureListTable, featureOffset);
            break;
        }
    }

    return featureTable;
}

static void _SFAddFeatureLookups(SFPatternBuilderRef patternBuilder, SFData featureTable)
{
    SFUInt16 lookupCount = SFFeature_LookupCount(featureTable);
    SFUInt16 index;

    for (index = 0; index < lookupCount; index++) {
        SFUInt16 lookupListIndex = SFFeature_LookupListIndex(featureTable, index);
        SFPatternBuilderAddLookup(patternBuilder, lookupListIndex);
    }
}

static SFBoolean _SFGetCustomValue(SFSchemeRef scheme, SFTag featureTag, SFUInt16 *featureValue)
{
    SFTag *featureTags = scheme->_featureTags;
    SFUInt16 *featureValues = scheme->_featureValues;
    SFUInteger featureCount = scheme->_featureCount;
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        if (featureTags[index] == featureTag) {
            *featureValue = featureValues[index];
            return SFTrue;
        }
    }

    return SFFalse;
}

static void _SFAddFeatureUnit(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    SFData langSysTable, SFData featureListTable,
    SFFeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFBoolean exists = SFFalse;
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        SFFeatureInfoRef featureInfo = &featureInfos[index];
        SFTag featureTag = featureInfo->tag;
        SFFeatureStatus featureStatus = featureInfo->status;
        SFUInt16 featureValue;
        SFUInt16 customValue;

        featureValue = (featureStatus == OFF_BY_DEFAULT ? 0 : 1);

        if (_SFGetCustomValue(scheme, featureTag, &customValue)) {
            /* Override the value if applicable. */
            if ((featureStatus != REQUIRED) || (featureStatus == REQUIRED && customValue != 0)) {
                featureValue = customValue;
            }
        }

        /* Process the feature if it is enabled. */
        if (featureValue != 0) {
            SFData featureTable = _SFSearchFeatureTable(langSysTable, featureListTable, featureTag);

            /* Add the feature if it exists in the language. */
            if (featureTable) {
                SFPatternBuilderAddFeature(patternBuilder, featureTag, featureValue, featureInfo->mask);
                _SFAddFeatureLookups(patternBuilder, featureTable);

                exists = SFTrue;
            }
        }
    }

    if (exists) {
        SFPatternBuilderMakeFeatureUnit(patternBuilder);
    }
}

static void _SFAddKnownFeatures(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    SFData langSysTable, SFData featureListTable,
    SFFeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFUInteger index = 0;

    while (index < featureCount) {
        SFFeatureInfoRef featureInfo = &featureInfos[index];
        SFUInteger unitLength = 1;

        if (featureInfo->execution == SIMULTANEOUS) {
            SFUInt8 group = featureInfo->group;
            SFUInteger next;

            /* Find out the number of features that need to be applied simultaneously. */
            for (next = index + 1; next < featureCount; next++) {
                if (featureInfos[next].group != group
                    || featureInfos[next].execution != SIMULTANEOUS)
                {
                    break;
                }
            }

            unitLength = next - index;
        }

        _SFAddFeatureUnit(scheme, patternBuilder, langSysTable, featureListTable, featureInfo, unitLength);
        index += unitLength;
    }
}

static SFBoolean _SFIsKnownFeature(SFTag featureTag, SFFeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        if (featureInfos[index].tag == featureTag) {
            return SFTrue;
        }
    }

    return SFFalse;
}

static void _SFAddCustomFeatures(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    SFData langSysTable, SFData featureListTable,
    SFFeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFBoolean exists = SFFalse;
    SFUInteger index;

    for (index = 0; index < scheme->_featureCount; index++) {
        SFTag featureTag = scheme->_featureTags[index];

        /*
         * Make sure that the shaping engine does not recognize this feature and it is not already
         * added as a substitution feature.
         */
        if (!_SFIsKnownFeature(featureTag, featureInfos, featureCount)
            && !SFPatternBuilderContainsFeature(patternBuilder, featureTag)) {
            SFUInt16 featureValue = scheme->_featureValues[index];

            /* Process the feature if it is enabled. */
            if (featureValue != 0) {
                SFData featureTable = _SFSearchFeatureTable(langSysTable, featureListTable, featureTag);

                /* Add the feature if it exists in the language. */
                if (featureTable) {
                    SFPatternBuilderAddFeature(patternBuilder, featureTag, featureValue, 0);
                    _SFAddFeatureLookups(patternBuilder, featureTable);

                    exists = SFTrue;
                }
            }
        }
    }

    if (exists) {
        SFPatternBuilderMakeFeatureUnit(patternBuilder);
    }
}

static void _SFAddHeaderTable(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    SFData headerTable, SFFeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFData scriptListTable = SFHeader_ScriptListTable(headerTable);
    SFData featureListTable = SFHeader_FeatureListTable(headerTable);
    SFData scriptTable;
    SFData langSysTable;

    /* Get script table belonging to the desired tag. */
    scriptTable = _SFSearchScriptTable(scriptListTable, scheme->_scriptTag);

    if (scriptTable) {
        /* Get lang sys table belonging to the desired tag. */
        langSysTable = _SFSearchLangSysTable(scriptTable, scheme->_languageTag);

        if (langSysTable) {
            _SFAddKnownFeatures(scheme, patternBuilder, langSysTable, featureListTable, featureInfos, featureCount);
            _SFAddCustomFeatures(scheme, patternBuilder, langSysTable, featureListTable, featureInfos, featureCount);
        }
    }
}

SF_INTERNAL void SFSchemeInitialize(SFSchemeRef scheme, SFShapingKnowledgeRef shapingKnowledge)
{
    scheme->_knowledge = shapingKnowledge;
    scheme->_font = NULL;
    scheme->_scriptTag = 0;
    scheme->_languageTag = 0;
    scheme->_featureTags = NULL;
    scheme->_featureValues = NULL;
    scheme->_featureCount = 0;
    scheme->_retainCount = 1;
}

SF_INTERNAL void SFSchemeFinalize(SFSchemeRef scheme)
{
    free(scheme->_featureTags);
    free(scheme->_featureValues);
}

SFSchemeRef SFSchemeCreate(void)
{
    SFSchemeRef scheme = malloc(sizeof(SFScheme));
    SFSchemeInitialize(scheme, &SFUnifiedKnowledgeInstance);

    return scheme;
}

void SFSchemeSetFont(SFSchemeRef scheme, SFFontRef font)
{
    scheme->_font = font;
}

void SFSchemeSetScriptTag(SFSchemeRef scheme, SFTag scriptTag)
{
    scheme->_scriptTag = scriptTag;
}

void SFSchemeSetLanguageTag(SFSchemeRef scheme, SFTag languageTag)
{
    scheme->_languageTag = languageTag;
}

void SFSchemeSetFeatureValues(SFSchemeRef scheme,
    SFTag *featureTags, SFUInt16 *featureValues, SFUInteger featureCount)
{
    SFTag *uniqueTags = realloc(scheme->_featureTags, sizeof(SFTag) * featureCount);
    SFUInt16 *latestValues = realloc(scheme->_featureValues, sizeof(SFUInt16) * featureCount);
    SFUInteger uniqueCount = 0;
    SFUInteger featureIndex;

    /* Copy the tags and values without duplicates. */
    for (featureIndex = 0; featureIndex < featureCount; featureIndex++) {
        SFTag currentTag = featureTags[featureIndex];
        SFUInt16 currentValue = featureValues[featureIndex];
        SFUInteger uniqueIndex;

        for (uniqueIndex = 0; uniqueIndex < uniqueCount; uniqueIndex++) {
            if (uniqueTags[uniqueIndex] == currentTag) {
                latestValues[uniqueIndex] = currentValue;
                break;
            }
        }

        if (uniqueIndex == uniqueCount) {
            uniqueTags[uniqueIndex] = currentTag;
            latestValues[uniqueIndex] = currentValue;
            uniqueCount += 1;
        }
    }

    scheme->_featureTags = uniqueTags;
    scheme->_featureValues = latestValues;
    scheme->_featureCount = uniqueCount;
}

SFPatternRef SFSchemeBuildPattern(SFSchemeRef scheme)
{
    SFFontRef font = scheme->_font;

    if (font) {
        SFScriptKnowledgeRef knowledge = SFShapingKnowledgeSeekScript(scheme->_knowledge, scheme->_scriptTag);
        SFPatternRef pattern = SFPatternCreate();
        SFPatternBuilder builder;

        SFPatternBuilderInitialize(&builder, pattern);
        SFPatternBuilderSetFont(&builder, scheme->_font);
        SFPatternBuilderSetScript(&builder, scheme->_scriptTag, knowledge->defaultDirection);
        SFPatternBuilderSetLanguage(&builder, scheme->_languageTag);

        if (font->tables.gsub) {
            SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);
            _SFAddHeaderTable(scheme, &builder, font->tables.gsub, knowledge->substFeatures.items, knowledge->substFeatures.count);
            SFPatternBuilderEndFeatures(&builder);
        }

        if (font->tables.gpos) {
            SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);
            _SFAddHeaderTable(scheme, &builder, font->tables.gpos, knowledge->posFeatures.items, knowledge->posFeatures.count);
            SFPatternBuilderEndFeatures(&builder);
        }

        SFPatternBuilderBuild(&builder);
        SFPatternBuilderFinalize(&builder);

        return pattern;
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
        SFSchemeFinalize(scheme);
        free(scheme);
    }
}
