/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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
#include "SFGDEF.h"
#include "SFPattern.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static SFData _SFGetLookupFromHeader(SFData header, SFUInt16 lookupIndex);

static void _SFApplyFeatureRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count, SFFeatureKind featureKind);
static void _SFApplyFeatureUnit(SFTextProcessorRef processor, SFFeatureUnitRef featureUnit);

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFPatternRef pattern, SFAlbumRef album)
{
    SFData gdef;

    /* Pattern must NOT be null. */
    SFAssert(pattern != NULL);
    /* Album must NOT be null. */
    SFAssert(album != NULL);

    gdef = pattern->font->cache.gdef;

    textProcessor->_font = pattern->font;
    textProcessor->_pattern = pattern;
    textProcessor->_album = album;
    textProcessor->_glyphClassDef = NULL;

    if (gdef) {
        SFOffset offset = SFGDEF_GlyphClassDefOffset(gdef);
        textProcessor->_glyphClassDef = SFData_Subdata(gdef, offset);
    }

    SFLocatorInitialize(&textProcessor->_locator, album, gdef);
}

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    _SFDiscoverGlyphs(textProcessor);
}

SF_INTERNAL void SFTextProcessorSubstituteGlyphs(SFTextProcessorRef textProcessor)
{
    SFPatternRef pattern = textProcessor->_pattern;

    _SFApplyFeatureRange(textProcessor, 0, pattern->featureUnits.gsub, SFFeatureKindSubstitution);
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

        SFAlbumSetPosition(album, index, position);
        SFAlbumSetAdvance(album, index, advance);
    }

    _SFApplyFeatureRange(textProcessor, pattern->featureUnits.gsub, pattern->featureUnits.gpos, SFFeatureKindPositioning);
}

static SFData _SFGetLookupFromHeader(SFData header, SFUInt16 lookupIndex)
{
    SFOffset listOffset = SFHeader_LookupListOffset(header);
    SFData lookupList = SFData_Subdata(header, listOffset);
    SFOffset lookupOffset = SFLookupList_LookupOffset(lookupList, lookupIndex);
    SFData lookup = SFData_Subdata(lookupList, lookupOffset);

    return lookup;
}

static void _SFApplyFeatureRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count, SFFeatureKind featureKind)
{
    SFPatternRef pattern = processor->_pattern;
    SFUInteger limit = index + count;

    processor->_featureKind = featureKind;

    for (; index < limit; index++) {
        /* Apply the feature unit. */
        _SFApplyFeatureUnit(processor, &pattern->featureUnits.items[index]);
    }
}

static void _SFApplyFeatureUnit(SFTextProcessorRef processor, SFFeatureUnitRef featureUnit)
{
    SFUInt16 *lookupArray = featureUnit->lookupIndexes.items;
    SFUInteger lookupCount = featureUnit->lookupIndexes.count;
    SFUInteger lookupIndex;

    /* Apply all lookups of the feature unit. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFLocatorRef locator = &processor->_locator;
        SFLocatorReset(locator, 0, processor->_album->glyphCount);
        SFLocatorSetRequiredTraits(locator, featureUnit->requiredTraits);

        _SFApplyLookup(processor, lookupArray[lookupIndex]);
    }
}

SF_PRIVATE void _SFApplyLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex)
{
    SFLocatorRef locator = &processor->_locator;
    SFData lookup;

    switch (processor->_featureKind) {
    case SFFeatureKindSubstitution:
        lookup = _SFGetLookupFromHeader(processor->_font->cache.gsub, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplySubstitutionLookup(processor, lookup);
        }
        break;

    case SFFeatureKindPositioning:
        lookup = _SFGetLookupFromHeader(processor->_font->cache.gpos, lookupIndex);
        while (SFLocatorMoveNext(locator)) {
            _SFApplyPositioningLookup(processor, lookup);
        }
        break;
    }
}
