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

#include "Common.h"
#include "OpenType.h"
#include "UnifiedEngine.h"
#include "SFBase.h"
#include "SFFont.h"
#include "SFPatternBuilder.h"
#include "SFPattern.h"
#include "SFScheme.h"

static void AddFeatureLookups(SFPatternBuilderRef patternBuilder, Data featureTable)
{
    SFUInt16 lookupCount = Feature_LookupCount(featureTable);
    SFUInt16 index;

    for (index = 0; index < lookupCount; index++) {
        SFUInt16 lookupListIndex = Feature_LookupListIndex(featureTable, index);
        SFPatternBuilderAddLookup(patternBuilder, lookupListIndex);
    }
}

static SFBoolean GetCustomValue(SFSchemeRef scheme, SFTag featureTag, SFUInt16 *featureValue)
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

static void AddFeatureUnit(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    Data langSysTable, Data featureListTable,
    FeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFBoolean exists = SFFalse;
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        FeatureInfoRef featureInfo = &featureInfos[index];
        SFTag featureTag = featureInfo->tag;
        FeatureStatus featureStatus = featureInfo->status;
        SFUInt16 featureValue;
        SFUInt16 customValue;

        featureValue = (featureStatus == OFF_BY_DEFAULT ? 0 : 1);

        if (GetCustomValue(scheme, featureTag, &customValue)) {
            /* Override the value if applicable. */
            if ((featureStatus != REQUIRED) || (featureStatus == REQUIRED && customValue != 0)) {
                featureValue = customValue;
            }
        }

        /* Process the feature if it is enabled. */
        if (featureValue != 0) {
            Data featureTable = SearchFeatureTable(langSysTable, featureListTable, featureTag);

            /* Add the feature if it exists in the language. */
            if (featureTable) {
                SFPatternBuilderAddFeature(patternBuilder, featureTag, featureValue, featureInfo->mask);
                AddFeatureLookups(patternBuilder, featureTable);

                exists = SFTrue;
            }
        }
    }

    if (exists) {
        SFPatternBuilderMakeFeatureUnit(patternBuilder);
    }
}

static void AddKnownFeatures(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    Data langSysTable, Data featureListTable,
    FeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFUInteger index = 0;

    while (index < featureCount) {
        FeatureInfoRef featureInfo = &featureInfos[index];
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

        AddFeatureUnit(scheme, patternBuilder, langSysTable, featureListTable, featureInfo, unitLength);
        index += unitLength;
    }
}

static SFBoolean IsKnownFeature(SFTag featureTag, FeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        if (featureInfos[index].tag == featureTag) {
            return SFTrue;
        }
    }

    return SFFalse;
}

static void AddCustomFeatures(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    Data langSysTable, Data featureListTable,
    FeatureInfo *featureInfos, SFUInteger featureCount)
{
    SFBoolean exists = SFFalse;
    SFUInteger index;

    for (index = 0; index < scheme->_featureCount; index++) {
        SFTag featureTag = scheme->_featureTags[index];

        /*
         * Make sure that the shaping engine does not recognize this feature and it is not already
         * added as a substitution feature.
         */
        if (!IsKnownFeature(featureTag, featureInfos, featureCount)
            && !SFPatternBuilderContainsFeature(patternBuilder, featureTag)) {
            SFUInt16 featureValue = scheme->_featureValues[index];

            /* Process the feature if it is enabled. */
            if (featureValue != 0) {
                Data featureTable = SearchFeatureTable(langSysTable, featureListTable, featureTag);

                /* Add the feature if it exists in the language. */
                if (featureTable) {
                    SFPatternBuilderAddFeature(patternBuilder, featureTag, featureValue, 0);
                    AddFeatureLookups(patternBuilder, featureTable);

                    exists = SFTrue;
                }
            }
        }
    }

    if (exists) {
        SFPatternBuilderMakeFeatureUnit(patternBuilder);
    }
}

static void AddHeaderTable(SFSchemeRef scheme, SFPatternBuilderRef patternBuilder,
    Data headerTable, FeatureInfo *featureInfos, SFUInteger featureCount)
{
    Data scriptListTable = Header_ScriptListTable(headerTable);
    Data featureListTable = Header_FeatureListTable(headerTable);
    Data scriptTable;
    Data langSysTable;

    /* Get script table belonging to the desired tag. */
    scriptTable = SearchScriptTable(scriptListTable, scheme->_scriptTag);

    if (scriptTable) {
        /* Get lang sys table belonging to the desired tag. */
        langSysTable = SearchLangSysTable(scriptTable, scheme->_languageTag);

        if (langSysTable) {
            AddKnownFeatures(scheme, patternBuilder, langSysTable, featureListTable, featureInfos, featureCount);
            AddCustomFeatures(scheme, patternBuilder, langSysTable, featureListTable, featureInfos, featureCount);
        }
    }
}

SF_INTERNAL void SFSchemeInitialize(SFSchemeRef scheme, ShapingKnowledgeRef shapingKnowledge)
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
    SFSchemeInitialize(scheme, &UnifiedKnowledgeInstance);

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
        ScriptKnowledgeRef knowledge = ShapingKnowledgeSeekScript(scheme->_knowledge, scheme->_scriptTag);
        SFPatternRef pattern = SFPatternCreate();
        SFPatternBuilder builder;

        SFPatternBuilderInitialize(&builder, pattern);
        SFPatternBuilderSetFont(&builder, scheme->_font);
        SFPatternBuilderSetScript(&builder, scheme->_scriptTag, knowledge->defaultDirection);
        SFPatternBuilderSetLanguage(&builder, scheme->_languageTag);

        if (font->resource->gsub) {
            SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);
            AddHeaderTable(scheme, &builder, font->resource->gsub, knowledge->substFeatures.items, knowledge->substFeatures.count);
            SFPatternBuilderEndFeatures(&builder);
        }

        if (font->resource->gpos) {
            SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);
            AddHeaderTable(scheme, &builder, font->resource->gpos, knowledge->posFeatures.items, knowledge->posFeatures.count);
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
