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

static void _SFApplyGroupRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count);
static void _SFApplyFeatureGroup(SFTextProcessorRef processor, SFFeatureGroupRef featureGroup);
static void _SFApplyLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex, SFHeaderKind headerKind);

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFPatternRef pattern, SFAlbumRef album)
{
    /* Pattern must NOT be null. */
    SFAssert(pattern != NULL);
    /* Album must NOT be null. */
    SFAssert(album != NULL);

    textProcessor->_font = pattern->font;
    textProcessor->_pattern = pattern;
    textProcessor->_album = album;

    SFLocatorInitialize(&textProcessor->_locator, album, textProcessor->_font->cache.gdef);
}

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    _SFDiscoverGlyphs(textProcessor);
}

SF_INTERNAL void SFTextProcessorSubstituteGlyphs(SFTextProcessorRef textProcessor)
{
    SFPatternRef pattern = textProcessor->_pattern;

    _SFApplyGroupRange(textProcessor, 0, pattern->groupCount.gsub);
}

SF_INTERNAL void SFTextProcessorPositionGlyphs(SFTextProcessorRef textProcessor)
{
    SFPatternRef pattern = textProcessor->_pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = textProcessor->_album;
    SFUInteger glyphCount = album->glyphCount;
    SFUInteger index;

    SFAlbumAllocatePositions(album);

    /* Set positions and advances of all glyphs. */
    for (index = 0; index < glyphCount; index++) {
        SFGlyphID glyphID = SFAlbumGetGlyph(album, index);
        SFPoint position = { 0, 0 };
        SFInteger advance = SFFontGetGlyphAdvance(font, glyphID);

        SFAlbumSetPosition(album, 0, position);
        SFAlbumSetAdvance(album, index, advance);
    }

    _SFApplyGroupRange(textProcessor, pattern->groupCount.gsub, pattern->groupCount.gpos);
}

static SFData _SFGetLookupFromHeader(SFData header, SFUInt16 lookupIndex)
{
    SFOffset listOffset = SF_HEADER__LOOKUP_LIST(header);
    SFData lookupList = SF_DATA__SUBDATA(header, listOffset);
    SFOffset lookupOffset = SF_LOOKUP_LIST__LOOKUP(lookupList, lookupIndex);
    SFData lookup = SF_DATA__SUBDATA(lookupList, lookupOffset);

    return lookup;
}

static void _SFApplyGroupRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count)
{
    SFPatternRef pattern = processor->_pattern;

    for (; index < count; index++) {
        /* Apply the feature group. */
        _SFApplyFeatureGroup(processor, &pattern->featureGroupArray[index]);
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
    SFLocatorRef locator = &processor->_locator;
    SFLocatorReset(locator);

    processor->_headerKind = headerKind;

    if (headerKind == SFHeaderKindGSUB) {
        SFData lookup = _SFGetLookupFromHeader(processor->_font->cache.gsub, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplyGSUBLookup(processor, lookup);
        }
    } else if (headerKind == SFHeaderKindGPOS) {
        SFData lookup = _SFGetLookupFromHeader(processor->_font->cache.gpos, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplyGPOSLookup(processor, lookup);
        }
    }
}
