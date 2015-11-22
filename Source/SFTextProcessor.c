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
#include <SFTypes.h>

#include "SFAssert.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static void _SFApplyAllFeatures(SFTextProcessorRef processor);
static void _SFApplyFeaturesInRange(SFTextProcessorRef processor, SFUInteger startIndex, SFUInteger limitIndex);
static void _SFApplyFeature(SFTextProcessorRef processor, SFUInteger featureIndex);
static void _SFApplyGroup(SFTextProcessorRef processor, SFUInteger groupIndex);
static void _SFApplyLookup(SFTextProcessorRef processor, SFData lookup, SFHeaderKind headerKind);

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFFontRef font, SFLanguageDetailRef langDetail, SFCollectionRef collection)
{
    /* Font must NOT be null. */
    SFAssert(font != NULL);
    /* Collection must NOT be null. */
    SFAssert(collection != NULL);

    textProcessor->_font = font;
    textProcessor->_langDetail = langDetail;
    textProcessor->_collection = collection;
}

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    _SFDiscoverGlyphs(textProcessor);
}

SF_INTERNAL void SFTextProcessorManipulateGlyphs(SFTextProcessorRef textProcessor)
{
    /* Language detail must NOT be null. */
    SFAssert(textProcessor->_langDetail != NULL);

    _SFApplyAllFeatures(textProcessor);
}

static void _SFApplyAllFeatures(SFTextProcessorRef processor)
{
    SFLanguageDetailRef langDetail = processor->_langDetail;
    SFUInteger featureCount = langDetail->featureCount.gsub + langDetail->featureCount.gpos;
    SFUInteger groupCount = langDetail->groupCount;
    SFUInteger gapIndex = 0;
    SFUInteger groupIndex;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++) {
        SFGroupDetailRef group = &langDetail->groupArray[groupIndex];
        SFUInteger startIndex = group->featureIndex;

        /* The range of group features must fall within language features. */
        SFAssert((startIndex + group->featureCount) <= featureCount);
        /* Group ranges must be continuous. */
        SFAssert(gapIndex <= startIndex);

        /* Apply all features in the gap before group. */
        _SFApplyFeaturesInRange(processor, gapIndex, startIndex);
        /* Apply the group. */
        _SFApplyGroup(processor, groupIndex);

        gapIndex = startIndex + group->featureCount;
    }

    /* Apply remaining features. */
    _SFApplyFeaturesInRange(processor, gapIndex, featureCount);
}

static void _SFApplyFeaturesInRange(SFTextProcessorRef processor, SFUInteger startIndex, SFUInteger limitIndex)
{
    /* Apply all features individually. */
    for (; startIndex < limitIndex; startIndex++) {
        _SFApplyFeature(processor, startIndex);
    }
}

static void _SFApplyFeature(SFTextProcessorRef processor, SFUInteger featureIndex)
{
    SFFeatureDetailRef feature = &processor->_langDetail->featureArray[featureIndex];
    SFUInteger lookupCount = feature->lookupCount;
    SFUInteger lookupIndex;

    processor->_feature = feature->feature;

    /* Apply all lookups of the feature. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFLookupDetailRef lookup = &feature->lookupArray[lookupIndex];
        _SFApplyLookup(processor, lookup->table, feature->headerKind);
    }
}

static void _SFApplyGroup(SFTextProcessorRef processor, SFUInteger groupIndex)
{
    SFLanguageDetailRef langDetail = processor->_langDetail;
    SFGroupDetailRef group = &langDetail->groupArray[groupIndex];
    SFHeaderKind headerKind = langDetail->featureArray[group->featureIndex].headerKind;
    SFLookupDetail *lookupArray = group->lookupArray;
    SFUInteger lookupCount = group->lookupCount;
    SFUInteger lookupIndex;

    /* Apply all lookups of the group. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFLookupDetailRef lookup = &lookupArray[lookupIndex];
        _SFApplyLookup(processor, lookup->table, headerKind);
    }
}

static void _SFApplyLookup(SFTextProcessorRef processor, SFData lookup, SFHeaderKind headerKind)
{
    processor->_headerKind = headerKind;

    if (headerKind == SFHeaderKindGSUB) {
        _SFApplyGSUBLookup(processor, lookup);
    } else if (headerKind == SFHeaderKindGPOS) {
        _SFApplyGPOSLookup(processor, lookup);
    }
}
