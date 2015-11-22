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
#include <SFScript.h>
#include <SFTypes.h>

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFData.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"

#include "SFGlyphDiscovery.h"
#include "SFShapingEngine.h"

static void _SFApplyAllFeatures(SFShapingEngineRef shapingEngine);
static void _SFApplyFeaturesInRange(SFShapingEngineRef shapingEngine, SFUInteger startIndex, SFUInteger limitIndex);
static void _SFApplyFeature(SFShapingEngineRef shapingEngine, SFUInteger featureIndex);
static void _SFApplyGroup(SFShapingEngineRef shapingEngine, SFUInteger groupIndex);
static void _SFApplyLookup(SFShapingEngineRef shapingEngine, SFData lookup, SFHeaderKind headerKind);

SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScript script)
{
    (*shapingKnowledge->_seekScript)(shapingKnowledge, script);
}

SF_INTERNAL SFUInteger SFScriptKnowledgeSeekFeature(SFScriptKnowledgeRef scriptKnowledge, SFFeature feature)
{
    if (!scriptKnowledge->_seekFeature) {
        SFUInteger index;

        for (index = 0; index < scriptKnowledge->featureCount; index++) {
            if (scriptKnowledge->featureArray[index] == feature) {
                return index;
            }
        }

        return SFInvalidIndex;
    }

    /* Invoke overridden implementation. */
    return (*scriptKnowledge->_seekFeature)(scriptKnowledge, feature);
}

SF_INTERNAL void SFShapingEngineInitialize(SFShapingEngineRef shapingEngine, SFShapingTraits traits, SFFontRef font, SFCollectionRef collection)
{
	/* The font must NOT be null. */
	SFAssert(font != NULL);
	/* The collection must NOT be null. */
	SFAssert(collection != NULL);

    shapingEngine->_langDetail = SFScriptCacheFindLanguage(&font->scripts, traits.script, traits.language);
    shapingEngine->_collection = collection;
    shapingEngine->_traits = traits;
}

SF_INTERNAL void SFShapingEnginePerformShaping(SFShapingEngineRef shapingEngine)
{
	_SFDiscoverGlyphs(shapingEngine);
	_SFApplyAllFeatures(shapingEngine);
}

static void _SFApplyAllFeatures(SFShapingEngineRef shapingEngine)
{
    SFLanguageDetailRef langDetail = shapingEngine->_langDetail;

    if (langDetail) {
        SFGroupDetail *groupArray = langDetail->groupArray;
        SFUInteger groupCount = langDetail->groupCount;
        SFUInteger gapIndex = 0;
        SFUInteger groupIndex;

        for (groupIndex = 0; groupIndex < groupCount; groupIndex++) {
            SFGroupDetailRef group = &groupArray[groupIndex];
            SFUInteger startIndex = group->featureIndex;

            /* The range of group features must fall within language features. */
            SFAssert((startIndex + group->featureCount) <= langDetail->featureCount);
            /* Group ranges must be continuous. */
            SFAssert(gapIndex <= startIndex);

            /* Apply all features in the gap before group. */
            _SFApplyFeaturesInRange(shapingEngine, gapIndex, startIndex);
            /* Apply the group. */
            _SFApplyGroup(shapingEngine, groupIndex);

            gapIndex = startIndex + group->featureCount;
        }

        /* Apply remaining features. */
        _SFApplyFeaturesInRange(shapingEngine, gapIndex, langDetail->featureCount);
    }
}

static void _SFApplyFeaturesInRange(SFShapingEngineRef shapingEngine, SFUInteger startIndex, SFUInteger limitIndex)
{
    /* Apply all features individually. */
    for (; startIndex < limitIndex; startIndex++) {
        _SFApplyFeature(shapingEngine, startIndex);
    }
}

static void _SFApplyFeature(SFShapingEngineRef shapingEngine, SFUInteger featureIndex)
{
    SFFeatureDetailRef feature = &shapingEngine->_langDetail->featureArray[featureIndex];
    SFLookupDetail *lookupArray = feature->lookupArray;
    SFUInteger lookupCount = feature->lookupCount;
    SFUInteger lookupIndex;

    shapingEngine->_feature = feature->feature;

    /* Apply all lookups of the feature. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFLookupDetailRef lookup = &lookupArray[lookupIndex];
        _SFApplyLookup(shapingEngine, lookup->table, feature->headerKind);
    }
}

static void _SFApplyGroup(SFShapingEngineRef shapingEngine, SFUInteger groupIndex)
{
    SFLanguageDetailRef langDetail = shapingEngine->_langDetail;
    SFGroupDetailRef group = &langDetail->groupArray[groupIndex];
    SFHeaderKind headerKind = langDetail->featureArray[group->featureIndex].headerKind;
    SFLookupDetail *lookupArray = group->lookupArray;
    SFUInteger lookupCount = group->lookupCount;
    SFUInteger lookupIndex;

    /* Apply all lookups of the group. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFLookupDetailRef lookup = &lookupArray[lookupIndex];
        _SFApplyLookup(shapingEngine, lookup->table, headerKind);
    }
}

static void _SFApplyLookup(SFShapingEngineRef shapingEngine, SFData lookup, SFHeaderKind headerKind)
{
    if (headerKind == SFHeaderKindGSUB) {
        _SFApplyGSUBLookup(shapingEngine, lookup);
    } else if (headerKind == SFHeaderKindGPOS) {
        _SFApplyGPOSLookup(shapingEngine, lookup);
    }
}
