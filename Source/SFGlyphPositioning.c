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
#include "SFShapingEngine.h"

static SFBoolean _SFApplySinglePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData singlePos);

static SFBoolean _SFApplyPairPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData pairPos);
static SFBoolean _SFApplyPairPosFormat1(SFShapingEngineRef engine, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);
static SFData _SFSearchPairRecord(SFData pairSetData, SFUInteger recordSize, SFGlyph glyph);
static SFBoolean _SFApplyPairPosFormat2(SFShapingEngineRef engine, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);

static void _SFApplyValueRecord(SFShapingEngineRef engine, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex);

static SFPoint _SFConvertAnchorToPoint(SFShapingEngineRef engine, SFData anchor);
static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyph inputGlyph, SFData *outExitAnchor, SFData *outEntryAnchor);
static SFBoolean _SFApplyCursivePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData cursivePos);

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator);
static SFBoolean _SFApplyMarkToBasePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markBasePos);
static SFBoolean _SFApplyMarkToBaseArrays(SFShapingEngineRef engine, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex);

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator, SFUInteger *outComponent);
static SFBoolean _SFApplyMarkLigPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markLigPos);
static SFBoolean _SFApplyMarkLigArrays(SFShapingEngineRef engine, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent);

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator);
static SFBoolean _SFApplyMarkToMarkPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markBasePos);
static SFBoolean _SFApplyMarkToMarkArrays(SFShapingEngineRef engine, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index);
static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInteger *outClass);

SF_PRIVATE void _SFApplyGPOSLookup(SFShapingEngineRef engine, SFLocatorRef locator, SFData lookup)
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

        didPosition = _SFApplyPos(engine, locator, lookupType, subtable);

        /* A subtable has performed positioning, so break the loop. */
        if (didPosition) {
            break;
        }
    }
}

SF_PRIVATE SFBoolean _SFApplyPos(SFShapingEngineRef engine, SFLocatorRef locator, SFLookupType lookupType, SFData subtable)
{
    switch (lookupType) {
    case SFLookupTypeSingleAdjustment:
        return _SFApplySinglePos(engine, &locator, subtable);

    case SFLookupTypePairAdjustment:
        return _SFApplyPairPos(engine, locator, subtable);

    case SFLookupTypeCursiveAttachment:
        return _SFApplyCursivePos(engine, locator, subtable);

    case SFLookupTypeMarkToBaseAttachment:
        return _SFApplyMarkToBasePos(engine, locator, subtable);

    case SFLookupTypeMarkToLigatureAttachment:
        return _SFApplyMarkLigPos(engine, locator, subtable);

    case SFLookupTypeMarkToMarkAttachment:
        return _SFApplyMarkToMarkPos(engine, locator, subtable);

    case SFLookupTypeContextPositioning:
        break;

    case SFLookupTypeChainedContextPositioning:
        break;

    case SFLookupTypeExtensionPositioning:
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplySinglePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData singlePos)
{
    SFCollectionRef collection = engine->_collection;
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

                _SFApplyValueRecord(engine, valueRecord, valueFormat, inputIndex);

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

                _SFApplyValueRecord(engine, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyPairPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData pairPos)
{
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
            didPosition = _SFApplyPairPosFormat1(engine, pairPos, firstIndex, secondIndex, &shouldSkip);
            break;

        case 2:
            didPosition = _SFApplyPairPosFormat2(engine, pairPos, firstIndex, secondIndex, &shouldSkip);
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

static SFBoolean _SFApplyPairPosFormat1(SFShapingEngineRef engine, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFCollectionRef collection = engine->_collection;
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
            SFData pairValueRecord;

            offset = SF_PAIR_POS_F1__PAIR_SET(pairPos, pairSetIndex);
            pairSet = SF_DATA__SUBDATA(pairPos, offset);
            pairValueRecord = _SFSearchPairRecord(pairSet, recordSize, secondGlyph);

            if (pairValueRecord) {
                if (value1Size) {
                    SFData value1 = SF_PAIR_VALUE_RECORD__VALUE1(pairValueRecord);
                    _SFApplyValueRecord(engine, value1, valueFormat1, firstIndex);
                }

                if (value2Size) {
                    SFData value2 = SF_PAIR_VALUE_RECORD__VALUE2(pairValueRecord, value1Size);
                    _SFApplyValueRecord(engine, value2, valueFormat2, secondIndex);

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

static SFBoolean _SFApplyPairPosFormat2(SFShapingEngineRef engine, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFCollectionRef collection = engine->_collection;
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
                _SFApplyValueRecord(engine, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                SFData value2 = SF_CLASS2_RECORD__VALUE2(class2Record, value1Size);
                _SFApplyValueRecord(engine, value2, valueFormat2, secondIndex);

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

static void _SFApplyValueRecord(SFShapingEngineRef engine, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFCollectionRef collection = engine->_collection;
    SFOffset offset;

    offset = 0;

    if (SF_VALUE_FORMAT__X_PLACEMENT(valueFormat)) {
        collection->positionArray[inputIndex].x += (SFInt16)SF_VALUE_RECORD__NEXT_VALUE(valueRecord, offset);
    }

    if (SF_VALUE_FORMAT__Y_PLACEMENT(valueFormat)) {
        collection->positionArray[inputIndex].y += (SFInt16)SF_VALUE_RECORD__NEXT_VALUE(valueRecord, offset);
    }

    switch (engine->_traits.direction) {
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

static SFPoint _SFConvertAnchorToPoint(SFShapingEngineRef engine, SFData anchor)
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

static SFBoolean _SFApplyCursivePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData cursivePos)
{
    SFCollectionRef collection = engine->_collection;
    SFUInteger inputIndex = locator->index;
    SFUInt16 format = SF_CURSIVE_POS__FORMAT(cursivePos);

    switch (format) {
    case 1:
        return _SFApplyCursivePosFormat1(engine, locator, cursivePos);
    }

    return SFFalse;
}

static SFBoolean _SFApplyCursivePosFormat1(SFShapingEngineRef engine, SFLocatorRef locator, SFData cursivePos)
{
    SFCollectionRef collection = engine->_collection;
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
                SFPoint exitPoint = _SFConvertAnchorToPoint(engine, inputExitAnchor);
                SFPoint entryPoint = _SFConvertAnchorToPoint(engine, nextEntryAnchor);
                SFInteger oldX;

                switch (engine->_traits.direction) {
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
                if (engine->_lookupFlag & SFLookupFlagRightToLeft) {
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

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator)
{
    SFLookupFlag lookupFlag = engine->_lookupFlag | SFLookupFlagIgnoreMarks;

    /*
    * NOTE:
    *      Previous non-mark glyph is assumed as a base glyph. It is not
    *      necessary to confirm whether that is actually a base glyph or not.
    */
    return SFLocatorGetBefore(locator, lookupFlag, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToBasePos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markBasePos)
{
    SFCollectionRef collection = engine->_collection;
    SFUInteger inputIndex = locator->index;
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
                SFUInteger prevIndex = _SFGetPreviousBaseGlyphIndex(engine, locator);

                /* Proceed only if there is a previous base glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData baseCoverage;
                    SFUInteger baseIndex;

                    offset = SF_MARK_BASE_POS__BASE_COVERAGE(markBasePos);
                    baseCoverage = SF_DATA__SUBDATA(markBasePos, offset);
                    baseIndex = _SFSearchCoverageIndex(baseCoverage, prevGlyph);

                    if (baseIndex != SFInvalidIndex) {
                        return _SFApplyMarkToBaseArrays(engine, markBasePos, markIndex, baseIndex);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToBaseArrays(SFShapingEngineRef engine, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex)
{
    SFCollectionRef collection = engine->_collection;
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
            markPoint = _SFConvertAnchorToPoint(engine, markAnchor);
            basePoint = _SFConvertAnchorToPoint(engine, baseAnchor);

            /* Connect mark glyph with base glyph. */
            collection->positionArray[markIndex].x = markPoint.x - basePoint.x;
            collection->positionArray[markIndex].y = markPoint.y - basePoint.y;

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator, SFUInteger *outComponent)
{
    SFCollectionRef collection = engine->_collection;
    SFLookupFlag lookupFlag = engine->_lookupFlag | SFLookupFlagIgnoreMarks;
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
         *      1) Start a loop from ligature index to input index.
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

static SFBoolean _SFApplyMarkLigPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markLigPos)
{
    SFCollectionRef collection = engine->_collection;
    SFUInteger inputIndex = locator->index;
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

                prevIndex = _SFGetPreviousLigatureGlyphIndex(engine, locator, &ligComponent);

                /* Proceed only if there is a previous ligature glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData ligCoverage;
                    SFUInteger ligIndex;

                    offset = SF_MARK_LIG_POS__LIGATURE_COVERAGE(markLigPos);
                    ligCoverage = SF_DATA__SUBDATA(markLigPos, offset);
                    ligIndex = _SFSearchCoverageIndex(ligCoverage, prevGlyph);

                    if (ligIndex != SFInvalidIndex) {
                        return _SFApplyMarkLigArrays(engine, markLigPos, markIndex, ligIndex, ligComponent);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkLigArrays(SFShapingEngineRef engine, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent)
{
    SFCollectionRef collection = engine->_collection;
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
                markPoint = _SFConvertAnchorToPoint(engine, markAnchor);
                ligPoint = _SFConvertAnchorToPoint(engine, ligAnchor);

                /* Connect mark glyph with ligature glyph. */
                collection->positionArray[markIndex].x = markPoint.x - ligPoint.x;
                collection->positionArray[markIndex].y = markPoint.y - ligPoint.y;

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFShapingEngineRef engine, SFLocatorRef locator)
{
    SFLookupFlag ignoreFlag = SFLookupFlagIgnoreBaseGlyphs
                            | SFLookupFlagIgnoreMarks
                            | SFLookupFlagIgnoreLigatures;
    SFLookupFlag lookupFlag = engine->_lookupFlag & ~ignoreFlag;

    /*
     * NOTE:
     *      Previous glyph is assumed to be a mark glyph. It is not necessary to
     *      confirm whether that is actually a mark glyph or not.
     */
    return SFLocatorGetBefore(locator, lookupFlag, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToMarkPos(SFShapingEngineRef engine, SFLocatorRef locator, SFData markMarkPos)
{
    SFCollectionRef collection = engine->_collection;
    SFUInteger inputIndex = locator->index;
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
                SFUInteger prevIndex = _SFGetPreviousMarkGlyphIndex(engine, locator);

                /* Proceed only if there is a previous mark glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyph prevGlyph = collection->glyphArray[prevIndex];
                    SFData mark2Coverage;
                    SFUInteger mark2Index;

                    offset = SF_MARK_MARK_POS__MARK2_COVERAGE(markMarkPos);
                    mark2Coverage = SF_DATA__SUBDATA(markMarkPos, offset);
                    mark2Index = _SFSearchCoverageIndex(mark2Coverage, prevIndex);

                    if (mark2Index != SFInvalidIndex) {
                        return _SFApplyMarkToMarkArrays(engine, markMarkPos, mark1Index, mark2Index);
                    }
                }
            }
        }
        break;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToMarkArrays(SFShapingEngineRef engine, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index)
{
    SFCollectionRef collection = engine->_collection;
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
            mark1Point = _SFConvertAnchorToPoint(engine, mark1Anchor);
            mark2Point = _SFConvertAnchorToPoint(engine, mark2Anchor);

            /* Connect mark1 glyph with mark2 glyph. */
            collection->positionArray[mark1Index].x = mark1Point.x - mark2Point.x;
            collection->positionArray[mark1Index].y = mark1Point.y - mark2Point.y;

            return SFTrue;
        }
    }

    return SFFalse;
}
