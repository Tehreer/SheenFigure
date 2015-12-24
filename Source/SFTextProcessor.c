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
#include "SFCommon.h"
#include "SFData.h"
#include "SFPattern.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static SFData _SFGetLookupFromHeader(SFData header, SFUInt16 lookupIndex);

static void _SFApplyAllFeatures(SFTextProcessorRef processor);
static void _SFApplyFeatureGroup(SFTextProcessorRef processor, SFFeatureGroupRef featureGroup);
static void _SFApplyLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex, SFHeaderKind headerKind);

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFFontRef font, SFPatternRef pattern, SFAlbumRef album)
{
    /* Font must NOT be null. */
    SFAssert(font != NULL);
    /* Album must NOT be null. */
    SFAssert(album != NULL);

    textProcessor->_font = font;
    textProcessor->_pattern = pattern;
    textProcessor->_album = album;
}

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    _SFDiscoverGlyphs(textProcessor);
}

SF_INTERNAL void SFTextProcessorManipulateGlyphs(SFTextProcessorRef textProcessor)
{
    /* Pattern must NOT be null. */
    SFAssert(textProcessor->_pattern != NULL);

    _SFApplyAllFeatures(textProcessor);
}

static SFData _SFGetLookupFromHeader(SFData header, SFUInt16 lookupIndex)
{
    SFOffset listOffset = SF_HEADER__LOOKUP_LIST(header);
    SFData lookupList = SF_DATA__SUBDATA(header, listOffset);
    SFOffset lookupOffset = SF_LOOKUP_LIST__LOOKUP(lookupList, lookupIndex);
    SFData lookup = SF_DATA__SUBDATA(lookupList, lookupOffset);

    return lookup;
}

static void _SFApplyAllFeatures(SFTextProcessorRef processor)
{
    SFPatternRef pattern = processor->_pattern;
    SFUInteger groupCount = pattern->groupCount.gsub + pattern->groupCount.gpos;
    SFUInteger nextIndex = 0;
    SFUInteger groupIndex;

    for (groupIndex = 0; groupIndex < groupCount; groupIndex++) {
        SFFeatureGroupRef featureGroup = &pattern->featureGroupArray[groupIndex];
        SFUInteger startIndex = featureGroup->featureIndex;

        /* Group ranges must be continuous. */
        SFAssert(nextIndex == startIndex);

        /* Apply the group. */
        _SFApplyFeatureGroup(processor, featureGroup);

        nextIndex = startIndex + featureGroup->featureCount;
    }
}

static void _SFApplyFeatureGroup(SFTextProcessorRef processor, SFFeatureGroupRef featureGroup)
{
    SFHeaderKind headerKind = featureGroup->headerKind;
    SFUInt16 *lookupArray = featureGroup->lookupIndexes;
    SFUInteger lookupCount = featureGroup->lookupCount;
    SFUInteger lookupIndex;

    /* Apply all lookups of the group. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        _SFApplyLookup(processor, lookupArray[lookupIndex], headerKind);
    }
}

static void _SFApplyLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex, SFHeaderKind headerKind)
{
    SFLocatorRef locator = processor->_locator;
    SFLocatorReset(locator);

    processor->_headerKind = headerKind;

    if (headerKind == SFHeaderKindGSUB) {
        SFData lookup = _SFGetLookupFromHeader(processor->_font->tables.gsub, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplyGSUBLookup(processor, lookup);
        }
    } else if (headerKind == SFHeaderKindGPOS) {
        SFData lookup = _SFGetLookupFromHeader(processor->_font->tables.gpos, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplyGPOSLookup(processor, lookup);
        }
    }
}
