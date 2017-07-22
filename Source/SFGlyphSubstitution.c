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

#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGSUB.h"
#include "SFLocator.h"
#include "SFPattern.h"
#include "SFOpenType.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef textProcessor, SFData singleSubst);

static SFBoolean _SFApplyMultipleSubst(SFTextProcessorRef textProcessor, SFData multipleSubst);
static SFBoolean _SFApplySequenceTable(SFTextProcessorRef textProcessor, SFData sequenceTable);

static SFBoolean _SFApplyAlternateSubst(SFTextProcessorRef textProcessor, SFData alternateSubst);
static SFBoolean _SFApplyAlternateSetTable(SFTextProcessorRef textProcessor, SFData alternateSetTable);

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef textProcessor, SFData subtable);
static SFBoolean _SFApplyLigatureSetTable(SFTextProcessorRef textProcessor, SFData ligatureSetTable);

SF_PRIVATE SFBoolean _SFApplySubstitutionSubtable(SFTextProcessorRef textProcessor, SFLookupType lookupType, SFData subtable)
{
    switch (lookupType) {
        case SFLookupTypeSingle:
            return _SFApplySingleSubst(textProcessor, subtable);

        case SFLookupTypeMultiple:
            return _SFApplyMultipleSubst(textProcessor, subtable);

        case SFLookupTypeAlternate:
            return _SFApplyAlternateSubst(textProcessor, subtable);

        case SFLookupTypeLigature:
            return _SFApplyLigatureSubst(textProcessor, subtable);

        case SFLookupTypeContext:
            return _SFApplyContextSubtable(textProcessor, subtable);

        case SFLookupTypeChainingContext:
            return _SFApplyChainContextSubtable(textProcessor, subtable);

        case SFLookupTypeExtension:
            return _SFApplyExtensionSubtable(textProcessor, subtable);

        case SFLookupTypeReverseChainingContext:
            break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef textProcessor, SFData singleSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFSingleSubst_Format(singleSubst);

    switch (format) {
        case 1: {
            SFOffset coverageOffset = SFSingleSubstF1_CoverageOffset(singleSubst);
            SFData coverageTable = SFData_Subdata(singleSubst, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFInt16 deltaGlyphID = SFSingleSubstF1_DeltaGlyphID(singleSubst);
                SFGlyphID substituteGlyph;
                SFGlyphTraits substituteTraits;

                substituteGlyph = (SFGlyphID)(inputGlyph + deltaGlyphID);
                substituteTraits = _SFGetGlyphTraits(textProcessor, substituteGlyph);

                /* Substitute the glyph and set its traits. */
                SFAlbumSetGlyph(album, inputIndex, substituteGlyph);
                SFAlbumSetTraits(album, inputIndex, substituteTraits);

                return SFTrue;
            }
            break;
        }

        case 2: {
            SFOffset coverageOffset = SFSingleSubstF2_CoverageOffset(singleSubst);
            SFData coverageTable = SFData_Subdata(singleSubst, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 glyphCount = SFSingleSubstF2_GlyphCount(singleSubst);

                if (coverageIndex < glyphCount) {
                    SFGlyphID substituteGlyph = SFSingleSubstF2_Substitute(singleSubst, coverageIndex);
                    SFGlyphTraits substituteTraits;

                    substituteTraits = _SFGetGlyphTraits(textProcessor, substituteGlyph);

                    /* Substitute the glyph and set its traits. */
                    SFAlbumSetGlyph(album, inputIndex, substituteGlyph);
                    SFAlbumSetTraits(album, inputIndex, substituteTraits);

                    return SFTrue;
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMultipleSubst(SFTextProcessorRef textProcessor, SFData multipleSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMultipleSubst_Format(multipleSubst);

    switch (format) {
        case 1: {
            SFOffset coverageOffset = SFMultipleSubstF1_CoverageOffset(multipleSubst);
            SFData coverageTable = SFData_Subdata(multipleSubst, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 sequenceCount = SFMultipleSubstF1_SequenceCount(multipleSubst);

                if (coverageIndex < sequenceCount) {
                    SFOffset sequenceOffset = SFMultipleSubstF1_SequenceOffset(multipleSubst, coverageIndex);
                    SFData sequenceTable = SFData_Subdata(multipleSubst, sequenceOffset);

                    return _SFApplySequenceTable(textProcessor, sequenceTable);
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplySequenceTable(SFTextProcessorRef textProcessor, SFData sequenceTable)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 glyphCount;

    glyphCount = SFSequence_GlyphCount(sequenceTable);

    if (glyphCount > 0) {
        SFGlyphID substituteGlyph;
        SFGlyphTraits substituteTraits;

        /* Get first substitute and its traits. */
        substituteGlyph = SFSequence_Substitute(sequenceTable, 0);
        substituteTraits = _SFGetGlyphTraits(textProcessor, substituteGlyph);

        /* Put substitute of first glyph and set its traits. */
        SFAlbumSetGlyph(album, inputIndex, substituteGlyph);
        SFAlbumSetTraits(album, inputIndex, substituteTraits);

        if (glyphCount != 1) {
            SFUInteger association = SFAlbumGetAssociation(album, inputIndex);
            SFUInteger subIndex;

            /* Reserve glyphs for remaining substitutes in the album. */
            SFLocatorReserveGlyphs(locator, glyphCount - 1);

            /* Initialize reserved glyphs. */
            for (subIndex = 1; subIndex < glyphCount; subIndex++) {
                SFUInteger newIndex = inputIndex + subIndex;

                /* Get substitute along with traits at current index. */
                substituteGlyph = SFSequence_Substitute(sequenceTable, subIndex);
                substituteTraits = _SFGetGlyphTraits(textProcessor, substituteGlyph);

                /* Initialize the glyph with substitute. */
                SFAlbumSetGlyph(album, newIndex, substituteGlyph);
                SFAlbumSetTraits(album, newIndex, substituteTraits);
                SFAlbumSetAssociation(album, newIndex, association);
            }

            /* Skip added elements in the locator. */
            SFLocatorJumpTo(locator, locator->index + glyphCount);
        }

        return SFTrue;
    }

    /*
     * NOTE:
     *      Latest OpenType standard prohibits the removal of glyph, if the sequence is empty.
     */

    return SFFalse;
}

static SFBoolean _SFApplyAlternateSubst(SFTextProcessorRef textProcessor, SFData alternateSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFAlternateSubst_Format(alternateSubst);

    switch (format) {
        case 1: {
            SFOffset coverageOffset = SFAlternateSubstF1_CoverageOffset(alternateSubst);
            SFData coverageTable = SFData_Subdata(alternateSubst, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 alternateSetCount = SFAlternateSubstF1_AlternateSetCount(alternateSubst);

                if (coverageIndex < alternateSetCount) {
                    SFOffset alternateSetOffset = SFAlternateSubstF1_AlternateSetOffset(alternateSubst, coverageIndex);
                    SFData alternateSetTable = SFData_Subdata(alternateSubst, alternateSetOffset);

                    return _SFApplyAlternateSetTable(textProcessor, alternateSetTable);
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyAlternateSetTable(SFTextProcessorRef textProcessor, SFData alternateSetTable)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 glyphCount;
    SFGlyphID alternateGlyph;
    SFGlyphTraits alternateTraits;

    glyphCount = SFAlternateSet_GlyphCount(alternateSetTable);
    alternateGlyph = SFAlternateSet_Alternate(alternateSetTable, 0);
    alternateTraits = _SFGetGlyphTraits(textProcessor, alternateGlyph);

    /* Substitute the glyph and set its traits. */
    SFAlbumSetGlyph(album, inputIndex, alternateGlyph);
    SFAlbumSetTraits(album, inputIndex, alternateTraits);

    return SFTrue;
}

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef textProcessor, SFData ligatureSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFLigatureSubst_Format(ligatureSubst);

    switch (format) {
        case 1: {
            SFOffset coverageOffset = SFLigatureSubstF1_CoverageOffset(ligatureSubst);
            SFData coverageTable = SFData_Subdata(ligatureSubst, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 ligSetCount = SFLigatureSubstF1_LigSetCount(ligatureSubst);

                if (coverageIndex < ligSetCount) {
                    SFOffset ligatureSetOffset = SFLigatureSubstF1_LigatureSetOffset(ligatureSubst, coverageIndex);
                    SFData ligatureSetTable = SFData_Subdata(ligatureSubst, ligatureSetOffset);

                    return _SFApplyLigatureSetTable(textProcessor, ligatureSetTable);
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyLigatureSetTable(SFTextProcessorRef textProcessor, SFData ligatureSetTable)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 ligCount;
    SFUInteger ligIndex;

    ligCount = SFLigatureSet_LigatureCount(ligatureSetTable);

    /* Match each ligature sequentially as they are ordered by preference. */
    for (ligIndex = 0; ligIndex < ligCount; ligIndex++) {
        SFOffset ligatureOffset = SFLigatureSet_LigatureOffset(ligatureSetTable, ligIndex);
        SFData ligatureTable = SFData_Subdata(ligatureSetTable, ligatureOffset);
        SFUInt16 compCount = SFLigature_CompCount(ligatureTable);
        SFUInteger *partIndexes;
        SFUInteger prevIndex;
        SFUInteger nextIndex;
        SFUInteger compIndex;

        partIndexes = SFAlbumGetTemporaryIndexArray(album, compCount);
        prevIndex = inputIndex;

        /*
         * Match all compononets starting from second one with input glyphs.
         *
         * NOTE:
         *      The loop is started from 1..CompCount, rather than 0..(CompCount - 1) so that
         *      it does not accidently overflow if the component count is zero.
         */
        for (compIndex = 1; compIndex < compCount; compIndex++) {
            nextIndex = SFLocatorGetAfter(locator, prevIndex);

            if (nextIndex != SFInvalidIndex) {
                SFGlyphID component = SFLigature_Component(ligatureTable, compIndex - 1);
                SFGlyphID glyph = SFAlbumGetGlyph(album, nextIndex);

                if (component != glyph) {
                    break;
                }
            } else {
                break;
            }

            partIndexes[compIndex] = nextIndex;
            prevIndex = nextIndex;
        }

        /* Do the substitution, if all components are matched. */
        if (compIndex == compCount) {
            SFGlyphID ligGlyph = SFLigature_LigGlyph(ligatureTable);
            SFGlyphTraits ligTraits = _SFGetGlyphTraits(textProcessor, ligGlyph);
            SFUInteger ligAssociation;

            /* Substitute the ligature glyph and set its traits. */
            SFAlbumSetGlyph(album, inputIndex, ligGlyph);
            SFAlbumSetTraits(album, inputIndex, ligTraits);

            ligAssociation = SFAlbumGetAssociation(album, inputIndex);
            prevIndex = inputIndex;

            /* Initialize component glyphs. */
            for (compIndex = 1; compIndex < compCount; compIndex++) {
                /* Get the next component. */
                nextIndex = partIndexes[compIndex];

                /* Make the glyph placeholder. */
                SFAlbumSetGlyph(album, nextIndex, 0);
                SFAlbumSetTraits(album, nextIndex, SFGlyphTraitPlaceholder);

                /* Form a cluster by setting the association of in-between glyphs. */
                for (; prevIndex <= nextIndex; prevIndex++) {
                    SFAlbumSetAssociation(album, nextIndex, ligAssociation);
                }
            }

            return SFTrue;
        }
    }

    return SFFalse;
}
