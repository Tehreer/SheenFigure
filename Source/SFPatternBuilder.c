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
#include <SFFeatureTag.h>
#include <SFLanguageTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SFAssert.h"
#include "SFList.h"
#include "SFPattern.h"
#include "SFPatternBuilder.h"

SF_INTERNAL void SFPatternBuilderInitialize(SFPatternBuilderRef builder)
{
    /* Initialize builder. */
    builder->_currentHeader = 0;
    builder->_gsubGroupCount = 0;
    builder->_gposGroupCount = 0;
    builder->_featureIndex = 0;

    SFListInitialize(&builder->_featureTags, sizeof(SFFeatureTag));
    SFListSetCapacity(&builder->_featureTags, 24);

    SFListInitialize(&builder->_featureGroups, sizeof(SFFeatureGroup));
    SFListSetCapacity(&builder->_featureGroups, 24);

    SFListInitialize(&builder->_lookupIndexes, sizeof(SFUInt16));
    SFListSetCapacity(&builder->_lookupIndexes, 32);
}

SF_INTERNAL void SFPatternBuilderSetScript(SFPatternBuilderRef builder, SFScriptTag scriptTag)
{
    builder->_scriptTag = scriptTag;
}

SF_INTERNAL void SFPatternBuilderSetLanguage(SFPatternBuilderRef builder, SFLanguageTag languageTag)
{
    builder->_languageTag = languageTag;
}

SF_INTERNAL void SFPatternBuilderAddFeature(SFPatternBuilderRef builder, SFFeatureTag featureTag)
{
    /* Only unique features can be added. */
    SFAssert(!SFListContainsItem(&builder->_featureTags, &featureTag));

    SFListAdd(&builder->_featureTags, featureTag);
}

SF_INTERNAL void SFPatternBuilderBeginHeader(SFPatternBuilderRef builder, SFHeaderKind kind)
{
    builder->_currentHeader = kind;
}

SF_INTERNAL void SFPatternBuilderAddLookup(SFPatternBuilderRef builder, SFUInt16 lookupIndex)
{
    /* Add only unique lookup indexes. */
    if (!SFListContainsItem(&builder->_lookupIndexes, &lookupIndex)) {
        SFListAdd(&builder->_lookupIndexes, lookupIndex);
    }
}

static int _SFLookupIndexComparison(const void *item1, const void *item2)
{
    SFUInteger *ref1 = (SFUInteger *)item1;
    SFUInteger *ref2 = (SFUInteger *)item2;

    return (int)(*ref1 - *ref2);
}

SF_INTERNAL void SFPatternBuilderMakeGroup(SFPatternBuilderRef builder)
{
    SFFeatureGroup featureGroup;
    featureGroup.lookupIndexes = NULL;
    featureGroup.lookupCount = 0;
    featureGroup.featureIndex = builder->_featureIndex;
    featureGroup.featureCount = builder->_featureTags.items.count - featureGroup.featureIndex;
    featureGroup.headerKind = builder->_currentHeader;

    /* Sort all lookup indexes. */
    SFListSort(&builder->_lookupIndexes, 0, builder->_lookupIndexes.items.count, _SFLookupIndexComparison);
    /* Set lookup indexes in current feature group. */
    SFListFinalizeKeepingArray(&builder->_lookupIndexes, &featureGroup.lookupIndexes, &featureGroup.lookupCount);

    /* Add the group in the list. */
    SFListAdd(&builder->_featureGroups, featureGroup);

    switch (builder->_currentHeader) {
    case SFHeaderKindGSUB:
        builder->_gsubGroupCount++;
        break;

    case SFHeaderKindGPOS:
        builder->_gposGroupCount++;
        break;
    }

    /* Initialize lookup indexes array. */
    SFListInitialize(&builder->_lookupIndexes, sizeof(SFUInt16));
    SFListSetCapacity(&builder->_lookupIndexes, 32);
}

SF_INTERNAL void SFPatternBuilderBuild(SFPatternBuilderRef builder, SFPatternRef pattern)
{
    SFUInteger groupCount;

    /* Initialize pattern. */
    pattern->groupCount.gsub = builder->_gsubGroupCount;
    pattern->groupCount.gpos = builder->_gposGroupCount;
    pattern->scriptTag = builder->_scriptTag;
    pattern->languageTag = builder->_languageTag;

    SFListFinalizeKeepingArray(&builder->_featureTags, &pattern->featureTagArray, &pattern->featureCount);
    SFListFinalizeKeepingArray(&builder->_featureGroups, &pattern->featureGroupArray, &groupCount);
}

SF_INTERNAL void SFPatternBuilderFinalize(SFPatternBuilderRef builder)
{
    SFListFinalize(&builder->_lookupIndexes);
}
