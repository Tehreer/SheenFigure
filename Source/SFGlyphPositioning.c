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

#include <stddef.h>
#include <stdlib.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGPOS.h"
#include "SFLocator.h"
#include "SFPattern.h"
#include "SFOpenType.h"

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
static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef processor, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex);

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent);
static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos);
static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef processor, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex);

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef processor);
static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos);
static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef processor, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex);

static SFData _SFMarkArrayGetAnchor(SFData markArray, SFUInteger markIndex, SFUInt16 *outClass);

static void _SFResolveLeftCursiveSegment(SFTextProcessorRef processor, SFUInteger inputIndex);
static void _SFResolveRightCursiveSegment(SFTextProcessorRef processor, SFUInteger inputIndex);
static void _SFResolveCursivePositions(SFTextProcessorRef processor, SFLocatorRef locator);
static void _SFResolveMarkPositions(SFTextProcessorRef processor, SFLocatorRef locator);

SF_PRIVATE void _SFApplyPositioningLookup(SFTextProcessorRef processor, SFData lookup)
{
    SFLookupType lookupType = SFLookup_LookupType(lookup);
    SFLookupFlag lookupFlag = SFLookup_LookupFlag(lookup);
    SFUInt16 subtableCount = SFLookup_SubtableCount(lookup);
    SFUInteger subtableIndex;

    SFLocatorSetLookupFlag(&processor->_locator, lookupFlag);

    if (lookupFlag & SFLookupFlagUseMarkFilteringSet) {
        SFUInt16 markFilteringSet = SFLookup_MarkFilteringSet(lookup, subtableCount);
        SFLocatorSetMarkFilteringSet(&processor->_locator, markFilteringSet);
    }

    /* Apply subtables in order until one of them performs positioning. */
    for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
        SFOffset subtableOffset = SFLookup_SubtableOffset(lookup, subtableIndex);
        SFData subtable = SFData_Subdata(lookup, subtableOffset);
        SFBoolean didPosition;

        didPosition = _SFApplyPositioningSubtable(processor, lookupType, subtable);

        /* A subtable has performed positioning, so break the loop. */
        if (didPosition) {
            break;
        }
    }
}

SF_PRIVATE SFBoolean _SFApplyPositioningSubtable(SFTextProcessorRef processor, SFLookupType lookupType, SFData subtable)
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
            return _SFApplyChainContextSubtable(processor, SFFeatureKindPositioning, subtable);

        case SFLookupTypeExtensionPositioning:
            return _SFApplyExtensionSubtable(processor, SFFeatureKindPositioning, subtable);
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
        case 1: {
            SFOffset offset = SFSinglePosF1_Coverage(singlePos);
            SFData coverage = SFData_Subdata(singlePos, offset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 valueFormat = SFSinglePosF1_ValueFormat(singlePos);
                SFData valueRecord = SFSinglePosF1_ValueRecord(singlePos);

                _SFApplyValueRecord(processor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
            break;
        }

        case 2: {
            SFOffset offset = SFSinglePosF2_CoverageOffset(singlePos);
            SFData coverage = SFData_Subdata(singlePos, offset);
            SFUInt16 valueFormat = SFSinglePosF2_ValueFormat(singlePos);
            SFUInt16 valueCount = SFSinglePosF2_ValueCount(singlePos);
            SFUInteger valueIndex;

            valueIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

            if (valueIndex < valueCount) {
                SFUInteger valueSize = SFValueRecord_Size(valueFormat);
                SFData valueRecord = SFSinglePosF2_ValueRecord(singlePos, valueIndex, valueSize);

                _SFApplyValueRecord(processor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
            break;
        }
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
    secondIndex = SFLocatorGetAfter(locator, firstIndex);

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
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, firstGlyph);

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
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SFPairPosF2_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = SFPairPosF2_ValueFormat2(pairPos);
        SFOffset classDef1Offset = SFPairPosF2_ClassDef1Offset(pairPos);
        SFOffset classDef2Offset = SFPairPosF2_ClassDef2Offset(pairPos);
        SFUInt16 class1Count = SFPairPosF2_Class1Count(pairPos);
        SFUInt16 class2Count = SFPairPosF2_Class2Count(pairPos);
        SFData classDef1 = SFData_Subdata(pairPos, classDef1Offset);
        SFData classDef2 = SFData_Subdata(pairPos, classDef2Offset);
        SFUInt16 class1Value = SFOpenTypeSearchGlyphClass(classDef1, firstGlyph);
        SFUInt16 class2Value = SFOpenTypeSearchGlyphClass(classDef2, secondGlyph);

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

static int _SFPairRecordGlyphComparison(const void *item1, const void *item2)
{
    SFGlyphID *ref1 = (SFGlyphID *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFGlyphID secondGlyph = SFPairValueRecord_SecondGlyph(ref2);

    return val1 - secondGlyph;
}

static SFData _SFSearchPairRecord(SFData pairSet, SFUInteger recordSize, SFGlyphID glyph)
{
    SFUInt16 valueCount = SFPairSet_PairValueCount(pairSet);
    SFData recordArray = SFPairSet_PairValueRecordArray(pairSet);

    void *item = bsearch(&glyph, recordArray, valueCount, recordSize, _SFPairRecordGlyphComparison);
    if (!item) {
        return NULL;
    }

    return (SFData)item;
}

static void _SFApplyValueRecord(SFTextProcessorRef processor, SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFAlbumRef album = processor->_album;
    SFOffset offset = 0;
    SFInt16 value;

    if (SFValueFormat_XPlacement(valueFormat)) {
        SFInteger glyphX = SFAlbumGetX(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

        SFAlbumSetX(album, inputIndex, glyphX + value);
    }

    if (SFValueFormat_YPlacement(valueFormat)) {
        SFInteger glyphY = SFAlbumGetY(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

        SFAlbumSetY(album, inputIndex, glyphY + value);
    }

    switch (processor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            if (SFValueFormat_XAdvance(valueFormat)) {
                SFAdvance advance = SFAlbumGetAdvance(album, inputIndex);

                value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

                SFAlbumSetAdvance(album, inputIndex, advance + value);
            }
            break;

        default:
            /* Unsupported direction. */
            SFAssert(0);
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

    entryExitIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

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
        SFUInteger secondIndex = SFLocatorGetAfter(locator, firstIndex);

        if (secondIndex != SFInvalidIndex) {
            SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
            SFData secondExitAnchor;
            SFData secondEntryAnchor;

            _SFSearchCursiveAnchors(cursivePos, secondGlyph, &secondExitAnchor, &secondEntryAnchor);

            /* Proceed only if entry anchor of second glyph exists. */
            if (secondEntryAnchor) {
                SFPoint exitPoint = _SFConvertAnchorToPoint(firstExitAnchor);
                SFPoint entryPoint = _SFConvertAnchorToPoint(secondEntryAnchor);
                SFGlyphTraits traits;
                SFInteger position;
                SFAdvance advance;

                traits = SFGlyphTraitCursive;

                switch (processor->_textDirection) {
                    case SFTextDirectionLeftToRight:
                        /*
                         * PROCESS:
                         *      - Set advance of first glyph in such a way that it ends at exit x.
                         *      - Set x of second glyph in such a way that it starts at entry x
                         *        while preserving its advance.
                         *      - Set y of second glyph in such a way that entry y and exit y meet.
                         */

                        /* Set advance of first glyph. */
                        position = SFAlbumGetX(album, firstIndex);
                        SFAlbumSetAdvance(album, firstIndex, position + exitPoint.x);

                        /* Preserve advance of second glyph. */
                        position = SFAlbumGetX(album, secondIndex);
                        advance = SFAlbumGetAdvance(album, secondIndex);
                        SFAlbumSetAdvance(album, secondIndex, advance - position + -entryPoint.x);

                        /* Set x of second glyph. */
                        SFAlbumSetX(album, secondIndex, -entryPoint.x);

                        /* Set y of second glyph taking RTL flag into account. */
                        if (locator->lookupFlag & SFLookupFlagRightToLeft) {
                            traits |= SFGlyphTraitRightToLeft;
                            SFAlbumSetY(album, secondIndex, entryPoint.y - exitPoint.y);
                        } else {
                            SFAlbumSetY(album, secondIndex, exitPoint.y - entryPoint.y);
                        }

                        break;

                    case SFTextDirectionRightToLeft:
                        /*
                         * REMARKS:
                         *      - In case of RTL, the direction of a glyph is reversed. So, it
                         *        starts from advance and ends at zero.
                         * PROCESS:
                         *      - Set advance of second glyph in such a way that it ends at entry x.
                         *      - Set x of first glyph in such a way that it starts at exit x while
                         *        preserving its advance.
                         *      - Set y of first glyph in such a way that entry y and exit y meet.
                         */

                        /* Set advance of second glyph. */
                        position = SFAlbumGetX(album, secondIndex);
                        SFAlbumSetAdvance(album, secondIndex, position + entryPoint.x);

                        /* Preserve advance of first glyph. */
                        position = SFAlbumGetX(album, firstIndex);
                        advance = SFAlbumGetAdvance(album, firstIndex);
                        SFAlbumSetAdvance(album, firstIndex, advance - position + -exitPoint.x);

                        /* Set x of first glyph. */
                        SFAlbumSetX(album, firstIndex, -exitPoint.x);
                        SFAlbumSetY(album, firstIndex, entryPoint.y - exitPoint.y);

                        /* Set y of first glyph taking RTL flag into account. */
                        if (locator->lookupFlag & SFLookupFlagRightToLeft) {
                            traits |= SFGlyphTraitRightToLeft;
                            SFAlbumSetY(album, firstIndex, entryPoint.y - exitPoint.y);
                        } else {
                            SFAlbumSetY(album, firstIndex, exitPoint.y - entryPoint.y);
                        }
                        break;

                    default:
                        /* Unsupported direction. */
                        SFAssert(0);
                        break;
                }

                /* Update the details of first glyph. */
                SFAlbumSetCursiveOffset(album, firstIndex, (SFUInt16)(secondIndex - firstIndex));
                SFAlbumInsertTraits(album, firstIndex, traits);
                /* Update the details of second glyph. */
                SFAlbumSetCursiveOffset(album, secondIndex, 0);
                SFAlbumInsertTraits(album, secondIndex, traits);

                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef processor)
{
    SFLocatorRef locator = &processor->_locator;
    SFLookupFlag lookupFlag = locator->lookupFlag;
    SFUInteger baseIndex;

    /* Make locator ignore marks. */
    SFLocatorSetLookupFlag(locator, lookupFlag | SFLookupFlagIgnoreMarks);

    /*
     * NOTE:
     *      Previous non-mark glyph is assumed to be a base glyph.
     */
    baseIndex = SFLocatorGetBefore(locator, locator->index);

    /* Restore the old lookup flag. */
    SFLocatorSetLookupFlag(locator, lookupFlag);

    return baseIndex;
}

static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef processor, SFData markBasePos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkBasePos_Format(markBasePos);

    switch (format) {
        case 1: {
            SFOffset offset = SFMarkBasePos_MarkCoverageOffset(markBasePos);
            SFData markCoverage = SFData_Subdata(markBasePos, offset);
            SFUInteger markIndex;

            markIndex = SFOpenTypeSearchCoverageIndex(markCoverage, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousBaseGlyphIndex(processor);

                /* Proceed only if there is a previous base glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyphID prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    SFData baseCoverage;
                    SFUInteger baseIndex;

                    offset = SFMarkBasePos_BaseCoverageOffset(markBasePos);
                    baseCoverage = SFData_Subdata(markBasePos, offset);
                    baseIndex = SFOpenTypeSearchCoverageIndex(baseCoverage, prevGlyph);

                    if (baseIndex != SFInvalidIndex) {
                        return _SFApplyMarkToBaseArrays(processor, markBasePos, markIndex, baseIndex, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef processor, SFData markBasePos, SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

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
            SFAlbumSetX(album, inputIndex, basePoint.x - markPoint.x);
            SFAlbumSetY(album, inputIndex, basePoint.y - markPoint.y);
            /* Update the details of mark glyph. */
            SFAlbumSetAttachmentOffset(album, inputIndex, (SFUInt16)(inputIndex - attachmentIndex));
            SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitAttached);

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef processor, SFUInteger *outComponent)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFLookupFlag lookupFlag = locator->lookupFlag;
    SFUInteger inputIndex = locator->index;
    SFUInteger ligatureIndex;

    /* Initialize component counter. */
    *outComponent = 0;

    /* Make locator ignore marks. */
    SFLocatorSetLookupFlag(locator, lookupFlag | SFLookupFlagIgnoreMarks);

    /*
     * NOTE:
     *      Previous non-mark glyph is assumed to be a ligature glyph.
     */
    ligatureIndex = SFLocatorGetBefore(locator, inputIndex);

    if (ligatureIndex != SFInvalidIndex) {
        SFUInteger association = SFAlbumGetSingleAssociation(album, ligatureIndex);
        SFUInteger nextIndex;

        /*
         * REMARKS:
         *      The glyphs acting as components of a ligature are not removed from the album, but
         *      their trait is set to SFGlyphTraitPlaceholder and their association is set to first
         *      character forming the ligature.
         *
         * PROCESS:
         *      1) Start loop from ligature index to input index.
         *      2) If association of a glyph matches with the association of ligature, it is a
         *         component of the ligature.
         *      3) Increase component counter for each matched association.
         */
        for (nextIndex = ligatureIndex + 1; nextIndex < inputIndex; nextIndex++) {
            if (SFAlbumGetSingleAssociation(album, nextIndex) == association) {
                (*outComponent)++;
            }
        }
    }

    /* Restore the old lookup flag. */
    SFLocatorSetLookupFlag(locator, lookupFlag);

    return ligatureIndex;
}

static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef processor, SFData markLigPos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkLigPos_Format(markLigPos);

    switch (format) {
        case 1: {
            SFOffset offset = SFMarkLigPos_MarkCoverageOffset(markLigPos);
            SFData markCoverage = SFData_Subdata(markLigPos, offset);
            SFUInteger markIndex;

            markIndex = SFOpenTypeSearchCoverageIndex(markCoverage, inputGlyph);

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
                    ligIndex = SFOpenTypeSearchCoverageIndex(ligCoverage, prevGlyph);

                    if (ligIndex != SFInvalidIndex) {
                        return _SFApplyMarkToLigArrays(processor, markLigPos, markIndex, ligIndex, ligComponent, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef processor, SFData markLigPos, SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFOffset offset;
    SFData markArray;
    SFUInt16 classValue;
    SFData markAnchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

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
                SFAlbumSetX(album, inputIndex, ligPoint.x - markPoint.x);
                SFAlbumSetY(album, inputIndex, ligPoint.y - markPoint.y);
                /* Update the details of mark glyph. */
                SFAlbumSetAttachmentOffset(album, inputIndex, (SFUInt16)(inputIndex - attachmentIndex));
                SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitAttached);

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
    SFLookupFlag lookupFlag = locator->lookupFlag;
    SFUInteger markIndex;

    /* Make locator ignore nothing. */
    SFLocatorSetLookupFlag(locator, lookupFlag & ~ignoreFlag);

    /*
     * NOTE:
     *      Previous glyph is assumed to be a mark glyph.
     */
    markIndex = SFLocatorGetBefore(locator, locator->index);

    /* Restore the old lookup flag. */
    SFLocatorSetLookupFlag(locator, lookupFlag);

    return markIndex;
}

static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef processor, SFData markMarkPos)
{
    SFAlbumRef album = processor->_album;
    SFUInteger inputIndex = processor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkMarkPos_Format(markMarkPos);

    switch (format) {
        case 1: {
            SFOffset offset = SFMarkMarkPos_Mark1CoverageOffset(markMarkPos);
            SFData mark1Coverage = SFData_Subdata(markMarkPos, offset);
            SFUInteger mark1Index;

            mark1Index = SFOpenTypeSearchCoverageIndex(mark1Coverage, inputGlyph);

            if (mark1Index != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousMarkGlyphIndex(processor);

                /* Proceed only if there is a previous mark glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFGlyphID prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    SFData mark2Coverage;
                    SFUInteger mark2Index;

                    offset = SFMarkMarkPos_Mark2CoverageOffset(markMarkPos);
                    mark2Coverage = SFData_Subdata(markMarkPos, offset);
                    mark2Index = SFOpenTypeSearchCoverageIndex(mark2Coverage, prevGlyph);

                    if (mark2Index != SFInvalidIndex) {
                        return _SFApplyMarkToMarkArrays(processor, markMarkPos, mark1Index, mark2Index, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef processor, SFData markMarkPos, SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFOffset offset;
    SFData mark1Array;
    SFUInt16 classValue;
    SFData mark1Anchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

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
            SFAlbumSetX(album, inputIndex, mark2Point.x - mark1Point.x);
            SFAlbumSetY(album, inputIndex, mark2Point.y - mark1Point.y);
            /* Update the details of mark1 glyph. */
            SFAlbumSetAttachmentOffset(album, inputIndex, (SFUInt16)(inputIndex - attachmentIndex));
            SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitAttached);

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
        SFData markAnchor = SFData_Subdata(markArray, anchorOffset);

        *outClass = classValue;
        return markAnchor;
    }

    *outClass = 0;
    return NULL;
}

static void _SFResolveLeftCursiveSegment(SFTextProcessorRef processor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For left-to-right cursively attached segment, first glyph is positioned on BASELINE
     *      pushing next glyphs downward.
     */

    SFAlbumRef album = processor->_album;
    SFUInteger offset;

    /* The glyph MUST be cursive and right-to-left. */
    SFAssert(SFAlbumGetTraits(album, inputIndex) & SFGlyphTraitCursive);
    /* The glyph must NOT be right-to-left. */
    SFAssert(!(SFAlbumGetTraits(album, inputIndex) & SFGlyphTraitRightToLeft));
    /* The glyph must NOT be resolved yet. */
    SFAssert(!(SFAlbumGetTraits(album, inputIndex) & SFGlyphTraitResolved));

    offset = SFAlbumGetCursiveOffset(album, inputIndex);

    if (offset) {
        SFUInteger nextIndex = inputIndex + offset;
        SFInteger inputY;
        SFInteger nextY;

        switch (processor->_textDirection) {
            case SFTextDirectionLeftToRight:
            case SFTextDirectionRightToLeft:
                inputY = SFAlbumGetY(album, inputIndex);
                nextY = SFAlbumGetY(album, nextIndex);

                SFAlbumSetY(album, nextIndex, nextY + inputY);
                break;

            default:
                /* Unsupported direction. */
                SFAssert(0);
                break;
        }

        _SFResolveLeftCursiveSegment(processor, nextIndex);

        /* Mark this glyph as resolved. */
        SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitResolved);
    }
}

static void _SFResolveRightCursiveSegment(SFTextProcessorRef processor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For right-to-left cursively attached segment, last glyph is positioned on BASELINE,
     *      pushing previous glyphs upward.
     */

    SFAlbumRef album = processor->_album;
    SFUInteger offset;

    /* The glyph MUST be cursive and right-to-left. */
    SFAssert(SFAlbumGetTraits(album, inputIndex) & (SFGlyphTraitCursive | SFGlyphTraitRightToLeft));
    /* The glyph must NOT be resolved yet. */
    SFAssert(!(SFAlbumGetTraits(album, inputIndex) & SFGlyphTraitResolved));

    offset = SFAlbumGetCursiveOffset(album, inputIndex);

    if (offset) {
        SFUInteger nextIndex = inputIndex + offset;
        SFInteger inputY;
        SFInteger nextY;

        _SFResolveRightCursiveSegment(processor, nextIndex);

        switch (processor->_textDirection) {
            case SFTextDirectionLeftToRight:
            case SFTextDirectionRightToLeft:
                inputY = SFAlbumGetY(album, inputIndex);
                nextY = SFAlbumGetY(album, nextIndex);

                SFAlbumSetY(album, inputIndex, inputY + nextY);
                break;
                
            default:
                /* Unsupported direction. */
                SFAssert(0);
                break;
        }

        /* Mark this glyph as resolved. */
        SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitResolved);
    }
}

static void _SFResolveCursivePositions(SFTextProcessorRef processor, SFLocatorRef locator)
{
    SFAlbumRef album = processor->_album;
    SFLocatorReset(locator, 0, album->glyphCount);

    while (SFLocatorMoveNext(locator)) {
        SFUInteger locatorIndex = locator->index;
        SFGlyphTraits traits = SFAlbumGetTraits(album, locatorIndex);

        if ((traits & (SFGlyphTraitCursive | SFGlyphTraitResolved)) == SFGlyphTraitCursive) {
            if (traits & SFGlyphTraitRightToLeft) {
                _SFResolveRightCursiveSegment(processor, locator->index);
            } else {
                _SFResolveLeftCursiveSegment(processor, locator->index);
            }
        }
    }
}

static void _SFResolveMarkPositions(SFTextProcessorRef processor, SFLocatorRef locator)
{
    SFAlbumRef album = processor->_album;
    SFLocatorReset(locator, 0, album->glyphCount);

    while (SFLocatorMoveNext(locator)) {
        SFUInteger inputIndex = locator->index;
        SFGlyphTraits traits = SFAlbumGetTraits(album, inputIndex);

        if (traits & SFGlyphTraitAttached) {
            SFUInt16 attachmentOffset = SFAlbumGetAttachmentOffset(album, inputIndex);
            SFUInteger attachmentIndex = inputIndex - attachmentOffset;
            SFInteger markX = SFAlbumGetX(album, inputIndex);
            SFInteger markY = SFAlbumGetY(album, inputIndex);
            SFUInteger index;

            /* Put the mark glyph OVER attached glyph. */
            markX += SFAlbumGetX(album, attachmentIndex);;
            markY += SFAlbumGetY(album, attachmentIndex);;

            /* Close the gap between the mark glyph and previous glyph. */
            switch (processor->_textDirection) {
                case SFTextDirectionLeftToRight:
                    for (index = attachmentIndex; index < inputIndex; index++) {
                        markX -= SFAlbumGetAdvance(album, index);
                    }
                    break;

                case SFTextDirectionRightToLeft:
                    for (index = attachmentIndex + 1; index <= inputIndex; index++) {
                        markX += SFAlbumGetAdvance(album, index);
                    }
                    break;

                default:
                    /* Unsupported direction. */
                    SFAssert(0);
                    break;
            }

            /* Update the position of mark glyph. */
            SFAlbumSetX(album, inputIndex, markX);
            SFAlbumSetY(album, inputIndex, markY);
        }
    }
}

SF_PRIVATE void _SFResolveAttachments(SFTextProcessorRef processor)
{
    SFAlbumRef album = processor->_album;
    SFLocator locator;

    SFLocatorInitialize(&locator, album, NULL);

    _SFResolveCursivePositions(processor, &locator);
    _SFResolveMarkPositions(processor, &locator);
}
