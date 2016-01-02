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
#include "SFLocator.h"
#include "SFGPOS.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFTextProcessor.h"

static SFBoolean _SFApplySinglePos(SFTextProcessorRef processor, SFData singlePos);

static SFBoolean _SFApplyPairPos(SFTextProcessorRef processor, SFData pairPos);
static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);
static SFBoolean _SFApplyPairPosF2(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);

static SFData _SFSearchPairRecord(SFData pairSet, SFUInteger recordSize, SFGlyphID glyph);
static void _SFApplyValueRecord(SFTextProcessorRef processor, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex);

static SFPoint _SFConvertAnchorToPoint(SFData anchor);
static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyphID inputGlyph, SFData *outExitAnchor, SFData *outEntryAnchor);
static SFBoolean _SFApplyCursivePos(SFTextProcessorRef processor, SFData cursivePos);
static SFBoolean _SFApplyCursivePosF1(SFTextProcessorRef processor, SFData cursivePos);

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef processor);
static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef processor, SFData markBasePos);
static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef processor, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex);

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent);
static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos);
static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef processor, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent);

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef processor);
static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos);
static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef processor, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index);

static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInt16 *outClass);

SF_PRIVATE void _SFApplyGPOSLookup(SFTextProcessorRef processor, SFData lookup)
{
    SFLookupType lookupType = SFLookup_LookupType(lookup);
    SFLookupFlag lookupFlag = SFLookup_LookupFlag(lookup);
    SFUInt16 subtableCount = SFLookup_SubtableCount(lookup);
    SFUInteger subtableIndex;

    SFLocatorSetLookupFlag(&processor->_locator, lookupFlag);

    /* Apply subtables in order until one of them performs positioning. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFOffset subtableOffset = SFLookup_SubtableOffset(lookup, subtableIndex);
        SFData subtable = SFData_Subdata(lookup, subtableOffset);
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
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFSinglePos_Format(singlePos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SFSinglePosF1_Coverage(singlePos);
            SFData coverage = SFData_Subdata(singlePos, offset);
            SFUInteger coverageIndex;

            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 valueFormat = SFSinglePosF1_ValueFormat(singlePos);
                SFData valueRecord = SFSinglePosF1_ValueRecord(singlePos);

                _SFApplyValueRecord(processor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFOffset offset = SFSinglePosF2_CoverageOffset(singlePos);
            SFData coverage = SFData_Subdata(singlePos, offset);
            SFUInt16 valueFormat = SFSinglePosF2_ValueFormat(singlePos);
            SFUInt16 valueCount = SFSinglePosF2_ValueCount(singlePos);
            SFUInteger valueIndex;

            valueIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (valueIndex < valueCount) {
                SFUInteger valueSize = SFValueRecord_Size(valueFormat);
                SFData valueRecord = SFSinglePosF2_ValueRecord(singlePos, valueIndex, valueSize);

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
    SFLocatorRef locator = &processor->_locator;
    SFBoolean didPosition = SFFalse;
    SFBoolean shouldSkip = SFFalse;
    SFUInteger firstIndex;
    SFUInteger secondIndex;

    firstIndex = locator->index;
    secondIndex = SFLocatorGetAfter(locator, firstIndex, locator->lookupFlag);

    /* Proceed only if pair glyph is available. */
    if (secondIndex != SFInvalidIndex) {
        SFUInt16 format = SFPairPos_Format(pairPos);

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
        SFLocatorJumpTo(locator, secondIndex);
    }

    return didPosition;
}

static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef processor, SFData pairPos, SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFAlbumRef album = processor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    SFOffset offset;
    SFData coverage;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    offset = SFPairPosF1_CoverageOffset(pairPos);
    coverage = SFData_Subdata(pairPos, offset);
    coverageIndex = _SFSearchCoverageIndex(coverage, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SFPairPosF1_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = SFPairPosF1_ValueFormat2(pairPos);
        SFUInt16 pairSetCount = SFPairPosF1_PairSetCount(pairPos);
        SFUInteger value1Size = SFValueRecord_Size(valueFormat1);
        SFUInteger value2Size = SFValueRecord_Size(valueFormat2);
        SFUInteger recordSize = SFPairValueRecord_Size(value1Size, value2Size);
        SFUInteger pairSetIndex;

        for (pairSetIndex = 0; pairSetIndex < pairSetCount; pairSetIndex++) {
            SFData pairSet;
            SFData pairRecord;

            offset = SFPairPosF1_PairSetOffset(pairPos, pairSetIndex);
            pairSet = SFData_Subdata(pairPos, offset);
            pairRecord = _SFSearchPairRecord(pairSet, recordSize, secondGlyph);

            if (pairRecord) {
                if (value1Size) {
                    SFData value1 = SFPairValueRecord_Value1(pairRecord);
                    _SFApplyValueRecord(processor, value1, valueFormat1, firstIndex);
                }

                if (value2Size) {
                    SFData value2 = SFPairValueRecord_Value2(pairRecord, value1Size);
                    _SFApplyValueRecord(processor, value2, valueFormat2, secondIndex);

                    /*
                     * Pair element should be skipped only if the value record for the second glyph
                     * is AVAILABLE.
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
    SFAlbumRef album = processor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    SFOffset offset;
    SFData coverage;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    offset = SFPairPosF2_CoverageOffset(pairPos);
    coverage = SFData_Subdata(pairPos, offset);
    coverageIndex = _SFSearchCoverageIndex(coverage, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SFPairPosF2_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = SFPairPosF2_ValueFormat2(pairPos);
        SFOffset classDef1Offset = SFPairPosF2_ClassDef1Offset(pairPos);
        SFOffset classDef2Offset = SFPairPosF2_ClassDef2Offset(pairPos);
        SFUInt16 class1Count = SFPairPosF2_Class1Count(pairPos);
        SFUInt16 class2Count = SFPairPosF2_Class2Count(pairPos);
        SFData classDef1 = SFData_Subdata(pairPos, classDef1Offset);
        SFData classDef2 = SFData_Subdata(pairPos, classDef2Offset);
        SFUInt16 class1Value = _SFSearchGlyphClass(classDef1, firstGlyph);
        SFUInt16 class2Value = _SFSearchGlyphClass(classDef2, secondGlyph);

        if (class1Value < class1Count && class2Value < class2Count) {
            SFUInteger value1Size = SFValueRecord_Size(valueFormat1);
            SFUInteger value2Size = SFValueRecord_Size(valueFormat2);
            SFUInteger class2Size = SFClass2Record_Value(value1Size, value2Size);
            SFUInteger class1Size = SFClass1Record_Size(class2Count, class2Size);
            SFData class1Record = SFPairPosF2_Class1Record(pairPos, class1Value, class1Size);
            SFData class2Record = SFClass1Record_Class2Record(class1Record, class2Value, class2Size);

            if (value1Size) {
                SFData value1 = SFClass2Record_Value1(class2Record);
                _SFApplyValueRecord(processor, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                SFData value2 = SFClass2Record_Value2(class2Record, value1Size);
                _SFApplyValueRecord(processor, value2, valueFormat2, secondIndex);

                /*
                 * Pair element should be skipped only if the value record for the second glyph is
                 * AVAILABLE.
                 */
                *outShouldSkip = SFTrue;
            }

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFData _SFSearchPairRecord(SFData pairSet, SFUInteger recordSize, SFGlyphID glyph)
{
    SFUInt16 valueCount = SFPairSet_PairValueCount(pairSet);
    SFData recordArray = SFPairSet_PairValueRecordArray(pairSet);

    if (valueCount > 0) {
        SFUInteger min = 0;
        SFUInteger max = valueCount - 1;

        while (min < max) {
            SFUInteger mid = (min + max) >> 1;
            SFData valueRecord = SFPairSet_PairValueRecord(recordArray, mid, recordSize);
            SFGlyphID secondGlyph = SFPairValueRecord_SecondGlyph(valueRecord);

            if (secondGlyph < glyph) {
                min = mid + 1;
            } else if (secondGlyph > glyph) {
                max = mid;
            } else {
                return valueRecord;
            }
        }
    }

    return NULL;
}

static void _SFApplyValueRecord(SFTextProcessorRef processor, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFAlbumRef album = processor->_album;
    SFOffset offset = 0;
    SFInt16 value;

    if (SFValueFormat_XPlacement(valueFormat)) {
        SFPoint position = SFAlbumGetPosition(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);
        position.x += value;

        SFAlbumSetPosition(album, inputIndex, position);
    }

    if (SFValueFormat_YPlacement(valueFormat)) {
        SFPoint position = SFAlbumGetPosition(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);
        position.y += value;

        SFAlbumSetPosition(album, inputIndex, position);
    }

    switch (processor->_direction) {
    case SFDirectionHorizontal:
        if (SFValueFormat_XAdvance(valueFormat)) {
            SFInteger advance = SFAlbumGetAdvance(album, inputIndex);

            value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);
            advance += value;

            SFAlbumSetAdvance(album, inputIndex, advance);
        }
        break;
    }

    /* NOTE:
     *      There is no need to support device tables as font size is asumed to be exactly same as
     *      the units per em of the font.
     */
}

static SFPoint _SFConvertAnchorToPoint(SFData anchor)
{
    SFUInt16 format;
    SFPoint point;

    format = SFAnchor_Format(anchor);

    switch (format) {
    case 1:
        point.x = SFAnchorF1_XCoordinate(anchor);
        point.y = SFAnchorF1_YCoordinate(anchor);
        break;

        /* TODO: Support remaining anchor formats. */

    default:
        point.x = 0;
        point.y = 0;
        break;
    }

    return point;
}

static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyphID inputGlyph, SFData *outExitAnchor, SFData *outEntryAnchor)
{
    SFOffset offset = SFCursivePos_CoverageOffset(cursivePos);
    SFData coverage = SFData_Subdata(cursivePos, offset);
    SFUInt16 entryExitCount = SFCursivePos_EntryExitCount(cursivePos);
    SFUInteger entryExitIndex;

    *outExitAnchor = NULL;
    *outEntryAnchor = NULL;

    entryExitIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

    if (entryExitIndex < entryExitCount) {
        SFData entryExitRecord = SFCursivePos_EntryExitRecord(cursivePos, entryExitIndex);
        SFOffset exitAnchorOffset = SFEntryExitRecord_ExitAnchorOffset(entryExitRecord);
        SFOffset entryAnchorOffset = SFEntryExitRecord_EntryAnchorOffset(entryExitRecord);

        if (exitAnchorOffset) {
            *outExitAnchor = SFData_Subdata(cursivePos, exitAnchorOffset);
        }

        if (entryAnchorOffset) {
            *outEntryAnchor = SFData_Subdata(cursivePos, entryAnchorOffset);
        }
    }
}

static SFBoolean _SFApplyCursivePos(SFTextProcessorRef processor, SFData cursivePos)
{
    SFUInt16 format = SFCursivePos_Format(cursivePos);

    switch (format) {
    case 1:
        return _SFApplyCursivePosF1(processor, cursivePos);
    }

    return SFFalse;
}

static SFBoolean _SFApplyCursivePosF1(SFTextProcessorRef processor, SFData cursivePos)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger firstIndex = locator->index;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFData firstExitAnchor;
    SFData firstEntryAnchor;

    _SFSearchCursiveAnchors(cursivePos, firstGlyph, &firstExitAnchor, &firstEntryAnchor);

    /* Proceed only if exit anchor of first glyph exists. */
    if (firstExitAnchor) {
        SFUInteger secondIndex = SFLocatorGetAfter(locator, firstIndex, locator->lookupFlag);

        if (secondIndex != SFInvalidIndex) {
            SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
            SFData secondExitAnchor;
            SFData secondEntryAnchor;

            _SFSearchCursiveAnchors(cursivePos, secondGlyph, &secondExitAnchor, &secondEntryAnchor);

            /* Proceed only if entry anchor of second glyph exists. */
            if (secondEntryAnchor) {
                SFPoint exitPoint = _SFConvertAnchorToPoint(firstExitAnchor);
                SFPoint entryPoint = _SFConvertAnchorToPoint(secondEntryAnchor);
                SFPoint firstPosition = SFAlbumGetPosition(album, firstIndex);
                SFPoint secondPosition = SFAlbumGetPosition(album, secondIndex);
                SFInteger secondAdvance = SFAlbumGetAdvance(album, secondIndex);

                switch (processor->_direction) {
                case SFDirectionHorizontal:
                    /*
                     * The exit glyph must be connected with entry glyph (which will replace its old
                     * advance) while preserving its x so that it is placed at intended location.
                     */
                    SFAlbumSetAdvance(album, firstIndex, firstPosition.x + exitPoint.x);
                    /*
                     * The entry glyph must be connected with exit glyph (which will replace its old
                     * x) while preserving its advance so that the pen moves to the intended
                     * location.
                     */
                    secondAdvance -= secondPosition.x + entryPoint.x;
                    secondPosition.x = -entryPoint.x;

                    /* Connect entry glyph with exit glyph vertically. */
                    secondPosition.y = exitPoint.y - entryPoint.y;

                    SFAlbumSetPosition(album, secondIndex, secondPosition);
                    SFAlbumSetAdvance(album, secondIndex, secondAdvance);

                    break;
                }

                /* Set traits of both elements. */
                if (locator->lookupFlag & SFLookupFlagRightToLeft) {
                    SFGlyphTrait traits;

                    traits = SFAlbumGetTraits(album, firstIndex) | SFGlyphTraitRightToLeft;
                    SFAlbumSetTraits(album, firstIndex, traits);
                    SFAlbumSetOffset(album, firstIndex, (SFUInt16)(secondIndex - firstIndex));

                    traits = SFAlbumGetTraits(album, secondIndex) | SFGlyphTraitRightToLeft;
                    SFAlbumSetTraits(album, secondIndex, traits);
                }

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef processor)
{
    SFLocatorRef locator = &processor->_locator;
    SFLookupFlag lookupFlag = locator->lookupFlag | SFLookupFlagIgnoreMarks;

    /*
     * NOTE:
     *      Previous non-mark glyph is assumed as a base glyph. It is not necessary to confirm
     *      whether that is actually a base glyph or not.
     */
    return SFLocatorGetBefore(locator, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef processor, SFData markBasePos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkBasePos_Format(markBasePos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SFMarkBasePos_MarkCoverageOffset(markBasePos);
            SFData markCoverage = SFData_Subdata(markBasePos, offset);
            SFUInteger markIndex;

            markIndex = _SFSearchCoverageIndex(markCoverage, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousBaseGlyphIndex(processor);

                /* Proceed only if there is a previous base glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyphID prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    SFData baseCoverage;
                    SFUInteger baseIndex;

                    offset = SFMarkBasePos_BaseCoverageOffset(markBasePos);
                    baseCoverage = SFData_Subdata(markBasePos, offset);
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
    SFAlbumRef album = processor->_album;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    classCount = SFMarkBasePos_ClassCount(markBasePos);
    offset = SFMarkBasePos_MarkArrayOffset(markBasePos);
    markArray = SFData_Subdata(markBasePos, offset);

    /* Get mark anchor and its class value. */
    markAnchor = _SFMarkArrayGetAnchor(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        SFData baseArray;
        SFUInt16 baseCount;

        offset = SFMarkBasePos_BaseArrayOffset(markBasePos);
        baseArray = SFData_Subdata(markBasePos, offset);
        baseCount = SFBaseArray_BaseCount(baseArray);

        /* Validate base index. */
        if (baseIndex < baseCount) {
            SFPoint markPosition = SFAlbumGetPosition(album, markIndex);
            SFData baseRecord;
            SFData baseAnchor;
            SFPoint markPoint;
            SFPoint basePoint;

            baseRecord = SFBaseArray_BaseRecord(baseArray, baseIndex, classCount);
            offset = SFBaseArray_BaseAnchorOffset(baseRecord, classValue);
            baseAnchor = SFData_Subdata(baseArray, offset);

            /* Get mark and base points from their respective anchors. */
            markPoint = _SFConvertAnchorToPoint(markAnchor);
            basePoint = _SFConvertAnchorToPoint(baseAnchor);

            /* Connect mark glyph with base glyph. */
            markPosition.x = markPoint.x - basePoint.x;
            markPosition.y = markPoint.y - basePoint.y;

            SFAlbumSetPosition(album, markIndex, markPosition);

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFLookupFlag lookupFlag = locator->lookupFlag | SFLookupFlagIgnoreMarks;
    SFUInteger inputIndex = locator->index;
    SFUInteger prevIndex;

    /* Initialize component counter. */
    *outComponent = 0;

    /*
     * NOTE:
     *      Previous non-mark glyph is assumed to be a ligature glyph. It is not necessary to
     *      confirm whether that is actually a ligature glyph or not.
     */
    prevIndex = SFLocatorGetBefore(locator, inputIndex, lookupFlag);

    if (prevIndex != SFInvalidIndex) {
        SFUInteger association = SFAlbumGetAssociation(album, prevIndex);
        SFUInteger nextIndex;

        /*
         * REMARKS:
         *      The glyphs acting as components of a ligature are not removed from the album, but
         *      their trait is set to SFGlyphTraitRemoved and their association is set to first
         *      character forming the ligature.
         *
         * PROCESS:
         *      1) Start loop from ligature index to input index.
         *      2) If association of a glyph matches with the association of igature, it is a
         *         component of the ligature.
         *      3) Increase component counter for each matched association.
         */
        for (nextIndex = prevIndex + 1; nextIndex < inputIndex; nextIndex++) {
            if (SFAlbumGetAssociation(album, nextIndex) == association) {
                (*outComponent)++;
            }
        }

        return SFTrue;
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkLigPos_Format(markLigPos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SFMarkLigPos_MarkCoverageOffset(markLigPos);
            SFData markCoverage = SFData_Subdata(markLigPos, offset);
            SFUInteger markIndex;

            markIndex = _SFSearchCoverageIndex(markCoverage, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex;
                SFUInteger ligComponent;

                prevIndex = _SFGetPreviousLigatureGlyphIndex(processor, &ligComponent);

                /* Proceed only if there is a previous ligature glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyphID prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    SFData ligCoverage;
                    SFUInteger ligIndex;

                    offset = SFMarkLigPos_LigatureCoverageOffset(markLigPos);
                    ligCoverage = SFData_Subdata(markLigPos, offset);
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
    SFAlbumRef album = processor->_album;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    classCount = SFMarkLigPos_ClassCount(markLigPos);
    offset = SFMarkLigPos_MarkArrayOffset(markLigPos);
    markArray = SFData_Subdata(markLigPos, offset);

    /* Get mark anchor and its class value. */
    markAnchor = _SFMarkArrayGetAnchor(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        SFData ligArray;
        SFUInt16 ligCount;

        offset = SFMarkLigPos_LigatureArrayOffset(markLigPos);
        ligArray = SFData_Subdata(markLigPos, offset);
        ligCount = SFBaseArray_BaseCount(ligArray);

        /* Validate ligature index. */
        if (ligIndex < ligCount) {
            SFData ligAttach;
            SFUInteger componentCount;

            offset = SFLigatureArray_LigatureAttachOffset(ligArray, ligIndex);
            ligAttach = SFData_Subdata(ligArray, offset);
            componentCount = SFLigatureAttach_ComponentCount(ligAttach);

            /* Validate ligature component. */
            if (ligComponent < componentCount) {
                SFPoint markPosition = SFAlbumGetPosition(album, markIndex);

                SFData compRecord;
                SFData ligAnchor;
                SFPoint markPoint;
                SFPoint ligPoint;

                compRecord = SFLigatureAttach_ComponentRecord(ligAttach, ligComponent, classCount);
                offset = SFComponentRecord_LigatureAnchorOffset(compRecord, classValue);
                ligAnchor = SFData_Subdata(ligAttach, offset);

                /* Get mark and ligature points from their respective anchors. */
                markPoint = _SFConvertAnchorToPoint(markAnchor);
                ligPoint = _SFConvertAnchorToPoint(ligAnchor);

                /* Connect mark glyph with ligature glyph. */
                markPosition.x = markPoint.x - ligPoint.x;
                markPosition.y = markPoint.y - ligPoint.y;

                SFAlbumSetPosition(album, markIndex, markPosition);

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef processor)
{
    SFLocatorRef locator = &processor->_locator;
    SFLookupFlag ignoreFlag = SFLookupFlagIgnoreBaseGlyphs
                            | SFLookupFlagIgnoreMarks
                            | SFLookupFlagIgnoreLigatures;
    SFLookupFlag lookupFlag = locator->lookupFlag & ~ignoreFlag;

    /*
     * NOTE:
     *      Previous glyph is assumed to be a mark glyph. It is not necessary to confirm whether
     *      that is actually a mark glyph or not.
     */
    return SFLocatorGetBefore(locator, locator->index, lookupFlag);
}

static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkMarkPos_Format(markMarkPos);

    switch (format) {
    case 1:
        {
            SFOffset offset = SFMarkMarkPos_Mark1CoverageOffset(markMarkPos);
            SFData mark1Coverage = SFData_Subdata(markMarkPos, offset);
            SFUInteger mark1Index;

            mark1Index = _SFSearchCoverageIndex(mark1Coverage, inputGlyph);

            if (mark1Index != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousMarkGlyphIndex(processor);

                /* Proceed only if there is a previous mark glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyphID prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    SFData mark2Coverage;
                    SFUInteger mark2Index;

                    offset = SFMarkMarkPos_Mark2CoverageOffset(markMarkPos);
                    mark2Coverage = SFData_Subdata(markMarkPos, offset);
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
    SFAlbumRef album = processor->_album;
    SFUInt16 classCount;
    SFOffset offset;
    SFData mark1Array;
    SFUInt16 classValue;
    SFData mark1Anchor;

    classCount = SFMarkMarkPos_ClassCount(markMarkPos);
    offset = SFMarkMarkPos_Mark1ArrayOffset(markMarkPos);
    mark1Array = SFData_Subdata(markMarkPos, offset);

    /* Get mark anchor and its class value. */
    mark1Anchor = _SFMarkArrayGetAnchor(mark1Array, mark1Index, &classValue);
    /* Validate mark anchor and its class value. */
    if (mark1Anchor && classValue < classCount) {
        SFData mark2Array;
        SFUInt16 mark2Count;

        offset = SFMarkMarkPos_Mark2ArrayOffset(markMarkPos);
        mark2Array = SFData_Subdata(markMarkPos, offset);
        mark2Count = SFBaseArray_BaseCount(mark2Array);

        /* Validate mark 2 index. */
        if (mark2Index < mark2Count) {
            SFPoint mark1Position = SFAlbumGetPosition(album, mark1Index);
            SFData mark2Record;
            SFData mark2Anchor;
            SFPoint mark1Point;
            SFPoint mark2Point;

            mark2Record = SFMark2Array_Mark2Record(mark2Array, mark2Index, classCount);
            offset = SFMark2Record_Mark2AnchorOffset(mark2Record, classValue);
            mark2Anchor = SFData_Subdata(mark2Array, offset);

            /* Get mark and base points from their respective anchors. */
            mark1Point = _SFConvertAnchorToPoint(mark1Anchor);
            mark2Point = _SFConvertAnchorToPoint(mark2Anchor);

            /* Connect mark1 glyph with mark2 glyph. */
            mark1Position.x = mark1Point.x - mark2Point.x;
            mark1Position.y = mark1Point.y - mark2Point.y;

            SFAlbumSetPosition(album, mark1Index, mark1Position);

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInt16 *outClass)
{
    SFUInt16 markCount = SFMarkArray_MarkCount(markArray);

    if (markIndex < markCount) {
        SFData markRecord = SFMarkArray_MarkRecord(markArray, markIndex);
        SFUInt16 classValue = SFMarkRecord_Class(markRecord);
        SFOffset anchorOffset = SFMarkRecord_MarkAnchorOffset(markRecord);
        SFData markAnchor = SFData_Subdata(markRecord, anchorOffset);

        *outClass = classValue;
        return markAnchor;
    }

    *outClass = 0;
    return NULL;
}
