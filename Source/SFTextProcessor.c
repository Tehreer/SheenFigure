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

#include "SFAlbum.h"
#include "SFArtist.h"
#include "SFAssert.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFFont.h"
#include "SFGDEF.h"
#include "SFGSUB.h"
#include "SFLocator.h"
#include "SFPattern.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static void ApplyFeatureRange(TextProcessorRef textProcessor, SFFeatureKind featureKind, SFUInteger index, SFUInteger count);

static LookupType PrepareLookup(TextProcessorRef textProcessor, SFUInt16 lookupIndex, Data *outLookupTable);
static void ApplySubtables(TextProcessorRef textProcessor, Data lookupTable, LookupType lookupType);

SF_INTERNAL void TextProcessorInitialize(TextProcessorRef textProcessor,
    SFPatternRef pattern, SFAlbumRef album, SFTextDirection textDirection,
    SFUInt16 ppemWidth, SFUInt16 ppemHeight, SFBoolean zeroWidthMarks)
{
    Data gdef;

    /* Pattern must NOT be null. */
    SFAssert(pattern != NULL);
    /* Album must NOT be null. */
    SFAssert(album != NULL);

    textProcessor->_pattern = pattern;
    textProcessor->_album = album;
    textProcessor->_glyphClassDef = NULL;
    textProcessor->_textDirection = textDirection;
    textProcessor->_ppemWidth = ppemWidth;
    textProcessor->_ppemHeight = ppemHeight;
    textProcessor->_zeroWidthMarks = zeroWidthMarks;
    textProcessor->_containsZeroWidthCodepoints = SFFalse;

    gdef = pattern->font->tables.gdef;
    if (gdef) {
        textProcessor->_glyphClassDef = GDEF_GlyphClassDefTable(gdef);
    }

    LocatorInitialize(&textProcessor->_locator, album, gdef);
}

SF_INTERNAL void TextProcessorDiscoverGlyphs(TextProcessorRef textProcessor)
{
    SFAlbumBeginFilling(textProcessor->_album);
    DiscoverGlyphs(textProcessor);
}

SF_INTERNAL void TextProcessorSubstituteGlyphs(TextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFPatternRef pattern = textProcessor->_pattern;
    Data gsubTable = pattern->font->tables.gsub;

    if (gsubTable) {
        Data lookupListTable = Header_LookupListTable(gsubTable);

        textProcessor->_lookupList = lookupListTable;
        textProcessor->_lookupOperation = ApplySubstitutionSubtable;

        ApplyFeatureRange(textProcessor, SFFeatureKindSubstitution, 0, pattern->featureUnits.gsub);
    }

    SFAlbumEndFilling(album);
}

static void HandleZeroWidthGlyphs(TextProcessorRef textProcessor)
{
    if (textProcessor->_containsZeroWidthCodepoints) {
        SFFontRef font = textProcessor->_pattern->font;
        SFAlbumRef album = textProcessor->_album;
        SFUInteger glyphCount = album->glyphCount;
        SFGlyphID spaceGlyph;
        SFUInteger index;

        spaceGlyph = SFFontGetGlyphIDForCodepoint(font, ' ');

        for (index = 0; index < glyphCount; index++) {
            GlyphTraits traits = SFAlbumGetAllTraits(album, index);
            if (traits & GlyphTraitZeroWidth) {
                SFAlbumSetGlyph(album, index, spaceGlyph);
                SFAlbumSetX(album, index, 0);
                SFAlbumSetY(album, index, 0);
                SFAlbumSetAdvance(album, index, 0);
            }
        }
    }
}

static void MakeMarksZeroWidth(TextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFUInteger count = SFAlbumGetGlyphCount(album);
    SFUInteger index;

    for (index = 0; index < count; index++) {
        GlyphTraits traits = SFAlbumGetAllTraits(album, index);
        if (traits & GlyphTraitMark) {
            SFAlbumSetAdvance(album, index, 0);
        }
    }
}

SF_INTERNAL void TextProcessorPositionGlyphs(TextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFPatternRef pattern = textProcessor->_pattern;
    SFFontRef font = pattern->font;
    Data gposTable = font->tables.gpos;
    SFUInteger glyphCount = album->glyphCount;
    SFUInteger index;

    SFAlbumBeginArranging(album);

    /* Set positions and advances of all glyphs. */
    for (index = 0; index < glyphCount; index++) {
        SFGlyphID glyphID = SFAlbumGetGlyph(album, index);
        GlyphTraits traits = SFAlbumGetAllTraits(album, index);
        SFAdvance advance = 0;

        /* Ignore placeholder glyphs. */
        if (traits != GlyphTraitPlaceholder) {
            advance = SFFontGetAdvanceForGlyph(font, SFFontLayoutHorizontal, glyphID);
        }

        SFAlbumSetX(album, index, 0);
        SFAlbumSetY(album, index, 0);
        SFAlbumSetAdvance(album, index, advance);
    }

    if (gposTable) {
        Data lookupListTable = Header_LookupListTable(gposTable);

        textProcessor->_lookupList = lookupListTable;
        textProcessor->_lookupOperation = ApplyPositioningSubtable;

        ApplyFeatureRange(textProcessor, SFFeatureKindPositioning, pattern->featureUnits.gsub, pattern->featureUnits.gpos);
        HandleZeroWidthGlyphs(textProcessor);

        if (textProcessor->_zeroWidthMarks) {
            MakeMarksZeroWidth(textProcessor);
        }

        ResolveAttachments(textProcessor);
    }

    SFAlbumEndArranging(album);
}

SF_INTERNAL void TextProcessorWrapUp(TextProcessorRef textProcessor)
{
    SFAlbumWrapUp(textProcessor->_album);
}

static void ApplyFeatureRange(TextProcessorRef textProcessor, SFFeatureKind featureKind, SFUInteger index, SFUInteger count)
{
    SFPatternRef pattern = textProcessor->_pattern;
    SFUInteger limit = index + count;
    SFBoolean reversible = (featureKind == SFFeatureKindSubstitution);

    for (; index < limit; index++) {
        SFFeatureUnitRef featureUnit = &pattern->featureUnits.items[index];
        SFLookupInfo *lookupArray = featureUnit->lookups.items;
        SFUInteger lookupCount = featureUnit->lookups.count;
        SFUInteger lookupIndex;

        /* Apply all lookups of the feature unit. */
        for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
            SFAlbumRef album = textProcessor->_album;
            LocatorRef locator = &textProcessor->_locator;
            SFLookupInfoRef lookupInfo = &lookupArray[lookupIndex];
            Data lookupTable;
            LookupType lookupType;

            LocatorReset(locator, 0, album->glyphCount);
            LocatorSetFeatureMask(locator, featureUnit->mask);

            lookupType = PrepareLookup(textProcessor, lookupInfo->index, &lookupTable);
            textProcessor->_lookupValue = lookupInfo->value;

            /* Apply current lookup on all glyphs. */
            if (!reversible || lookupType != LookupTypeReverseChainingContext) {
                while (LocatorMoveNext(locator)) {
                    ApplySubtables(textProcessor, lookupTable, lookupType);
                }
            } else {
                LocatorJumpTo(locator, album->glyphCount);

                while (LocatorMovePrevious(locator)) {
                    ApplySubtables(textProcessor, lookupTable, lookupType);
                }
            }
        }
    }
}

SF_PRIVATE void ApplyLookup(TextProcessorRef textProcessor, SFUInt16 lookupIndex)
{
    Data lookupTable;
    LookupType lookupType;

    lookupType = PrepareLookup(textProcessor, lookupIndex, &lookupTable);
    ApplySubtables(textProcessor, lookupTable, lookupType);
}

static LookupType PrepareLookup(TextProcessorRef textProcessor, SFUInt16 lookupIndex, Data *outLookupTable)
{
    Data lookupListTable = textProcessor->_lookupList;
    Data lookupTable;
    LookupType lookupType;
    LookupFlag lookupFlag;

    lookupTable = LookupList_LookupTable(lookupListTable, lookupIndex);
    lookupType = Lookup_LookupType(lookupTable);
    lookupFlag = Lookup_LookupFlag(lookupTable);

    LocatorSetLookupFlag(&textProcessor->_locator, lookupFlag);

    if (lookupFlag & LookupFlagUseMarkFilteringSet) {
        SFUInt16 subtableCount = Lookup_SubtableCount(lookupTable);
        SFUInt16 markFilteringSet = Lookup_MarkFilteringSet(lookupTable, subtableCount);

        LocatorSetMarkFilteringSet(&textProcessor->_locator, markFilteringSet);
    }

    *outLookupTable = lookupTable;
    return lookupType;
}

static void ApplySubtables(TextProcessorRef textProcessor, Data lookupTable, LookupType lookupType)
{
    SFUInt16 subtableCount = Lookup_SubtableCount(lookupTable);
    SFUInteger subtableIndex;

    /* Apply subtables in order until one of them performs substitution/positioning. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        Data subtable = Lookup_SubtableData(lookupTable, subtableIndex);

        if (textProcessor->_lookupOperation(textProcessor, lookupType, subtable)) {
            /* A subtable has performed substitution/positioning, so break the loop. */
            break;
        }
    }
}
