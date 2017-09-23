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

#include "SFAlbum.h"
#include "SFArtist.h"
#include "SFAssert.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFFont.h"
#include "SFGDEF.h"
#include "SFPattern.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static void _SFApplyFeatureRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count);

static void _SFPrepareLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex, SFData *outLookupTable);
static void _SFApplySubtables(SFTextProcessorRef processor, SFData lookupTable);

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFPatternRef pattern,
    SFAlbumRef album, SFTextDirection textDirection, SFTextMode textMode)
{
    SFData gdef;

    /* Pattern must NOT be null. */
    SFAssert(pattern != NULL);
    /* Album must NOT be null. */
    SFAssert(album != NULL);

    textProcessor->_pattern = pattern;
    textProcessor->_album = album;
    textProcessor->_glyphClassDef = NULL;
    textProcessor->_textDirection = textDirection;
    textProcessor->_textMode = textMode;

    gdef = pattern->font->tables.gdef;
    if (gdef) {
        SFOffset offset = SFGDEF_GlyphClassDefOffset(gdef);
        textProcessor->_glyphClassDef = SFData_Subdata(gdef, offset);
    }

    SFLocatorInitialize(&textProcessor->_locator, album, gdef);
}

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    SFAlbumBeginFilling(textProcessor->_album);
    _SFDiscoverGlyphs(textProcessor);
}

SF_INTERNAL void SFTextProcessorSubstituteGlyphs(SFTextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFPatternRef pattern = textProcessor->_pattern;
    SFData gsubTable = pattern->font->tables.gsub;

    if (gsubTable) {
        SFOffset lookupListOffset = SFHeader_LookupListOffset(gsubTable);
        SFData lookupListTable = SFData_Subdata(gsubTable, lookupListOffset);

        textProcessor->_lookupList = lookupListTable;
        textProcessor->_lookupOperation = _SFApplySubstitutionSubtable;

        _SFApplyFeatureRange(textProcessor, 0, pattern->featureUnits.gsub);
    }

    SFAlbumEndFilling(album);
}

SF_INTERNAL void SFTextProcessorPositionGlyphs(SFTextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFPatternRef pattern = textProcessor->_pattern;
    SFFontRef font = pattern->font;
    SFData gposTable = font->tables.gpos;
    SFUInteger glyphCount = album->glyphCount;
    SFUInteger index;

    SFAlbumBeginArranging(album);

    /* Set positions and advances of all glyphs. */
    for (index = 0; index < glyphCount; index++) {
        SFGlyphID glyphID = SFAlbumGetGlyph(album, index);
        SFGlyphTraits traits = SFAlbumGetTraits(album, index);
        SFAdvance advance = 0;

        /* Ignore placeholder glyphs. */
        if (traits != SFGlyphTraitPlaceholder) {
            advance = SFFontGetAdvanceForGlyph(font, SFFontLayoutHorizontal, glyphID);
        }

        SFAlbumSetX(album, index, 0);
        SFAlbumSetY(album, index, 0);
        SFAlbumSetAdvance(album, index, advance);
    }

    if (gposTable) {
        SFOffset lookupListOffset = SFHeader_LookupListOffset(gposTable);
        SFData lookupListTable = SFData_Subdata(gposTable, lookupListOffset);

        textProcessor->_lookupList = lookupListTable;
        textProcessor->_lookupOperation = _SFApplyPositioningSubtable;

        _SFApplyFeatureRange(textProcessor, pattern->featureUnits.gsub, pattern->featureUnits.gpos);
        _SFResolveAttachments(textProcessor);
    }

    SFAlbumEndArranging(album);
}

SF_INTERNAL void SFTextProcessorWrapUp(SFTextProcessorRef textProcessor)
{
    SFAlbumWrapUp(textProcessor->_album);
}

static void _SFApplyFeatureRange(SFTextProcessorRef processor, SFUInteger index, SFUInteger count)
{
    SFPatternRef pattern = processor->_pattern;
    SFUInteger limit = index + count;

    for (; index < limit; index++) {
        SFFeatureUnitRef featureUnit = &pattern->featureUnits.items[index];
        SFUInt16 *lookupArray = featureUnit->lookupIndexes.items;
        SFUInteger lookupCount = featureUnit->lookupIndexes.count;
        SFUInteger lookupIndex;

        /* Apply all lookups of the feature unit. */
        for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
            SFLocatorRef locator = &processor->_locator;
            SFData lookupTable;

            SFLocatorReset(locator, 0, processor->_album->glyphCount);
            SFLocatorSetFeatureMask(locator, featureUnit->featureMask);

            _SFPrepareLookup(processor, lookupArray[lookupIndex], &lookupTable);

            /* Apply current lookup on all glyphs. */
            while (SFLocatorMoveNext(locator)) {
                _SFApplySubtables(processor, lookupTable);
            }
        }
    }
}

SF_PRIVATE void _SFApplyLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex)
{
    SFData lookupTable;

    _SFPrepareLookup(processor, lookupIndex, &lookupTable);
    _SFApplySubtables(processor, lookupTable);
}

static void _SFPrepareLookup(SFTextProcessorRef processor, SFUInt16 lookupIndex, SFData *outLookupTable)
{
    SFData lookupListTable = processor->_lookupList;
    SFData lookupTable;
    SFLookupFlag lookupFlag;

    lookupTable = SFLookupList_LookupTable(lookupListTable, lookupIndex);
    lookupFlag = SFLookup_LookupFlag(lookupTable);

    SFLocatorSetLookupFlag(&processor->_locator, lookupFlag);

    if (lookupFlag & SFLookupFlagUseMarkFilteringSet) {
        SFUInt16 subtableCount = SFLookup_SubtableCount(lookupTable);
        SFUInt16 markFilteringSet = SFLookup_MarkFilteringSet(lookupTable, subtableCount);

        SFLocatorSetMarkFilteringSet(&processor->_locator, markFilteringSet);
    }

    *outLookupTable = lookupTable;
}

static void _SFApplySubtables(SFTextProcessorRef processor, SFData lookupTable)
{
    SFLookupType lookupType;
    SFUInt16 subtableCount;
    SFUInteger subtableIndex;

    lookupType = SFLookup_LookupType(lookupTable);
    subtableCount = SFLookup_SubtableCount(lookupTable);

    /* Apply subtables in order until one of them performs substitution/positioning. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFData subtable = SFLookup_SubtableData(lookupTable, subtableIndex);

        if (processor->_lookupOperation(processor, lookupType, subtable)) {
            /* A subtable has performed substitution/positioning, so break the loop. */
            break;
        }
    }
}
