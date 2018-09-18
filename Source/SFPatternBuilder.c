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

#include "SFArtist.h"
#include "SFAssert.h"
#include "SFBase.h"
#include "SFList.h"
#include "SFPattern.h"
#include "SFPatternBuilder.h"

static int _SFLookupIndexComparison(const void *item1, const void *item2)
{
    SFLookupInfo *ref1 = (SFLookupInfo *)item1;
    SFLookupInfo *ref2 = (SFLookupInfo *)item2;

    return (int)(ref1->index - ref2->index);
}

SF_INTERNAL void SFPatternBuilderInitialize(SFPatternBuilderRef builder, SFPatternRef pattern)
{
    /* Pattern must NOT be null. */
    SFAssert(pattern != NULL);

    /* Initialize the builder. */
    builder->_pattern = pattern;
    builder->_font = NULL;
    builder->_gsubUnitCount = 0;
    builder->_gposUnitCount = 0;
    builder->_featureIndex = 0;
    builder->_scriptTag = 0;
    builder->_languageTag = 0;
    builder->_defaultDirection = SFTextDirectionLeftToRight;
    builder->_featureMask = 0;
    builder->_featureKind = 0;
    builder->_canBuild = SFTrue;

    SFListInitialize(&builder->_featureTags, sizeof(SFTag));
    SFListSetCapacity(&builder->_featureTags, 24);

    SFListInitialize(&builder->_featureUnits, sizeof(SFFeatureUnit));
    SFListSetCapacity(&builder->_featureUnits, 24);

    SFListInitialize(&builder->_lookupInfos, sizeof(SFLookupInfo));
    SFListSetCapacity(&builder->_lookupInfos, 32);
}

SF_INTERNAL void SFPatternBuilderFinalize(SFPatternBuilderRef builder)
{
    /* The pattern MUST be built before finalizing the builder. */
    SFAssert(builder->_canBuild == SFFalse);

    SFListFinalize(&builder->_lookupInfos);
}

SF_INTERNAL void SFPatternBuilderSetFont(SFPatternBuilderRef builder, SFFontRef font)
{
    /* Font must NOT be null. */
    SFAssert(font != NULL);

    builder->_font = font;
}

SF_INTERNAL void SFPatternBuilderSetScript(SFPatternBuilderRef builder, SFTag scriptTag, SFTextDirection defaultDirection)
{
    /* The direction MUST be valid. */
    SFAssert(defaultDirection == SFTextDirectionLeftToRight || defaultDirection == SFTextDirectionRightToLeft);

    builder->_scriptTag = scriptTag;
    builder->_defaultDirection = defaultDirection;
}

SF_INTERNAL void SFPatternBuilderSetLanguage(SFPatternBuilderRef builder, SFTag languageTag)
{
    builder->_languageTag = languageTag;
}

SF_INTERNAL void SFPatternBuilderBeginFeatures(SFPatternBuilderRef builder, SFFeatureKind featureKind)
{
    /* One kind of features must be ended before beginning new ones. */
    SFAssert(builder->_featureKind == 0);
    /* Feature kind must be valid. */
    SFAssert(featureKind == SFFeatureKindSubstitution || featureKind == SFFeatureKindPositioning);

    builder->_featureKind = featureKind;
}

SF_INTERNAL void SFPatternBuilderAddFeature(SFPatternBuilderRef builder,
    SFTag featureTag, SFUInt16 featureValue, SFUInt16 featureMask)
{
    /* The kind of features must be specified before adding them. */
    SFAssert(builder->_featureKind != 0);
    /* Only unique features can be added. */
    SFAssert(!SFListContainsItem(&builder->_featureTags, &featureTag));
    /* Feature value must be non-zero. */
    SFAssert(featureValue != 0);

    /* Add the feature in the list. */
    SFListAdd(&builder->_featureTags, featureTag);
    /* Set the value of the feature. */
    builder->_featureValue = featureValue;
    /* Insert the mask of the feature. */
    builder->_featureMask |= featureMask;
}

SF_INTERNAL void SFPatternBuilderAddLookup(SFPatternBuilderRef builder, SFUInt16 lookupIndex)
{
    SFLookupInfo lookupInfo;

    /* A feature MUST be available before adding lookups. */
    SFAssert((builder->_featureTags.count - builder->_featureIndex) > 0);

    lookupInfo.index = lookupIndex;
    lookupInfo.value = builder->_featureValue;

    /* Add only unique lookup indexes. */
    if (!SFListContainsItem(&builder->_lookupInfos, &lookupInfo)) {
        SFListAdd(&builder->_lookupInfos, lookupInfo);
    }
}

SF_INTERNAL void SFPatternBuilderMakeFeatureUnit(SFPatternBuilderRef builder)
{
    SFFeatureUnit featureUnit;

    /* At least one feature MUST be available before making a feature unit. */
    SFAssert((builder->_featureTags.count - builder->_featureIndex) > 0);

    /* Sort all lookup indexes. */
    SFListSort(&builder->_lookupInfos, 0, builder->_lookupInfos.count, _SFLookupIndexComparison);
    /* Set lookup indexes in current feature unit. */
    SFListFinalizeKeepingArray(&builder->_lookupInfos, &featureUnit.lookups.items, &featureUnit.lookups.count);
    /* Set covered range of feature unit. */
    featureUnit.range.start = builder->_featureIndex;
    featureUnit.range.count = builder->_featureTags.count - builder->_featureIndex;
    featureUnit.mask = builder->_featureMask;

    /* Add the feature unit in the list. */
    SFListAdd(&builder->_featureUnits, featureUnit);

    /* Increase feature unit count. */
    switch (builder->_featureKind) {
        case SFFeatureKindSubstitution:
            builder->_gsubUnitCount++;
            break;

        case SFFeatureKindPositioning:
            builder->_gposUnitCount++;
            break;

        default:
            /* Invalid feature kind. */
            SFAssert(0);
            break;
    }

    /* Increase feature index. */
    builder->_featureIndex += featureUnit.range.count;

    /* Initialize lookup indexes array. */
    SFListInitialize(&builder->_lookupInfos, sizeof(SFLookupInfo));
    SFListSetCapacity(&builder->_lookupInfos, 32);
    /* Reset feature mask. */
    builder->_featureMask = 0;
}

SF_INTERNAL void SFPatternBuilderEndFeatures(SFPatternBuilderRef builder)
{
    /* The features of specified kind MUST be begun before ending them. */
    SFAssert(builder->_featureKind != 0);
    /* There should be NO pending feature unit. */
    SFAssert(builder->_featureTags.count == builder->_featureIndex);

    builder->_featureKind = 0;
}

SF_INTERNAL void SFPatternBuilderBuild(SFPatternBuilderRef builder)
{
    SFPatternRef pattern = builder->_pattern;
    SFUInteger unitCount;

    /* Pattern should be built ONLY ONCE with a builder. */
    SFAssert(builder->_canBuild == SFTrue);
    /* All features MUST be ended before building the pattern. */
    SFAssert(builder->_featureKind == 0);

    /* Initialize the pattern. */
    pattern->font = SFFontRetain(builder->_font);
    pattern->featureUnits.gsub = builder->_gsubUnitCount;
    pattern->featureUnits.gpos = builder->_gposUnitCount;
    pattern->scriptTag = builder->_scriptTag;
    pattern->languageTag = builder->_languageTag;
    pattern->defaultDirection = builder->_defaultDirection;

    SFListFinalizeKeepingArray(&builder->_featureTags, &pattern->featureTags.items, &pattern->featureTags.count);
    SFListFinalizeKeepingArray(&builder->_featureUnits, &pattern->featureUnits.items, &unitCount);

    builder->_canBuild = SFFalse;
}
