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

#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGSUB.h"
#include "SFLocator.h"
#include "SFOpenType.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef textProcessor, SFData singleSubst);

static SFBoolean _SFApplyMultipleSubst(SFTextProcessorRef textProcessor, SFData multipleSubst);
static SFBoolean _SFApplySequenceTable(SFTextProcessorRef textProcessor, SFData sequence);

static SFBoolean _SFApplyAlternateSubst(SFTextProcessorRef textProcessor, SFData alternateSubst);
static SFBoolean _SFApplyAlternateSetTable(SFTextProcessorRef textProcessor, SFData alternateSet);

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef textProcessor, SFData ligatureSubst);
static SFBoolean _SFApplyLigatureSetTable(SFTextProcessorRef textProcessor, SFData ligatureSet);

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
            return _SFApplyReverseChainSubst(textProcessor, subtable);
    }

    return SFFalse;
}

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef textProcessor, SFData singleSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = SFSingleSubst_Format(singleSubst);

    switch (substFormat) {
        case 1: {
            SFData coverage = SFSingleSubstF1_CoverageTable(singleSubst);
            SFInt16 delta = SFSingleSubstF1_DeltaGlyphID(singleSubst);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SFOpenTypeSearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                SFGlyphID subGlyph = (SFGlyphID)(locGlyph + delta);
                SFGlyphTraits subTraits = _SFGetGlyphTraits(textProcessor, subGlyph);

                /* Substitute the glyph and set its traits. */
                SFAlbumSetGlyph(album, locator->index, subGlyph);
                SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

                return SFTrue;
            }
            break;
        }

        case 2: {
            SFData coverage = SFSingleSubstF2_CoverageTable(singleSubst);
            SFUInt16 glyphCount = SFSingleSubstF2_GlyphCount(singleSubst);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SFOpenTypeSearchCoverageIndex(coverage, locGlyph);

            if (covIndex < glyphCount) {
                SFGlyphID subGlyph = SFSingleSubstF2_Substitute(singleSubst, covIndex);
                SFGlyphTraits subTraits;

                subTraits = _SFGetGlyphTraits(textProcessor, subGlyph);

                /* Substitute the glyph and set its traits. */
                SFAlbumSetGlyph(album, locator->index, subGlyph);
                SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

                return SFTrue;
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
    SFUInt16 substformat;

    substformat = SFMultipleSubst_Format(multipleSubst);

    switch (substformat) {
        case 1: {
            SFData coverage = SFMultipleSubstF1_CoverageTable(multipleSubst);
            SFUInt16 seqCount = SFMultipleSubstF1_SequenceCount(multipleSubst);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SFOpenTypeSearchCoverageIndex(coverage, locGlyph);

            if (covIndex < seqCount) {
                SFData sequence = SFMultipleSubstF1_SequenceTable(multipleSubst, covIndex);
                return _SFApplySequenceTable(textProcessor, sequence);
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplySequenceTable(SFTextProcessorRef textProcessor, SFData sequence)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInt16 glyphCount;

    glyphCount = SFSequence_GlyphCount(sequence);

    if (glyphCount > 0) {
        /* Get first substitute and its traits. */
        SFGlyphID subGlyph = SFSequence_Substitute(sequence, 0);
        SFGlyphTraits subTraits = _SFGetGlyphTraits(textProcessor, subGlyph);

        /* Put substitute of first glyph and set its traits. */
        SFAlbumSetGlyph(album, locator->index, subGlyph);
        SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

        if (glyphCount != 1) {
            SFUInteger association = SFAlbumGetAssociation(album, locator->index);
            SFUInteger subIndex;

            /* Reserve glyphs for remaining substitutes in the album. */
            SFLocatorReserveGlyphs(locator, glyphCount - 1);

            /* Initialize reserved glyphs. */
            for (subIndex = 1; subIndex < glyphCount; subIndex++) {
                SFUInteger newIndex = locator->index + subIndex;

                /* Get substitute along with traits at current index. */
                subGlyph = SFSequence_Substitute(sequence, subIndex);
                subTraits = _SFGetGlyphTraits(textProcessor, subGlyph);

                /* Initialize the glyph with substitute. */
                SFAlbumSetGlyph(album, newIndex, subGlyph);
                SFAlbumSetAllTraits(album, newIndex, subTraits | SFGlyphTraitSequence);
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
    SFUInt16 substFormat;

    substFormat = SFAlternateSubst_Format(alternateSubst);

    switch (substFormat) {
        case 1: {
            SFData coverage = SFAlternateSubstF1_CoverageTable(alternateSubst);
            SFUInt16 altSetCount = SFAlternateSubstF1_AlternateSetCount(alternateSubst);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SFOpenTypeSearchCoverageIndex(coverage, locGlyph);

            if (covIndex < altSetCount) {
                SFData alternateSet = SFAlternateSubstF1_AlternateSetTable(alternateSubst, covIndex);
                return _SFApplyAlternateSetTable(textProcessor, alternateSet);
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyAlternateSetTable(SFTextProcessorRef textProcessor, SFData alternateSet)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInt16 glyphCount;
    SFGlyphID altGlyph;
    SFGlyphTraits altTraits;

    glyphCount = SFAlternateSet_GlyphCount(alternateSet);
    altGlyph = SFAlternateSet_Alternate(alternateSet, 0);
    altTraits = _SFGetGlyphTraits(textProcessor, altGlyph);

    /* Substitute the glyph and set its traits. */
    SFAlbumSetGlyph(album, locator->index, altGlyph);
    SFAlbumReplaceBasicTraits(album, locator->index, altTraits);

    return SFTrue;
}

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef textProcessor, SFData ligatureSubst)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = SFLigatureSubst_Format(ligatureSubst);

    switch (substFormat) {
        case 1: {
            SFData coverage = SFLigatureSubstF1_CoverageTable(ligatureSubst);
            SFUInt16 ligSetCount = SFLigatureSubstF1_LigSetCount(ligatureSubst);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SFOpenTypeSearchCoverageIndex(coverage, locGlyph);

            if (covIndex < ligSetCount) {
                SFData ligatureSet = SFLigatureSubstF1_LigatureSetTable(ligatureSubst, covIndex);
                return _SFApplyLigatureSetTable(textProcessor, ligatureSet);
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyLigatureSetTable(SFTextProcessorRef textProcessor, SFData ligatureSet)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInt16 ligCount;
    SFUInteger ligIndex;

    ligCount = SFLigatureSet_LigatureCount(ligatureSet);

    /* Match each ligature sequentially as they are ordered by preference. */
    for (ligIndex = 0; ligIndex < ligCount; ligIndex++) {
        SFData ligature = SFLigatureSet_LigatureTable(ligatureSet, ligIndex);
        SFUInt16 compCount = SFLigature_CompCount(ligature);
        SFUInteger *partIndexes;
        SFUInteger prevIndex;
        SFUInteger nextIndex;
        SFUInteger compIndex;

        partIndexes = SFAlbumGetTemporaryIndexArray(album, compCount);
        prevIndex = locator->index;

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
                SFGlyphID component = SFLigature_Component(ligature, compIndex - 1);
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
            SFGlyphID ligGlyph = SFLigature_LigGlyph(ligature);
            SFGlyphTraits ligTraits = _SFGetGlyphTraits(textProcessor, ligGlyph);
            SFUInteger ligAssociation;

            /* Substitute the ligature glyph and set its traits. */
            SFAlbumSetGlyph(album, locator->index, ligGlyph);
            SFAlbumReplaceBasicTraits(album, locator->index, ligTraits);

            ligAssociation = SFAlbumGetAssociation(album, locator->index);
            prevIndex = locator->index;

            /* Initialize component glyphs. */
            for (compIndex = 1; compIndex < compCount; compIndex++) {
                /* Get the next component. */
                nextIndex = partIndexes[compIndex];

                /* Make the glyph placeholder. */
                SFAlbumSetGlyph(album, nextIndex, 0);
                SFAlbumReplaceBasicTraits(album, nextIndex, SFGlyphTraitPlaceholder);

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
