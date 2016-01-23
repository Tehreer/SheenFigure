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

#include "SFCommon.h"
#include "SFData.h"
#include "SFLocator.h"
#include "SFGSUB.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef processor, SFData singleSubst);

static SFBoolean _SFApplyMultipleSubst(SFTextProcessorRef processor, SFData multipleSubst);
static SFBoolean _SFApplySequence(SFTextProcessorRef processor, SFData sequence);

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef processor, SFData subtable);
static SFBoolean _SFApplyLigatureSet(SFTextProcessorRef processor, SFData ligatureSet);

SF_PRIVATE void _SFApplySubstitutionLookup(SFTextProcessorRef processor, SFData lookup)
{
    SFLookupType lookupType = SFLookup_LookupType(lookup);
    SFLookupFlag lookupFlag = SFLookup_LookupFlag(lookup);
    SFUInt16 subtableCount = SFLookup_SubtableCount(lookup);
    SFUInt16 subtableIndex;

    SFLocatorSetLookupFlag(&processor->_locator, lookupFlag);

    /* Apply subtables in order until one of them performs substitution. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFOffset offset = SFLookup_SubtableOffset(lookup, subtableIndex);
        SFData subtable = SFData_Subdata(lookup, offset);
        SFBoolean didSubstitute;

        didSubstitute = _SFApplySubstitutionSubtable(processor, lookupType, subtable);

        /* A subtable has performed substition, so break the loop. */
        if (didSubstitute) {
            break;
        }
    }
}

SF_PRIVATE SFBoolean _SFApplySubstitutionSubtable(SFTextProcessorRef processor, SFLookupType lookupType, SFData subtable)
{
    switch (lookupType) {
    case SFLookupTypeSingle:
        return _SFApplySingleSubst(processor, subtable);

    case SFLookupTypeMultiple:
        return _SFApplyMultipleSubst(processor, subtable);

    case SFLookupTypeAlternate:
        break;

    case SFLookupTypeLigature:
        return _SFApplyLigatureSubst(processor, subtable);

    case SFLookupTypeContext:
        break;

    case SFLookupTypeChainingContext:
        return _SFApplyChainContextSubtable(processor, subtable);

    case SFLookupTypeExtension:
        return _SFApplyExtensionSubtable(processor, subtable);

    case SFLookupTypeReverseChainingContext:
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef processor, SFData singleSubst)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFSingleSubst_Format(singleSubst);

    switch (format) {
    case 1:
        {
            SFOffset offset = SFSingleSubstF1_CoverageOffset(singleSubst);
            SFData coverage = SFData_Subdata(singleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFInt16 delta = SFSingleSubstF1_DeltaGlyphID(singleSubst);
                SFGlyphID substitute = (SFGlyphID)(inputGlyph + delta);
                SFGlyphTraits traits = _SFGetGlyphTraits(processor, substitute);

                /* Substitute the glyph and set its traits. */
                SFAlbumSetGlyph(album, inputIndex, substitute);
                SFAlbumSetTraits(album, inputIndex, traits);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFOffset offset = SFSingleSubstF2_CoverageOffset(singleSubst);
            SFData coverage = SFData_Subdata(singleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 glyphCount = SFSingleSubstF2_GlyphCount(singleSubst);

                if (coverageIndex < glyphCount) {
                    SFGlyphID substitute = SFSingleSubstF2_Substitute(singleSubst, coverageIndex);
                    SFGlyphTraits traits = _SFGetGlyphTraits(processor, substitute);

                    /* Substitute the glyph and set its traits. */
                    SFAlbumSetGlyph(album, inputIndex, substitute);
                    SFAlbumSetTraits(album, inputIndex, traits);

                    return SFTrue;
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMultipleSubst(SFTextProcessorRef processor, SFData multipleSubst)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMultipleSubst_Format(multipleSubst);

    switch (format) {
        case 1:
        {
            SFOffset offset = SFMultipleSubstF1_CoverageOffset(multipleSubst);
            SFData coverage = SFData_Subdata(multipleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 sequenceCount = SFMultipleSubstF1_SequenceCount(multipleSubst);

                if (coverageIndex < sequenceCount) {
                    SFData sequence;

                    offset = SFMultipleSubstF1_SequenceOffset(multipleSubst, coverageIndex);
                    sequence = SFData_Subdata(multipleSubst, offset);

                    return _SFApplySequence(processor, sequence);
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySequence(SFTextProcessorRef processor, SFData sequence)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 glyphCount;

    glyphCount = SFSequence_GlyphCount(sequence);

    if (glyphCount > 0) {
        SFGlyphID substitute;
        SFGlyphTraits traits;

        /* Get first substitute and its traits. */
        substitute = SFSequence_Substitute(sequence, 0);
        traits = _SFGetGlyphTraits(processor, substitute);

        /* Put substitute of first glyph and set its traits. */
        SFAlbumSetGlyph(album, inputIndex, substitute);
        SFAlbumSetTraits(album, inputIndex, traits);

        if (glyphCount != 1) {
            SFUInteger association = SFAlbumGetAssociation(album, inputIndex);
            SFUInteger subIndex;

            /* Reserve glyphs for remaining substitutes in the album. */
            SFLocatorReserveGlyphs(locator, glyphCount - 1);

            /* Initialize reserved glyphs. */
            for (subIndex = 1; subIndex < glyphCount; subIndex++) {
                SFUInteger newIndex = inputIndex + subIndex;

                /* Get substitute along with traits at current index. */
                substitute = SFSequence_Substitute(sequence, subIndex);
                traits = _SFGetGlyphTraits(processor, substitute);

                /* Initialize the glyph with substitute. */
                SFAlbumSetGlyph(album, newIndex, substitute);
                SFAlbumSetTraits(album, newIndex, traits);
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

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef processor, SFData ligatureSubst)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFLigatureSubst_Format(ligatureSubst);

    switch (format) {
        case 1:
        {
            SFOffset offset = SFLigatureSubstF1_CoverageOffset(ligatureSubst);
            SFData coverage = SFData_Subdata(ligatureSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 ligSetCount = SFLigatureSubstF1_LigSetCount(ligatureSubst);
                SFData ligatureSet;

                if (coverageIndex < ligSetCount) {
                    offset = SFLigatureSubstF1_LigatureSetOffset(ligatureSubst, coverageIndex);
                    ligatureSet = SFData_Subdata(ligatureSubst, offset);

                    return _SFApplyLigatureSet(processor, ligatureSet);
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyLigatureSet(SFTextProcessorRef processor, SFData ligatureSet)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 ligCount;
    SFUInteger ligIndex;

    ligCount = SFLigatureSet_LigatureCount(ligatureSet);

    /* Match each ligature sequentially as they are ordered by preference. */
    for (ligIndex = 0; ligIndex < ligCount; ligIndex++) {
        SFOffset offset = SFLigatureSet_LigatureOffset(ligatureSet, ligIndex);
        SFData ligature = SFData_Subdata(ligatureSet, offset);
        SFUInt16 compCount = SFLigature_CompCount(ligature);
        SFUInteger prevIndex;
        SFUInteger nextIndex;
        SFUInteger compIndex;

        prevIndex = inputIndex;

        /*
         * Match all compononets starting from second one with input glyphs.
         *
         * NOTE:
         *      The loop is started from 1..CompCount, rather than 0..(CompCount - 1) so that
         *      it does not accidently become too large if the component count is zero.
         */
        for (compIndex = 1; compIndex < compCount; compIndex++) {
            nextIndex = SFLocatorGetAfter(locator, prevIndex);

            if (inputIndex != SFInvalidIndex) {
                SFGlyphID component = SFLigature_Component(ligature, compIndex - 1);
                SFGlyphID glyph = SFAlbumGetGlyph(album, nextIndex);

                if (component != glyph) {
                    break;
                }
            } else {
                break;
            }

            prevIndex = nextIndex;
        }

        /* Do the substitution, if all components are matched. */
        if (compIndex == compCount) {
            SFGlyphID ligGlyph = SFLigature_LigGlyph(ligature);
            SFGlyphTraits traits = _SFGetGlyphTraits(processor, ligGlyph);
            SFUInteger association;

            /* Substitute the ligature glyph and set its traits. */
            SFAlbumSetGlyph(album, inputIndex, ligGlyph);
            SFAlbumSetTraits(album, inputIndex, traits);

            association = SFAlbumGetAssociation(album, inputIndex);
            prevIndex = inputIndex;

            /* Initialize component glyphs. */
            for (compIndex = 1; compIndex < compCount; compIndex++) {
                nextIndex = SFLocatorGetAfter(locator, prevIndex);

                SFAlbumSetGlyph(album, nextIndex, 0);
                SFAlbumSetTraits(album, nextIndex, SFGlyphTraitRemoved);
                SFAlbumSetAssociation(album, nextIndex, association);

                prevIndex = nextIndex;
            }

            return SFTrue;
        }
    }

    return SFFalse;
}
