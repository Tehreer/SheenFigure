/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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
#include <SFFeature.h>
#include <SFLanguage.h>
#include <SFScript.h>
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SFAssert.h"
#include "SFScriptCache.h"
#include "SFScriptBuilder.h"

SF_INTERNAL void SFScriptBuilderInitialize(SFScriptBuilderRef builder, SFScriptCacheRef scriptCache)
{
    /* Script cache must not be 'NULL'. */
    SFAssert(builder->scriptCache != NULL);

    /* Initialize script cache. */
    scriptCache->scriptArray = NULL;
    scriptCache->scriptCount = 0;

    /* Initialize builder. */
    builder->scriptCache = scriptCache;
    builder->_currentScript = NULL;
    builder->_currentLanguage = NULL;
    builder->_currentFeature = NULL;
    builder->_currentlookup = NULL;
    builder->_scriptCapacity = 0;
    builder->_languageCapacity = 0;
    builder->_featureCapacity = 0;
    builder->_groupCapacity = 0;
    builder->_lookupCapacity = 0;
    builder->_groupCapacity = 0;
}

SF_INTERNAL void SFScriptBuilderReserveScripts(SFScriptBuilderRef builder, SFUInteger scriptCount)
{
    /* Scripts must be reserved only once. */
    SFAssert(builder->scriptCache->scriptArray == NULL);
    /* Script count must be non zero. */
    SFAssert(scriptCount != 0);

    builder->scriptCache->scriptArray = malloc(sizeof(SFScriptDetail) * scriptCount);
    builder->_scriptCapacity = scriptCount;
}

SF_INTERNAL void SFScriptBuilderAddScript(SFScriptBuilderRef builder, SFScript script)
{
    SFScriptCacheRef scriptCache = builder->scriptCache;
    SFScriptDetailRef currentScript;

    /* Script cache must have capacity for a new script. */
    SFAssert(scriptCache->scriptCount < builder->_scriptCapacity);

    /* Increment script count and initialize current script. */
    currentScript = &scriptCache->scriptArray[scriptCache->scriptCount++];
    currentScript->languageArray = NULL;
    currentScript->languageCount = 0;
    currentScript->script = script;

    /* Set current script in builder. */
    builder->_currentScript = currentScript;
}

SF_INTERNAL void SFScriptBuilderReserveLanguages(SFScriptBuilderRef builder, SFUInteger languageCount)
{
    SFScriptDetailRef currentScript = builder->_currentScript;
    SFUInteger actualCapacity;

    /* A script must be available before reserving languages. */
    SFAssert(currentScript != NULL);
    /* Languages must be reserved only once. */
    SFAssert(currentScript->languageArray == NULL);
    /* Language count must be non zero. */
    SFAssert(languageCount != 0);

    /* Include default language as well. */
    actualCapacity = languageCount + 1;

    currentScript->languageArray = malloc(sizeof(SFLanguageDetail) * actualCapacity);
    currentScript->defaultLanguage = NULL;
    builder->_languageCapacity = languageCount;
}

SF_INTERNAL void SFScriptBuilderAddLanguage(SFScriptBuilderRef builder, SFLanguage language)
{
    SFScriptDetailRef currentScript = builder->_currentScript;
    SFLanguageDetailRef currentLanguage;

    /* A script must be available before adding languages. */
    SFAssert(currentScript != NULL);
    /* Current script must have capacity for a new language. */
    SFAssert(currentScript->languageCount < builder->_languageCapacity);

    /* Get current language */
    if (language != SFLanguageDflt) {
        currentLanguage = &currentScript->languageArray[++currentScript->languageCount];
    } else {
        /* Default language will always be at first index. */
        currentLanguage = &currentScript->languageArray[0];
        currentScript->defaultLanguage = currentLanguage;
    }

    /* Initialize current language. */
    currentLanguage->requiredFeature.gsub = NULL;
    currentLanguage->requiredFeature.gpos = NULL;
    currentLanguage->featureArray = NULL;
    currentLanguage->groupArray = NULL;
    currentLanguage->featureCount.gsub = 0;
	currentLanguage->featureCount.gpos = 0;
    currentLanguage->groupCount = 0;
    currentLanguage->language = language;

    /* Set current language in builder. */
    builder->_currentLanguage = currentLanguage;
}

SF_INTERNAL void SFScriptBuilderReserveFeatures(SFScriptBuilderRef builder, SFUInteger featureCount)
{
    SFFeatureDetail *featureArray;
    SFUInteger index;

    /* A language must be available before reserving features. */
    SFAssert(builder->_currentLanguage != NULL);
    /* Features must be reserved only once for a language. */
    SFAssert(builder->_currentLanguage->featureArray == NULL);
    /* Feature count must be greater than zero. */
    SFAssert(featureCount > 0);

    /* Allocate feature array. */
    featureArray = malloc(sizeof(SFFeatureDetail) * featureCount);

    /* Mark each element uninitialized by setting its feature to zero. */
    for (index = 0; index < featureCount; index++) {
        featureArray[index].feature = 0;
    }

    /* Save array and its capacity. */
    builder->_currentLanguage->featureArray = featureArray;
    builder->_featureCapacity = featureCount;
}

SF_INTERNAL void SFScriptBuilderBeginFeatures(SFScriptBuilderRef builder, SFHeaderKind kind)
{

}

SF_INTERNAL void SFScriptBuilderReserveGroups(SFScriptBuilderRef builder, SFUInteger groupCount)
{
    /* A language must be available before reserving feature groups. */
    SFAssert(builder->_currentLanguage != NULL);
    /* Feature groups must be reserved only once. */
    SFAssert(builder->_currentLanguage->groupArray == NULL);
    /* Group count must be non zero. */
    SFAssert(groupCount != 0);

    builder->_currentLanguage->groupArray = malloc(sizeof(SFGroupDetail) * groupCount);
    builder->_groupCapacity = groupCount;
}

SF_INTERNAL void SFScriptBiulderAddGroup(SFScriptBuilderRef builder, SFUInteger featureIndex, SFUInteger featureCount)
{
    SFLanguageDetailRef currentLanguage = builder->_currentLanguage;
    SFGroupDetailRef currentGroup;

    /* A language must be available before adding groups. */
    SFAssert(currentLanguage != NULL);
    /* Current language must have the capcity for a new group. */
    SFAssert(currentLanguage->groupCount < builder->_groupCapacity);
    /* Features must be reserved before adding groups. */
    SFAssert(builder->_featureCapacity != 0);
    /* Feature index must fall within reserved capacity. */
    SFAssert(featureIndex < builder->_featureCapacity);
    /* Specified number of features must fall within reserved capacity. */
    SFAssert((featureIndex + featureCount) < builder->_featureCapacity);

    /* Increment group count and initialize current group. */
    currentGroup = &currentLanguage->groupArray[currentLanguage->groupCount++];
    currentGroup->lookupArray = NULL;
    currentGroup->featureIndex = featureIndex;
    currentGroup->featureCount = featureCount;
    currentGroup->lookupCount = 0;
}

SF_INTERNAL void SFScriptBuilderAddFeature(SFScriptBuilderRef builder, SFFeature feature, SFHeaderKind kind, SFUInteger orderIndex)
{
    SFLanguageDetailRef currentLanguage = builder->_currentLanguage;
    SFFeatureDetailRef currentFeature;

    /* A language must be available before adding features. */
    SFAssert(currentLanguage != NULL);
    /* Current language must have the capcity to hold the feature at specified index. */
    SFAssert(orderIndex < builder->_featureCapacity);
    /* The feature must be uninitialized at the specified index. */
    SFAssert(currentLanguage->featureArray[orderIndex].feature == 0);

    /* Increment feature count. */
    currentLanguage->featureCount++;

    /* Initialize current feature. */
    currentFeature = &currentLanguage->featureArray[orderIndex];
    currentFeature->lookupArray = NULL;
    currentFeature->lookupCount = 0;
    currentFeature->feature = feature;
    currentFeature->headerKind = kind;

    /* Set current feature in builder. */
    builder->_currentFeature = currentFeature;
}

SF_INTERNAL void SFScriptBuilderReserveLookups(SFScriptBuilderRef builder, SFUInteger lookupCount)
{
    SFLookupDetail *lookupArray;
    SFUInteger index;

    /* A feature must be available before reserving lookups. */
    SFAssert(builder->_currentFeature != NULL);
    /* Lookups must be reserved only once. */
    SFAssert(builder->_currentFeature->lookupArray == NULL);
    /* Lookup count must be non zero. */
    SFAssert(lookupCount != 0);

    /* Allocate lookup array. */
    lookupArray = malloc(sizeof(SFLookupDetail) * lookupCount);

    /* Mark each element uninitialized by setting its table to 'NULL'. */
    for (index = 0; index < lookupCount; index++) {
        lookupArray[index].table = NULL;
    }

    /* Save array and its capacity. */
    builder->_currentFeature->lookupArray = lookupArray;
    builder->_lookupCapacity = lookupCount;
}

SF_INTERNAL void SFScriptBuilderAddLookup(SFScriptBuilderRef builder, SFData lookupTable, SFUInteger orderIndex)
{
    SFFeatureDetailRef currentFeature = builder->_currentFeature;
    SFLookupDetailRef currentLookup;

    /* A feature must be available before adding lookups. */
    SFAssert(currentFeature != NULL);
    /* Current feature must have capcity to hold the lookup at specified index. */
    SFAssert(orderIndex < builder->_lookupCapacity);
    /* The lookup must be uninitialized at the specified index. */
    SFAssert(currentFeature->lookupArray[orderIndex].table == NULL);

    /* Increment lookup count. */
    currentFeature->lookupCount++;

    /* Initialize current lookup. */
    currentLookup = &currentFeature->lookupArray[orderIndex];
    currentLookup->table = lookupTable;
}

SF_INTERNAL void SFScriptBuilderCloseFeature(SFScriptBuilderRef builder)
{
    SFFeatureDetailRef feature = builder->_currentFeature;
    SFLookupDetail *lookupArray = feature->lookupArray;
    SFUInteger capacity = builder->_lookupCapacity;
    SFUInteger shiftIndex = SFInvalidIndex;
    SFUInteger index;

    /* A feature must be available before it can be closed. */
    SFAssert(feature != NULL);

    /* Fill uninitialized elements by shifting initialized ones to the left. */
    for (index = 0; index < capacity; index++) {
        if (shiftIndex != SFInvalidIndex) {
            if (lookupArray[index].table != NULL) {
                lookupArray[shiftIndex++] = lookupArray[index];
            }
        } else if (lookupArray[index].table == NULL) {
            shiftIndex = index;
        }
    }

    if (feature->lookupCount) {
        /* Shrink lookup array to actual size. */
        feature->lookupArray = realloc(lookupArray, sizeof(SFLookupDetail) * feature->lookupCount);
    } else {
        /* If feature contains no lookup, cleanup. */
        feature->lookupArray = NULL;
        free(lookupArray);
    }

    /* Dismiss current feature from builder. */
    builder->_currentFeature = NULL;
    builder->_lookupCapacity = 0;
}

static void _SFBuildGroupLookups(SFGroupDetailRef group, SFUInteger lookupCapacity)
{
    const SFUInteger lookupsSize = sizeof(SFLookupDetail) * lookupCapacity;
    const SFUInteger positionsSize = sizeof(SFUInteger) * lookupCapacity;
    SFUInteger previousIndex = SFInvalidIndex;
    SFUInteger lookupCount = 0;
    SFUInt8 *memory;
    SFLookupDetail *lookups;
    SFUInteger *positions;

    /* Allocate memory for lookups as well as positions. */
    memory = malloc(lookupsSize + positionsSize);
    lookups = (SFLookupDetail *)memory;
    positions = (SFUInteger *)(memory + lookupsSize);

    /* Merge lookup arrays of all features without duplicates. */
    for (;;) {
        SFLookupDetail minLookup = { SFInvalidIndex };
        SFUInteger featureIndex = SFInvalidIndex;
        SFUInteger limit = group->featureIndex + group->featureCount;
        SFUInteger start;

        /* Go through current positions and get the minimum. */
        for (start = group->featureIndex; start < group->featureCount; start++) {
            //SFFeatureDetailRef feature = &group->featureArray[i];
            //SFUInteger position = positions[i];

            //if (position < feature->lookupCount && feature->lookupArray[position].index < minLookup.index) {
                /* Save the minimum lookup and its respective feature's index. */
            //    minLookup = feature->lookupArray[position];
            //    featureIndex = i;
            //}
        }

        /* If there is no minimum, the index will be invalid. */
        if (featureIndex == SFInvalidIndex) {
            break;
        }

        /* Ignore if duplicate. */
        //if (previousIndex != minLookup.index) {
        //    lookups[lookupCount++] = minLookup;
        //    previousIndex = minLookup.index;
        //}

        positions[featureIndex]++;
    }

    /*
     * Actual number of lookups must be less than or equal to the provided capacity. Otherwise,
     * memory will be corrupted.
     */
    SFAssert(lookupCount <= lookupCapacity);

    /* Shrink lookup array to actual size. */
    lookups = realloc(lookups, sizeof(SFLookupDetail) * lookupCount);

    /* Save lookup array and its size in the group. */
    group->lookupArray = lookups;
    group->lookupCount = lookupCount;
}

static void _SFBuildLanguageGroups(SFLanguageDetailRef language)
{
    SFUInteger i;

    for (i = 0; i < language->groupCount; i++) {
        SFFeatureDetail *featureArray = language->featureArray;
        SFGroupDetailRef group = &language->groupArray[i];
        SFUInteger lookupCapacity = 0;
        SFUInteger j;

        /* Estimate number of lookups in the group. */
        for (j = 0; j < group->featureCount; j++) {
            lookupCapacity += featureArray[j].lookupCount;
        }

        /* Build lookup array of the group. */
        _SFBuildGroupLookups(group, lookupCapacity);
    }
}

SF_INTERNAL void SFScriptBuilderCloseLanguage(SFScriptBuilderRef builder)
{
    SFLanguageDetailRef language = builder->_currentLanguage;
    SFFeatureDetail *featureArray = language->featureArray;
    SFUInteger capacity = builder->_featureCapacity;
    SFUInteger shiftIndex = SFInvalidIndex;
    SFUInteger index;

    /* A language must be available before it can be closed. */
    SFAssert(language != NULL);

    /* Fill uninitialized elements by shifting initialized ones to the left. */
    for (index = 0; index < capacity; index++) {
        if (shiftIndex != SFInvalidIndex) {
            if (featureArray[index].feature) {
                featureArray[shiftIndex++] = featureArray[index];
            }
        } else if (!featureArray[index].feature) {
            shiftIndex = index;
        }
    }

    if (language->featureCount) {
        /* Shrink feature array to actual size. */
        language->featureArray = realloc(featureArray, sizeof(SFFeatureDetail) * language->featureCount);
        /* Build language groups. */
        _SFBuildLanguageGroups(language);
    } else {
        /* If language contains no feature, cleanup. */
        language->featureArray = NULL;
        free(featureArray);
    }

    /* Dismiss current language from builder. */
    builder->_currentLanguage = NULL;
    builder->_featureCapacity = 0;
    builder->_groupCapacity = 0;
}

SF_INTERNAL void SFScriptBuilderCloseScript(SFScriptBuilderRef builder)
{
    SFScriptDetailRef script = builder->_currentScript;
    SFLanguageDetail *languages = script->languageArray;

    /* A script must be available before it can be closed. */
    SFAssert(script != NULL);

    if (script->languageCount) {
        /* Shrink language array to actual size. */
        script->languageArray = realloc(languages, sizeof(SFFeatureDetail) * script->languageCount);
    } else {
        /* If script contains no language, cleanup. */
        script->languageArray = NULL;
        free(languages);
    }

    /* Dismiss current script from builder. */
    builder->_currentScript = NULL;
    builder->_languageCapacity = 0;
}

SF_INTERNAL void SFScriptBuilderInvalidate(SFScriptBuilderRef builder)
{
}
