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
#include "SFLocator.h"
#include "SFGPOS.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFTextProcessor.h"

static SFBoolean _SFApplySinglePos(SFTextProcessorRef processor, SFData singlePos);

static SFBoolean _SFApplyPairPos(SFTextProcessorRef processor, SFData pairPos);
static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);
static SFBoolean _SFApplyPairPosF2(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);

static SFData _SFSearchPairRecord(SFData pairSet, SFUInteger recordSize, SFGlyph glyph);
static void _SFApplyValueRecord(SFTextProcessorRef processor, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex);

static SFPoint _SFConvertAnchorToPoint(SFData anchor);
static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyph inputGlyph, SFData *outExitAnchor, SFData *outEntryAnchor);
static SFBoolean _SFApplyCursivePos(SFTextProcessorRef processor, SFData cursivePos);

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef processor);
static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef processor, SFData markBasePos);
static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef processor, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex);

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent);
static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos);
static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef processor, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent);

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef processor);
static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos);
static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef processor, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index);

static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInteger *outClass);

SF_PRIVATE void _SFApplyGPOSLookup(SFTextProcessorRef processor, SFData lookup)
{
    SFLookupType lookupType = SF_LOOKUP__LOOKUP_TYPE(lookup);
    SFLookupFlag lookupFlag = SF_LOOKUP__LOOKUP_FLAG(lookup);
    SFUInt16 subtableCount = SF_LOOKUP__SUB_TABLE_COUNT(lookup);
    SFUInteger subtableIndex;

    /* Apply subtables in order until one of them performs positioning. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFOffset subtableOffset = SF_LOOKUP__SUB_TABLE(lookup, subtableIndex);
        SFData subtable = SF_DATA__SUBDATA(lookup, subtableOffset);
        SFBoolean didPosition;

        didPosition = _SFApplyPos(processor, lookupType, subtable);

        /* A subtable has performed positioning, so break the loop. */
        if (didPosition) {
            break;
        }
    }
}

SF_PRIVATE SFBoolean _SFApplyPos(SFTextProcessorRef processor, SFLookupType lookupType, SFData subtable)
{
    switch (lookupType) {
    case SFLookupTypeSingleAdjustment:
        return _SFApplySinglePos(processor, subtable);

    case SFLookupTypePairAdjustment:
        return _SFApplyPairPos(processor, subtable);

    case SFLookupTypeCursiveAttachment:
        return _SFApplyCursivePos(processor, subtable);

    case SFLookupTypeMarkToBaseAttachment:
        return _SFApplyMarkToBasePos(processor, subtable);

    case SFLookupTypeMarkToLigatureAttachment:
        return _SFApplyMarkToLigPos(processor, subtable);

    case SFLookupTypeMarkToMarkAttachment:
        return _SFApplyMarkToMarkPos(processor, subtable);

    case SFLookupTypeContextPositioning:
        break;

    case SFLookupTypeChainedContextPositioning:
        break;

    case SFLookupTypeExtensionPositioning:
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySinglePos(SFTextProcessorRef processor, SFData singlePos)
{
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyph inputGlyph = collection->glyphArray[inputIndex];
    SFUInt16 format;

    format = SF_SINGLE_POS_FORMAT(singlePos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SF_SINGLE_POS_F1__COVERAGE(singlePos);
            SFData coverage = SF_DATA__SUBDATA(singlePos, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 valueFormat = SF_SINGLE_POS_F1__VALUE_FORMAT(singlePos);
                SFData valueRecord = SF_SINGLE_POS_F1__VALUE_RECORD(singlePos);

                _SFApplyValueRecord(processor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFOffset offset = SF_SINGLE_POS_F2__COVERAGE(singlePos);
            SFData coverage = SF_DATA__SUBDATA(singlePos, offset);
            SFUInt16 valueFormat = SF_SINGLE_POS_F2__VALUE_FORMAT(singlePos);
            SFUInt16 valueCount = SF_SINGLE_POS_F2__VALUE_COUNT(singlePos);
            SFUInteger valueIndex;

            valueIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (valueIndex < valueCount) {
                SFUInteger valueSize = SF_VALUE_RECORD__SIZE(valueFormat);
                SFData valueRecord = SF_SINGLE_POS_F2__VALUE_RECORD(singlePos, valueIndex, valueSize);

                _SFApplyValueRecord(processor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyPairPos(SFTextProcessorRef processor, SFData pairPos)
{
    SFLocatorRef locator = processor->_locator;
    SFBoolean didPosition = SFFalse;
    SFBoolean shouldSkip = SFFalse;
    SFUInteger firstIndex;
    SFUInteger secondIndex;

    firstIndex = locator->index;
    secondIndex = SFLocatorGetAfter(locator, firstIndex);

    /* Proceed only if pair glyph is available. */
    if (secondIndex != SFInvalidIndex) {
        SFUInt16 format = SF_PAIR_POS_FORMAT(pairPos);

        switch (format) {
        case 1:
            didPosition = _SFApplyPairPosF1(processor, pairPos, firstIndex, secondIndex, &shouldSkip);
            break;

        case 2:
            didPosition = _SFApplyPairPosF2(processor, pairPos, firstIndex, secondIndex, &shouldSkip);
            break;
        }
    }

    if (shouldSkip) {
        SFLocatorJumpTo(locator, secondIndex + 1);
    } else {
        /*
         * Whether pair positioning is applied or not, next element will always
         * be the one found above. So jump the locator to it.
         */
        SFLocatorJumpTo(locator, secondIndex);
    }

    return didPosition;
}

static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFCollectionRef collection = processor->_collection;
    SFGlyph firstGlyph = collection->glyphArray[firstIndex];
    SFGlyph secondGlyph = collection->glyphArray[secondIndex];
    SFOffset offset;
    SFData coverage;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    offset = SF_PAIR_POS_F1__COVERAGE(pairPos);
    coverage = SF_DATA__SUBDATA(pairPos, offset);
    coverageIndex = _SFSearchCoverageIndex(coverage, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SF_PAIR_POS_F1__VALUE_FORMAT1(pairPos);
        SFUInt16 valueFormat2 = SF_PAIR_POS_F1__VALUE_FORMAT2(pairPos);
        SFUInt16 pairSetCount = SF_PAIR_POS_F1__PAIR_SET_COUNT(pairPos);
        SFUInteger value1Size = SF_VALUE_RECORD__SIZE(valueFormat1);
        SFUInteger value2Size = SF_VALUE_RECORD__SIZE(valueFormat2);
        SFUInteger recordSize = SF_PAIR_VALUE_RECORD__SIZE(value1Size, value2Size);
        SFUInteger pairSetIndex;

        for (pairSetIndex = 0; pairSetIndex < pairSetCount; pairSetIndex++) {
            SFData pairSet;
            SFData pairRecord;

            offset = SF_PAIR_POS_F1__PAIR_SET(pairPos, pairSetIndex);
            pairSet = SF_DATA__SUBDATA(pairPos, offset);
            pairRecord = _SFSearchPairRecord(pairSet, recordSize, secondGlyph);

            if (pairRecord) {
                if (value1Size) {
                    SFData value1 = SF_PAIR_VALUE_RECORD__VALUE1(pairRecord);
                    _SFApplyValueRecord(processor, value1, valueFormat1, firstIndex);
                }

                if (value2Size) {
                    SFData value2 = SF_PAIR_VALUE_RECORD__VALUE2(pairRecord, value1Size);
                    _SFApplyValueRecord(processor, value2, valueFormat2, secondIndex);

                    /*
                     * Pair element should be skipped only if the value record
                     * for the second glyph is AVAILABLE.
                     */
                    *outShouldSkip = SFTrue;
                }

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyPairPosF2(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFCollectionRef collection = processor->_collection;
    SFGlyph firstGlyph = collection->glyphArray[firstIndex];
    SFGlyph secondGlyph = collection->glyphArray[secondIndex];
    SFOffset offset;
    SFData coverage;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    offset = SF_PAIR_POS_F2__COVERAGE(pairPos);
    coverage = SF_DATA__SUBDATA(pairPos, offset);
    coverageIndex = _SFSearchCoverageIndex(coverage, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SF_PAIR_POS_F2__VALUE_FORMAT1(pairPos);
        SFUInt16 valueFormat2 = SF_PAIR_POS_F2__VALUE_FORMAT2(pairPos);
        SFOffset classDef1Offset = SF_PAIR_POS_F2__CLASS_DEF1(pairPos);
        SFOffset classDef2Offset = SF_PAIR_POS_F2__CLASS_DEF2(pairPos);
        SFUInt16 class1Count = SF_PAIR_POS_F2__CLASS1_COUNT(pairPos);
        SFUInt16 class2Count = SF_PAIR_POS_F2__CLASS2_COUNT(pairPos);
        SFData classDef1 = SF_DATA__SUBDATA(pairPos, classDef1Offset);
        SFData classDef2 = SF_DATA__SUBDATA(pairPos, classDef2Offset);
        SFUInt16 class1Value = _SFSearchGlyphClass(classDef1, firstGlyph);
        SFUInt16 class2Value = _SFSearchGlyphClass(classDef2, secondGlyph);

        if (class1Value < class1Count && class2Value < class2Count) {
            SFUInteger value1Size = SF_VALUE_RECORD__SIZE(valueFormat1);
            SFUInteger value2Size = SF_VALUE_RECORD__SIZE(valueFormat2);
            SFUInteger class2Size = SF_CLASS2_RECORD__SIZE(value1Size, value2Size);
            SFUInteger class1Size = SF_CLASS1_RECORD__SIZE(class2Count, class2Size);
            SFData class1Record = SF_PAIR_POS_F2__CLASS1_RECORD(pairPos, class1Value, class1Size);
            SFData class2Record = SF_CLASS1_RECORD__CLASS2_RECORD(class1Record, class2Value, class2Size);

            if (value1Size) {
                SFData value1 = SF_CLASS2_RECORD__VALUE1(class2Record);
                _SFApplyValueRecord(processor, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                SFData value2 = SF_CLASS2_RECORD__VALUE2(class2Record, value1Size);
                _SFApplyValueRecord(processor, value2, valueFormat2, secondIndex);

                /*
                 * Pair element should be skipped only if the value record for
                 * the second glyph is AVAILABLE.
                 */
                *outShouldSkip = SFTrue;
            }

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFData _SFSearchPairRecord(SFData pairSet, SFUInteger recordSize, SFGlyph glyph)
{
    SFUInt16 valueCount = SF_PAIR_SET__PAIR_VALUE_COUNT(pairSet);
    SFData recordArray = SF_PAIR_SET__PAIR_VALUE_RECORD_ARRAY(pairSet);

    if (valueCount > 0) {
        SFUInteger min = 0;
        SFUInteger max = valueCount - 1;

        while (min <= max) {
            SFUInteger mid = (min + max) >> 1;
            SFData valurRecord = SF_PAIR_SET__PAIR_VALUE_RECORD(recordArray, mid, recordSize);
            SFGlyph secondGlyph = SF_PAIR_VALUE_RECORD__SECOND_GLYPH(valurRecord);

            if (secondGlyph < glyph) {
                min = mid + 1;
            } else if (secondGlyph > glyph) {
                max = mid - 1;
            } else {
                return valurRecord;
            }
        }
    }

    return NULL;
}

static void _SFApplyValueRecord(SFTextProcessorRef processor, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFCollectionRef collection = processor->_collection;
    SFOffset offset;

    offset = 0;

    if (SF_VALUE_FORMAT__X_PLACEMENT(valueFormat)) {
        collection->positionArray[inputIndex].x += (SFInt16)SF_VALUE_RECORD__NEXT_VALUE(valueRecord, offset);
    }

    if (SF_VALUE_FORMAT__Y_PLACEMENT(valueFormat)) {
        collection->positionArray[inputIndex].y += (SFInt16)SF_VALUE_RECORD__NEXT_VALUE(valueRecord, offset);
    }

    switch (processor->_direction) {
    case SFDirectionHorizontal:
        if (SF_VALUE_FORMAT__X_ADVANCE(valueFormat)) {
            collection->advanceArray[inputIndex] += (SFInt16)SF_VALUE_RECORD__NEXT_VALUE(valueRecord, offset);
        }
        break;
    }

    /* NOTE:
    *      There is no need to support device tables as font size is asumed to
    *      be exactly same as the units per em of the font.
    */
}

static SFPoint _SFConvertAnchorToPoint(SFData anchor)
{
    SFUInt16 format;
    SFPoint point;

    format = SF_ANCHOR_FORMAT(anchor);

    switch (format) {
    case 1:
        point.x = SF_ANCHOR_F1__X_COORDINATE(anchor);
        point.y = SF_ANCHOR_F2__Y_COORDINATE(anchor);
        break;

        /* TODO: Support remaining anchor formats. */

    default:
        point.x = 0;
        point.y = 0;
        break;
    }

    return point;
}

static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyph inputGlyph, SFData *outExitAnchor, SFData *outEntryAnchor)
{
    SFOffset offset = SF_CURSIVE_POS__COVERAGE(cursivePos);
    SFData coverage = SF_DATA__SUBDATA(cursivePos, offset);
    SFUInt16 entryExitCount = SF_CURSIVE_POS__ENTRY_EXIT_COUNT(cursivePos);
    SFUInteger entryExitIndex;

    *outExitAnchor = NULL;
    *outEntryAnchor = NULL;

    entryExitIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

    if (entryExitIndex < entryExitCount) {
        SFData entryExitRecord = SF_CURSIVE_POS__ENTRY_EXIT_RECORD(cursivePos, entryExitIndex);
        SFOffset exitAnchorOffset = SF_ENTRY_EXIT_RECORD__EXIT_ANCHOR(entryExitRecord);
        SFOffset entryAnchorOffset = SF_ENTRY_EXIT_RECORD__ENTRY_ANCHOR(entryExitRecord);

        if (exitAnchorOffset) {
            *outExitAnchor = SF_DATA__SUBDATA(cursivePos, exitAnchorOffset);
        }

        if (entryAnchorOffset) {
            *outEntryAnchor = SF_DATA__SUBDATA(cursivePos, entryAnchorOffset);
        }
    }
}

static SFBoolean _SFApplyCursivePos(SFTextProcessorRef processor, SFData cursivePos)
{
    SFUInt16 format = SF_CURSIVE_POS__FORMAT(cursivePos);

    switch (format) {
    case 1:
        return _SFApplyCursivePosF1(processor, cursivePos);
    }

    return SFFalse;
}

static SFBoolean _SFApplyCursivePosF1(SFTextProcessorRef processor, SFData cursivePos)
{
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyph inputGlyph = collection->glyphArray[inputIndex];
    SFData *inputExitAnchor;
    SFData *inputEntryAnchor;

    _SFSearchCursiveAnchors(cursivePos, inputGlyph, inputExitAnchor, inputEntryAnchor);

    /* Proceed only if exit anchor of input glyph exists. */
    if (inputExitAnchor) {
        SFUInteger nextIndex = SFLocatorGetAfter(locator, inputIndex);

        if (nextIndex != SFInvalidIndex) {
            SFGlyph nextGlyph = collection->glyphArray[nextIndex];
            SFData *nextExitAnchor;
            SFData *nextEntryAnchor;

            _SFSearchCursiveAnchors(cursivePos, nextGlyph, nextExitAnchor, nextEntryAnchor);

            /* Proceed only if entry anchor of next glyph exists. */
            if (nextEntryAnchor) {
                SFPoint exitPoint = _SFConvertAnchorToPoint(inputExitAnchor);
                SFPoint entryPoint = _SFConvertAnchorToPoint(nextEntryAnchor);
                SFInteger oldX;

                switch (processor->_direction) {
                case SFDirectionHorizontal:
                    /*
                     * The exit glyph must be connected with entry glyph (which
                     * will replace its old advance) while preserving its x so
                     * that it is placed at intended location.
                     */
                    collection->advanceArray[inputIndex] = collection->positionArray[inputIndex].x + exitPoint.x;
                    /*
                     * The entry glyph must be connected with exit glyph (which
                     * will replace its old x) while preserving its advance so
                     * that the pen moves to the intended location.
                     */
                    oldX = collection->positionArray[nextIndex].x;
                    collection->positionArray[nextIndex].x = -entryPoint.x;
                    collection->advanceArray[nextIndex] -= oldX + entryPoint.x;

                    /* Connect entry glyph with exit glyph vertically. */
                    collection->positionArray[nextIndex].y = exitPoint.y - entryPoint.y;

                    break;
                }

                /* Set traits of both elements. */
                if (processor->_lookupFlag & SFLookupFlagRightToLeft) {
                    collection->detailArray[inputIndex].traits |= SFGlyphTraitRightToLeft;
                    collection->detailArray[inputIndex].offset = nextIndex - inputIndex;

                    collection->detailArray[nextIndex].traits |= SFGlyphTraitRightToLeft;
                }

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef processor)
{
    SFLocatorRef locator = processor->_locator;
    SFLookupFlag lookupFlag = processor->_lookupFlag | SFLookupFlagIgnoreMarks;

    /*
    * NOTE:
    *      Previous non-mark glyph is assumed as a base glyph. It is not
    *      necessary to confirm whether that is actually a base glyph or not.
    */
    return SFLocatorGetBefore(locator, lookupFlag, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef processor, SFData markBasePos)
{
    SFCollectionRef collection = processor->_collection;
    SFUInteger inputIndex = processor->_locator->index;
    SFGlyph inputGlyph = collection->glyphArray[inputIndex];
    SFUInt16 format;

    format = SF_MARK_BASE_POS__FORMAT(markBasePos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SF_MARK_BASE_POS__MARK_COVERAGE(markBasePos);
            SFData markCoverage = SF_DATA__SUBDATA(markBasePos, offset);
            SFUInteger markIndex;

            markIndex = _SFSearchCoverageIndex(markCoverage, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousBaseGlyphIndex(processor);

                /* Proceed only if there is a previous base glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData baseCoverage;
                    SFUInteger baseIndex;

                    offset = SF_MARK_BASE_POS__BASE_COVERAGE(markBasePos);
                    baseCoverage = SF_DATA__SUBDATA(markBasePos, offset);
                    baseIndex = _SFSearchCoverageIndex(baseCoverage, prevGlyph);

                    if (baseIndex != SFInvalidIndex) {
                        return _SFApplyMarkToBaseArrays(processor, markBasePos, markIndex, baseIndex);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef processor, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex)
{
    SFCollectionRef collection = processor->_collection;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    classCount = SF_MARK_BASE_POS__CLASS_COUNT(markBasePos);
    offset = SF_MARK_BASE_POS__MARK_ARRAY(markBasePos);
    markArray = SF_DATA__SUBDATA(markBasePos, offset);

    /* Get mark anchor and its class value. */
    markAnchor = _SFMarkArrayGetAnchor(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        SFData baseArray;
        SFUInt16 baseCount;

        offset = SF_MARK_BASE_POS__BASE_ARRAY(markBasePos);
        baseArray = SF_DATA__SUBDATA(markBasePos, offset);
        baseCount = SF_BASE_ARRAY__BASE_COUNT(baseArray);

        /* Validate base index. */
        if (baseIndex < baseCount) {
            SFData baseRecord;
            SFData baseAnchor;
            SFPoint markPoint;
            SFPoint basePoint;

            baseRecord = SF_BASE_ARRAY__BASE_RECORD(baseArray, baseIndex, classCount);
            offset = SF_BASE_RECORD__BASE_ANCHOR(baseRecord, classValue);
            baseAnchor = SF_DATA__SUBDATA(baseArray, offset);

            /* Get mark and base points from their respective anchors. */
            markPoint = _SFConvertAnchorToPoint(markAnchor);
            basePoint = _SFConvertAnchorToPoint(baseAnchor);

            /* Connect mark glyph with base glyph. */
            collection->positionArray[markIndex].x = markPoint.x - basePoint.x;
            collection->positionArray[markIndex].y = markPoint.y - basePoint.y;

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent)
{
    SFCollectionRef collection = processor->_collection;
    SFLocatorRef locator = processor->_locator;
    SFLookupFlag lookupFlag = processor->_lookupFlag | SFLookupFlagIgnoreMarks;
    SFUInteger inputIndex = locator->index;
    SFUInteger prevIndex;

    /* Initialize component counter. */
    *outComponent = 0;

    /*
     * NOTE:
     *      Previous non-mark glyph is assumed to be a ligature glyph. It is not
     *      necessary to confirm whether that is actually a ligature glyph or not.
     */
    prevIndex = SFLocatorGetBefore(locator, lookupFlag, inputIndex);

    if (prevIndex != SFInvalidIndex) {
        SFUInteger association = collection->detailArray[prevIndex].association;
        SFUInteger nextIndex;

        /*
         * REMARKS:
         *      The glyphs acting as components of a ligature are not removed
         *      from the collection, but their trait is set to
         *      SFGlyphTraitRemoved and their association is set to first
         *      character forming the ligature.
         *
         * PROCESS:
         *      1) Start loop from ligature index to input index.
         *      2) If association of a glyph matches with the association of
         *         ligature, it is a component of the ligature.
         *      3) Increase component counter for each matched association.
         */
        for (nextIndex = prevIndex + 1; nextIndex < inputIndex; nextIndex++) {
            if (collection->detailArray[nextIndex].association == association) {
                *outComponent++;
            }
        }

        return SFTrue;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos)
{
    SFCollectionRef collection = processor->_collection;
    SFUInteger inputIndex = processor->_locator->index;
    SFGlyph inputGlyph = collection->glyphArray[inputIndex];
    SFUInt16 format;

    format = SF_MARK_LIG_POS__FORMAT(markLigPos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SF_MARK_LIG_POS__MARK_COVERAGE(markLigPos);
            SFData markCoverage = SF_DATA__SUBDATA(markLigPos, offset);
            SFUInteger markIndex;

            markIndex = _SFSearchCoverageIndex(markCoverage, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex;
                SFUInteger ligComponent;

                prevIndex = _SFGetPreviousLigatureGlyphIndex(processor, &ligComponent);

                /* Proceed only if there is a previous ligature glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData ligCoverage;
                    SFUInteger ligIndex;

                    offset = SF_MARK_LIG_POS__LIGATURE_COVERAGE(markLigPos);
                    ligCoverage = SF_DATA__SUBDATA(markLigPos, offset);
                    ligIndex = _SFSearchCoverageIndex(ligCoverage, prevGlyph);

                    if (ligIndex != SFInvalidIndex) {
                        return _SFApplyMarkToLigArrays(processor, markLigPos, markIndex, ligIndex, ligComponent);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef processor, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent)
{
    SFCollectionRef collection = processor->_collection;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    classCount = SF_MARK_LIG_POS__CLASS_COUNT(markLigPos);
    offset = SF_MARK_LIG_POS__MARK_ARRAY(markLigPos);
    markArray = SF_DATA__SUBDATA(markLigPos, offset);

    /* Get mark anchor and its class value. */
    markAnchor = _SFMarkArrayGetAnchor(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        SFData ligArray;
        SFUInt16 ligCount;

        offset = SF_MARK_LIG_POS__LIGATURE_ARRAY(markLigPos);
        ligArray = SF_DATA__SUBDATA(markLigPos, offset);
        ligCount = SF_BASE_ARRAY__BASE_COUNT(ligArray);

        /* Validate ligature index. */
        if (ligIndex < ligCount) {
            SFData ligAttach;
            SFUInteger componentCount;

            offset = SF_LIGATURE_ARRAY__LIGATURE_ATTACH(ligArray, ligIndex, classCount);
            ligAttach = SF_DATA__SUBDATA(ligArray, offset);
            componentCount = SF_LIGATURE_ATTACH__COMPONENT_COUNT(ligAttach);

            /* Validate ligature component. */
            if (ligComponent < componentCount) {
                SFData compRecord;
                SFData ligAnchor;
                SFPoint markPoint;
                SFPoint ligPoint;

                compRecord = SF_LIGATURE_ATTACH__COMPONENT_RECORD(ligAttach, ligComponent, classCount);
                offset = SF_COMPONENT_RECORD__LIGATURE_ANCHOR(compRecord, classValue);
                ligAnchor = SF_DATA__SUBDATA(ligAttach, offset);

                /* Get mark and ligature points from their respective anchors. */
                markPoint = _SFConvertAnchorToPoint(markAnchor);
                ligPoint = _SFConvertAnchorToPoint(ligAnchor);

                /* Connect mark glyph with ligature glyph. */
                collection->positionArray[markIndex].x = markPoint.x - ligPoint.x;
                collection->positionArray[markIndex].y = markPoint.y - ligPoint.y;

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef processor)
{
    SFLocatorRef locator = processor->_locator;
    SFLookupFlag ignoreFlag = SFLookupFlagIgnoreBaseGlyphs
                            | SFLookupFlagIgnoreMarks
                            | SFLookupFlagIgnoreLigatures;
    SFLookupFlag lookupFlag = processor->_lookupFlag & ~ignoreFlag;

    /*
     * NOTE:
     *      Previous glyph is assumed to be a mark glyph. It is not necessary to
     *      confirm whether that is actually a mark glyph or not.
     */
    return SFLocatorGetBefore(locator, lookupFlag, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos)
{
    SFCollectionRef collection = processor->_collection;
    SFUInteger inputIndex = processor->_locator->index;
    SFGlyph inputGlyph = collection->glyphArray[inputIndex];
    SFUInt16 format;

    format = SF_MARK_MARK_POS__FORMAT(markMarkPos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SF_MARK_MARK_POS__MARK1_COVERAGE(markMarkPos);
            SFData mark1Coverage = SF_DATA__SUBDATA(markMarkPos, offset);
            SFUInteger mark1Index;

            mark1Index = _SFSearchCoverageIndex(mark1Coverage, inputGlyph);

            if (mark1Index != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousMarkGlyphIndex(processor);

                /* Proceed only if there is a previous mark glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData mark2Coverage;
                    SFUInteger mark2Index;

                    offset = SF_MARK_MARK_POS__MARK2_COVERAGE(markMarkPos);
                    mark2Coverage = SF_DATA__SUBDATA(markMarkPos, offset);
                    mark2Index = _SFSearchCoverageIndex(mark2Coverage, prevGlyph);

                    if (mark2Index != SFInvalidIndex) {
                        return _SFApplyMarkToMarkArrays(processor, markMarkPos, mark1Index, mark2Index);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef processor, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index)
{
    SFCollectionRef collection = processor->_collection;
    SFUInt16 classCount;
    SFOffset offset;
    SFData mark1Array;
    SFUInt16 classValue;
    SFData mark1Anchor;

    classCount = SF_MARK_MARK_POS__CLASS_COUNT(markMarkPos);
    offset = SF_MARK_MARK_POS__MARK1_ARRAY(markMarkPos);
    mark1Array = SF_DATA__SUBDATA(markMarkPos, offset);

    /* Get mark anchor and its class value. */
    mark1Anchor = _SFMarkArrayGetAnchor(mark1Array, mark1Index, &classValue);
    /* Validate mark anchor and its class value. */
    if (mark1Anchor && classValue < classCount) {
        SFData mark2Array;
        SFUInt16 mark2Count;

        offset = SF_MARK_MARK_POS__MARK2_ARRAY(markMarkPos);
        mark2Array = SF_DATA__SUBDATA(markMarkPos, offset);
        mark2Count = SF_BASE_ARRAY__BASE_COUNT(mark2Array);

        /* Validate mark 2 index. */
        if (mark2Index < mark2Count) {
            SFData mark2Record;
            SFData mark2Anchor;
            SFPoint mark1Point;
            SFPoint mark2Point;

            mark2Record = SF_MARK2_ARRAY__MARK2_RECORD(mark2Array, mark2Index, classCount);
            offset = SF_MARK2_RECORD__MARK2_ANCHOR(mark2Record, classValue);
            mark2Anchor = SF_DATA__SUBDATA(mark2Array, offset);

            /* Get mark and base points from their respective anchors. */
            mark1Point = _SFConvertAnchorToPoint(mark1Anchor);
            mark2Point = _SFConvertAnchorToPoint(mark2Anchor);

            /* Connect mark1 glyph with mark2 glyph. */
            collection->positionArray[mark1Index].x = mark1Point.x - mark2Point.x;
            collection->positionArray[mark1Index].y = mark1Point.y - mark2Point.y;

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInteger *outClass)
{
    SFUInt16 markCount = SF_MARK_ARRAY__MARK_COUNT(markArray);

    if (markIndex < markCount) {
        SFData markRecord = SF_MARK_ARRAY__MARK_RECORD(markArray, markIndex);
        SFUInt16 classValue = SF_MARK_RECORD__CLASS(markRecord);
        SFOffset anchorOffset = SF_MARK_RECORD__MARK_ANCHOR(markRecord);
        SFData markAnchor = SF_DATA__SUBDATA(markRecord, anchorOffset);

        *outClass = classValue;
        return markAnchor;
    }

    *outClass = 0;
    return NULL;
}
