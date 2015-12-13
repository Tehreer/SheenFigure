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
#include "SFPattern.h"
#include "SFPatternBuilder.h"

SF_INTERNAL void SFPatternBuilderInitialize(SFPatternBuilderRef builder, SFPatternRef pattern)
{
    /* Initialize builder. */
    builder->_currentGroup = NULL;
    builder->_currentHeader = 0;
    builder->_gsubGroupCount = 0;
    builder->_gposGroupCount = 0;
    builder->_featureIndex = 0;

    SFListInitialize(&builder->_featureTags, sizeof(SFFeature));
    SFListSetCapacity(&builder->_featureTags, 0, 24);

    SFListInitialize(&builder->_featureGroups, sizeof(SFFeatureGroup));
    SFListSetCapacity(&builder->_featureGroups, 0, 24);
}

SF_INTERNAL void SFPatternBuilderSetScript(SFPatternBuilderRef builder, SFScript script)
{
    builder->_script = script;
}

SF_INTERNAL void SFPatternBuilderSetLanguage(SFPatternBuilderRef builder, SFLanguage language)
{
    builder->_language = language;
}

SF_INTERNAL void SFPatternBuilderAddFeature(SFPatternBuilderRef builder, SFFeature feature)
{
    /* Only unique features can be added. */
    SFAssert(!SFListContainsItem(&builder->_featureTags, &feature));

    SFListAdd(&builder->_featureTags, feature);
}

SF_INTERNAL void SFPatternBuilderBeginHeader(SFPatternBuilderRef builder, SFHeaderKind kind)
{
    builder->_currentHeader = kind;
}

SF_INTERNAL void SFPatternBuilderAddGroup(SFPatternBuilderRef builder, SFUInteger featureCount)
{
    SFFeatureGroup featureGroup;

    /* The previous group must be closed before adding a new one. */
    SFAssert(builder->_currentGroup == NULL);
    /* Feature count must be valid. */
    SFAssert((builder->_featureIndex + featureCount) <= builder->_featureTags.items.count);

    featureGroup.lookupIndexes = NULL;
    featureGroup.lookupCount = 0;
    featureGroup.featureIndex = builder->_featureIndex;
    featureGroup.featureCount = featureCount;
    featureGroup.headerKind = builder->_currentHeader;

    SFListAdd(&builder->_featureGroups, featureGroup);
    builder->_currentGroup = SFListGetRef(&builder->_featureGroups, builder->_featureGroups.items.count);

    switch (builder->_currentHeader) {
    case SFHeaderKindGSUB:
        builder->_gsubGroupCount++;
        break;

    case SFHeaderKindGPOS:
        builder->_gposGroupCount++;
        break;
    }

    SFListInitialize(&builder->_lookupIndexes, sizeof(SFUInt16));
    SFListSetCapacity(&builder->_lookupIndexes, 32);
}

SF_INTERNAL void SFPatternBuilderAddLookup(SFPatternBuilderRef builder, SFUInt16 lookupIndex)
{
    /* A feature group must be available before adding lookups. */
    SFAssert(builder->_currentGroup != NULL);

    /* Add only unique lookup indexes. */
    if (!SFListContainsItem(&builder->_lookupIndexes, &lookupIndex)) {
        SFListAdd(&builder->_lookupIndexes, lookupIndex);
    }
}

static int _SFLookupIndexComparison(const void *item1, const void *item2)
{
    SFUInteger *ref1 = (SFUInteger *)item1;
    SFUInteger *ref2 = (SFUInteger *)item2;

    return (*ref1 - *ref2);
}

SF_INTERNAL void SFPatternBuilderCloseGroup(SFPatternBuilderRef builder)
{
    SFFeatureGroupRef featureGroup = builder->_currentGroup;

    /* A feature must be available before it can be closed. */
    SFAssert(featureGroup != NULL);

    /* Sort all lookup indexes. */
    SFListSort(&builder->_lookupIndexes, 0, builder->_lookupIndexes.items.count, _SFLookupIndexComparison);
    /* Set lookup indexes in current feature group. */
    SFListFinalizeKeepingArray(&builder->_lookupIndexes, &featureGroup->lookupIndexes, &featureGroup->lookupCount);

    /* Dismiss current feature group from builder. */
    builder->_featureIndex += featureGroup->featureCount;
    builder->_currentGroup = NULL;
}

SF_INTERNAL void SFPatternBuilderBuild(SFPatternBuilderRef builder, SFPatternRef pattern)
{
    SFUInteger groupCount;

    /* Initialize pattern. */
    pattern->groupCount.gsub = builder->_gsubGroupCount;
    pattern->groupCount.gpos = builder->_gposGroupCount;
    pattern->script = builder->_script;
    pattern->language = builder->_language;

    SFListFinalizeKeepingArray(&builder->_featureTags, &pattern->featureTagArray, &pattern->featureCount);
    SFListFinalizeKeepingArray(&builder->_featureGroups, &pattern->featureGroupArray, &groupCount);
}

SF_INTERNAL void SFPatternBuilderFinalize(SFPatternBuilderRef builder)
{
}
