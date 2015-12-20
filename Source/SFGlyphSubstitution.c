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

SF_PRIVATE void _SFApplyGSUBLookup(SFTextProcessorRef processor, SFData lookup)
{
    SFLookupType lookupType = SF_LOOKUP__LOOKUP_TYPE(lookup);
    SFLookupFlag lookupFlag = SF_LOOKUP__LOOKUP_FLAG(lookup);
    SFUInt16 subtableCount = SF_LOOKUP__SUB_TABLE_COUNT(lookup);
    SFUInt16 subtableIndex;

    /* Set lookup flag in shaping engine. */
    processor->_lookupFlag = lookupFlag;

    /* Apply subtables in order until one of them performs substitution. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFOffset offset = SF_LOOKUP__SUB_TABLE(lookup, subtableIndex);
        SFData subtable = SF_DATA__SUBDATA(lookup, offset);
        SFBoolean didSubstitute;

        didSubstitute = _SFApplySubst(processor, lookupType, subtable);

        /* A subtable has performed substition, so break the loop. */
        if (didSubstitute) {
            break;
        }
    }
}

SF_PRIVATE SFBoolean _SFApplySubst(SFTextProcessorRef processor, SFLookupType lookupType, SFData subtable)
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
        break;

    case SFLookupTypeExtension:
        return _SFApplyExtensionSubtable(processor, subtable);

    case SFLookupTypeReverseChainingContext:
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySingleSubst(SFTextProcessorRef processor, SFData singleSubst)
{
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFCollectionGetGlyph(collection, inputIndex);
    SFUInt16 format;

    format = SF_SINGLE_SUBST_FORMAT(singleSubst);

    switch (format) {
    case 1:
        {
            SFOffset offset = SF_SINGLE_SUBST_F1__COVERAGE(singleSubst);
            SFData coverage = SF_DATA__SUBDATA(singleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFInt16 delta = SF_SINGLE_SUBST_F1__DELTA_GLYPH_ID(singleSubst);
                SFGlyphID substitute = (SFGlyphID)(inputGlyph + delta);
                SFGlyphTrait traits = _SFGetGlyphTrait(processor, substitute);

                /* Substitute the glyph and set its traits. */
                SFCollectionSetGlyph(collection, inputIndex, substitute);
                SFCollectionSetTraits(collection, inputIndex, traits);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFOffset offset = SF_SINGLE_SUBST_F2__COVERAGE(singleSubst);
            SFData coverage = SF_DATA__SUBDATA(singleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 glyphCount = SF_SINGLE_SUBST_F2__GLYPH_COUNT(singleSubst);

                if (coverageIndex < glyphCount) {
                    SFGlyphID substitute = SF_SINGLE_SUBST_F2__SUBSTITUTE(singleSubst, coverageIndex);
                    SFGlyphTrait traits = _SFGetGlyphTrait(processor, substitute);

                    /* Substitute the glyph and set its traits. */
                    SFCollectionSetGlyph(collection, inputIndex, substitute);
                    SFCollectionSetTraits(collection, inputIndex, traits);

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
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFCollectionGetGlyph(collection, inputIndex);
    SFUInt16 format;

    format = SF_MULTIPLE_SUBST_FORMAT(multipleSubst);

    switch (format) {
        case 1:
        {
            SFOffset offset = SF_MULTIPLE_SUBST_F1__COVERAGE(multipleSubst);
            SFData coverage = SF_DATA__SUBDATA(multipleSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 sequenceCount = SF_MULTIPLE_SUBST_F1__SEQUENCE_COUNT(multipleSubst);

                if (coverageIndex < sequenceCount) {
                    SFData sequence;

                    offset = SF_MULTIPLE_SUBST_F1__SEQUENCE(multipleSubst, inputIndex);
                    sequence = SF_DATA__SUBDATA(multipleSubst, offset);

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
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 glyphCount;

    glyphCount = SF_SEQUENCE__GLYPH_COUNT(sequence);

    if (glyphCount > 0) {
        SFGlyphID substitute;
        SFGlyphTrait traits;

        /* Get first substitute and its traits. */
        substitute = SF_SEQUENCE__SUBSTITUTE(sequence, 0);
        traits = _SFGetGlyphTrait(processor, substitute);

        /* Put substitute of first glyph and set its traits. */
        SFCollectionSetGlyph(collection, inputIndex, substitute);
        SFCollectionSetTraits(collection, inputIndex, traits);

        if (glyphCount != 1) {
            SFUInteger association = SFCollectionGetAssociation(collection, inputIndex);
            SFUInteger subIndex;

            /* Reserve glyphs for remaining substitutes in the collection. */
            SFCollectionReserveElements(collection, inputIndex + 1, glyphCount - 1);

            /* Initialize reserved glyphs. */
            for (subIndex = 1; subIndex < glyphCount; subIndex++) {
                SFUInteger newIndex = inputIndex + subIndex;

                /* Get substitute along with traits at current index. */
                substitute = SF_SEQUENCE__SUBSTITUTE(sequence, subIndex);
                traits = _SFGetGlyphTrait(processor, substitute);

                /* Initialize the glyph with substitute. */
                SFCollectionSetGlyph(collection, newIndex, substitute);
                SFCollectionSetTraits(collection, newIndex, traits);
                SFCollectionSetAssociation(collection, newIndex, association);
            }

            /* Skip added elements in the locator. */
            SFLocatorSkip(locator, glyphCount - 1);
        }

        return SFTrue;
    }

    /*
     * NOTE:
     *      Latest OpenType standard prohibits the removal of glyph, if the
     *      sequence is empty.
     */

    return SFFalse;
}

static SFBoolean _SFApplyLigatureSubst(SFTextProcessorRef processor, SFData ligatureSubst)
{
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFCollectionGetGlyph(collection, inputIndex);
    SFUInt16 format;

    format = SF_LIGATURE_SUBST_FORMAT(ligatureSubst);

    switch (format) {
        case 1:
        {
            SFOffset offset = SF_LIGATURE_SUBST_F1__COVERAGE(ligatureSubst);
            SFData coverage = SF_DATA__SUBDATA(ligatureSubst, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 ligSetCount = SF_LIGATURE_SUBST_F1__LIG_SET_COUNT(ligatureSubst);
                SFData ligatureSet;

                if (coverageIndex < ligSetCount) {
                    offset = SF_LIGATURE_SUBST_F1__LIGATURE_SET(ligatureSubst, coverageIndex);
                    ligatureSet = SF_DATA__SUBDATA(ligatureSubst, offset);

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
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 ligCount;
    SFUInteger ligIndex;

    ligCount = SF_LIGATURE_SET__LIGATURE_COUNT(ligatureSet);

    /* Match each ligature sequentially as they are ordered by preference. */
    for (ligIndex = 0; ligIndex < ligCount; ligIndex++) {
        SFOffset offset = SF_LIGATURE_SET__LIGATURE(ligatureSet, ligIndex);
        SFData ligature = SF_DATA__SUBDATA(ligatureSet, offset);
        SFUInt16 compCount = SF_LIGATURE__COMP_COUNT(ligature);
        SFUInteger prevIndex;
        SFUInteger nextIndex;
        SFUInteger compIndex;

        prevIndex = inputIndex;

        /* Match all compononets starting from second one with input glyphs. */
        for (compIndex = 1; compIndex < compCount; compIndex++) {
            nextIndex = SFLocatorGetAfter(locator, prevIndex);

            if (inputIndex != SFInvalidIndex) {
                SFGlyphID component = SF_LIGATURE__COMPONENT(ligature, compIndex);
                SFGlyphID glyph = SFCollectionGetGlyph(collection, nextIndex);

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
            SFGlyphID ligGlyph = SF_LIGATURE__LIG_GLYPH(ligature);
            SFGlyphTrait traits = _SFGetGlyphTrait(processor, ligGlyph);
            SFUInteger association;

            /* Substitute the ligature glyph and set its traits. */
            SFCollectionSetGlyph(collection, inputIndex, ligGlyph);
            SFCollectionSetTraits(collection, inputIndex, traits);

            association = SFCollectionGetAssociation(collection, inputIndex);
            prevIndex = inputIndex;

            /* Initialize component glyphs. */
            for (compIndex = 1; compIndex < compCount; compIndex++) {
                nextIndex = SFLocatorGetAfter(locator, prevIndex);

                SFCollectionSetGlyph(collection, nextIndex, 0);
                SFCollectionSetTraits(collection, nextIndex, SFGlyphTraitRemoved);
                SFCollectionSetAssociation(collection, nextIndex, association);

                prevIndex = nextIndex;
            }

            return SFTrue;
        }
    }

    return SFFalse;
}
