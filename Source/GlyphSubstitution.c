/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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

#include "Data.h"
#include "Locator.h"
#include "SFBase.h"

#include "Tables/GSUB.h"
#include "Tables/OpenType.h"

#include "GlyphDiscovery.h"
#include "GlyphManipulation.h"
#include "GlyphSubstitution.h"
#include "TextProcessor.h"

static SFBoolean ApplySequenceTable(TextProcessorRef textProcessor, Data sequence);
static SFBoolean ApplyAlternateSetTable(TextProcessorRef textProcessor, Data alternateSet);
static SFBoolean ApplyLigatureSetTable(TextProcessorRef textProcessor, Data ligatureSet);

static SFBoolean ApplySingleSubst(TextProcessorRef textProcessor, Data singleSubst)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = SingleSubst_Format(singleSubst);

    switch (substFormat) {
    case 1: {
        Data coverage = SingleSubstF1_CoverageTable(singleSubst);
        SFInt16 delta = SingleSubstF1_DeltaGlyphID(singleSubst);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex != SFInvalidIndex) {
            SFGlyphID subGlyph = (SFGlyphID)(locGlyph + delta);
            GlyphTraits subTraits = GetGlyphTraits(textProcessor, subGlyph);

            /* Substitute the glyph and set its traits. */
            SFAlbumSetGlyph(album, locator->index, subGlyph);
            SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

            return SFTrue;
        }

        return SFFalse;
    }

    case 2: {
        Data coverage = SingleSubstF2_CoverageTable(singleSubst);
        SFUInt16 glyphCount = SingleSubstF2_GlyphCount(singleSubst);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex < glyphCount) {
            SFGlyphID subGlyph = SingleSubstF2_Substitute(singleSubst, covIndex);
            GlyphTraits subTraits;

            subTraits = GetGlyphTraits(textProcessor, subGlyph);

            /* Substitute the glyph and set its traits. */
            SFAlbumSetGlyph(album, locator->index, subGlyph);
            SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

            return SFTrue;
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyMultipleSubst(TextProcessorRef textProcessor, Data multipleSubst)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 substformat;

    substformat = MultipleSubst_Format(multipleSubst);

    switch (substformat) {
    case 1: {
        Data coverage = MultipleSubstF1_CoverageTable(multipleSubst);
        SFUInt16 seqCount = MultipleSubstF1_SequenceCount(multipleSubst);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex < seqCount) {
            Data sequence = MultipleSubstF1_SequenceTable(multipleSubst, covIndex);
            return ApplySequenceTable(textProcessor, sequence);
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplySequenceTable(TextProcessorRef textProcessor, Data sequence)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 glyphCount;

    glyphCount = Sequence_GlyphCount(sequence);

    if (glyphCount > 0) {
        /* Get first substitute and its traits. */
        SFGlyphID subGlyph = Sequence_Substitute(sequence, 0);
        GlyphTraits subTraits = GetGlyphTraits(textProcessor, subGlyph);

        /* Put substitute of first glyph and set its traits. */
        SFAlbumSetGlyph(album, locator->index, subGlyph);
        SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

        if (glyphCount != 1) {
            SFUInteger association = SFAlbumGetAssociation(album, locator->index);
            SFUInteger subIndex;

            /* Reserve glyphs for remaining substitutes in the album. */
            LocatorReserveGlyphs(locator, glyphCount - 1);

            /* Initialize reserved glyphs. */
            for (subIndex = 1; subIndex < glyphCount; subIndex++) {
                SFUInteger newIndex = locator->index + subIndex;

                /* Get substitute along with traits at current index. */
                subGlyph = Sequence_Substitute(sequence, subIndex);
                subTraits = GetGlyphTraits(textProcessor, subGlyph);

                /* Initialize the glyph with substitute. */
                SFAlbumSetGlyph(album, newIndex, subGlyph);
                SFAlbumSetAllTraits(album, newIndex, subTraits | GlyphTraitSequence);
                SFAlbumSetAssociation(album, newIndex, association);
            }

            /* Skip added elements in the locator. */
            LocatorJumpTo(locator, locator->index + glyphCount);
        }

        return SFTrue;
    }

    /*
     * NOTE:
     *      Latest OpenType standard prohibits the removal of glyph, if the sequence is empty.
     */

    return SFFalse;
}

static SFBoolean ApplyAlternateSubst(TextProcessorRef textProcessor, Data alternateSubst)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = AlternateSubst_Format(alternateSubst);

    switch (substFormat) {
    case 1: {
        Data coverage = AlternateSubstF1_CoverageTable(alternateSubst);
        SFUInt16 altSetCount = AlternateSubstF1_AlternateSetCount(alternateSubst);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex < altSetCount) {
            Data alternateSet = AlternateSubstF1_AlternateSetTable(alternateSubst, covIndex);
            return ApplyAlternateSetTable(textProcessor, alternateSet);
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyAlternateSetTable(TextProcessorRef textProcessor, Data alternateSet)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 altIndex = textProcessor->_lookupValue - 1;
    SFUInt16 glyphCount;

    glyphCount = AlternateSet_GlyphCount(alternateSet);

    /* Make sure that alternate index is valid. */
    if (altIndex < glyphCount) {
        SFGlyphID altGlyph = AlternateSet_Alternate(alternateSet, altIndex);
        GlyphTraits altTraits = GetGlyphTraits(textProcessor, altGlyph);

        /* Substitute the glyph and set its traits. */
        SFAlbumSetGlyph(album, locator->index, altGlyph);
        SFAlbumReplaceBasicTraits(album, locator->index, altTraits);

        return SFTrue;
    }

    return SFFalse;
}

static SFBoolean ApplyLigatureSubst(TextProcessorRef textProcessor, Data ligatureSubst)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = LigatureSubst_Format(ligatureSubst);

    switch (substFormat) {
    case 1: {
        Data coverage = LigatureSubstF1_CoverageTable(ligatureSubst);
        SFUInt16 ligSetCount = LigatureSubstF1_LigSetCount(ligatureSubst);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex < ligSetCount) {
            Data ligatureSet = LigatureSubstF1_LigatureSetTable(ligatureSubst, covIndex);
            return ApplyLigatureSetTable(textProcessor, ligatureSet);
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyLigatureSetTable(TextProcessorRef textProcessor, Data ligatureSet)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 ligCount;
    SFUInteger ligIndex;

    ligCount = LigatureSet_LigatureCount(ligatureSet);

    /* Match each ligature sequentially as they are ordered by preference. */
    for (ligIndex = 0; ligIndex < ligCount; ligIndex++) {
        Data ligature = LigatureSet_LigatureTable(ligatureSet, ligIndex);
        SFUInt16 compCount = Ligature_CompCount(ligature);
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
            nextIndex = LocatorGetAfter(locator, prevIndex, SFTrue);

            if (nextIndex != SFInvalidIndex) {
                SFGlyphID component = Ligature_Component(ligature, compIndex - 1);
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
            SFGlyphID ligGlyph = Ligature_LigGlyph(ligature);
            GlyphTraits ligTraits = GetGlyphTraits(textProcessor, ligGlyph);
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
                SFAlbumReplaceBasicTraits(album, nextIndex, GlyphTraitPlaceholder);

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

SF_PRIVATE SFBoolean ApplySubstitutionSubtable(TextProcessorRef textProcessor, LookupType lookupType, Data subtable)
{
    switch (lookupType) {
    case LookupTypeSingle:
        return ApplySingleSubst(textProcessor, subtable);

    case LookupTypeMultiple:
        return ApplyMultipleSubst(textProcessor, subtable);

    case LookupTypeAlternate:
        return ApplyAlternateSubst(textProcessor, subtable);

    case LookupTypeLigature:
        return ApplyLigatureSubst(textProcessor, subtable);

    case LookupTypeContext:
        return ApplyContextSubtable(textProcessor, subtable);

    case LookupTypeChainingContext:
        return ApplyChainContextSubtable(textProcessor, subtable);

    case LookupTypeExtension:
        return ApplyExtensionSubtable(textProcessor, subtable);

    case LookupTypeReverseChainingContext:
        return ApplyReverseChainSubst(textProcessor, subtable);

    default:
        /* Invalid lookup type. */
        return SFFalse;
    }
}
