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

static SFBoolean _SFApplySinglePos(SFTextProcessorRef textProcessor, SFData singlePos);

static SFBoolean _SFApplyPairPos(SFTextProcessorRef textProcessor, SFData pairPos);
static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef textProcessor, SFData pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);
static SFBoolean _SFApplyPairPosF2(SFTextProcessorRef textProcessor, SFData pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);

static SFBoolean _SFApplyCursivePos(SFTextProcessorRef textProcessor, SFData cursivePos);
static SFBoolean _SFApplyCursivePosF1(SFTextProcessorRef textProcessor, SFData cursivePos);
static SFBoolean _SFApplyCursiveAnchors(SFTextProcessorRef textProcessor,
    SFData exitAnchorTable, SFData entryAnchorTable, SFUInteger firstIndex, SFUInteger secondIndex);

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef textProcessor);
static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef textProcessor, SFData markBasePos);
static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef textProcessor, SFData markBasePos,
    SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex);

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef textProcessor, SFUInteger *outComponent);
static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef textProcessor, SFData markLigPos);
static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef textProcessor, SFData markLigPos,
    SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex);

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef textProcessor);
static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef textProcessor, SFData markMarkPos);
static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef textProcessor, SFData markMarkPos,
    SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex);

static SFData _SFMarkArrayGetAnchorTable(SFData markArray, SFUInteger markIndex, SFUInt16 *outClass);

static void _SFResolveLeftCursiveSegment(SFTextProcessorRef textProcessor, SFUInteger inputIndex);
static void _SFResolveRightCursiveSegment(SFTextProcessorRef textProcessor, SFUInteger inputIndex);
static void _SFResolveCursivePositions(SFTextProcessorRef textProcessor, SFLocatorRef locator);
static void _SFResolveMarkPositions(SFTextProcessorRef textProcessor, SFLocatorRef locator);

SF_PRIVATE SFBoolean _SFApplyPositioningSubtable(SFTextProcessorRef textProcessor, SFLookupType lookupType, SFData subtable)
{
    switch (lookupType) {
        case SFLookupTypeSingleAdjustment:
            return _SFApplySinglePos(textProcessor, subtable);

        case SFLookupTypePairAdjustment:
            return _SFApplyPairPos(textProcessor, subtable);

        case SFLookupTypeCursiveAttachment:
            return _SFApplyCursivePos(textProcessor, subtable);

        case SFLookupTypeMarkToBaseAttachment:
            return _SFApplyMarkToBasePos(textProcessor, subtable);

        case SFLookupTypeMarkToLigatureAttachment:
            return _SFApplyMarkToLigPos(textProcessor, subtable);

        case SFLookupTypeMarkToMarkAttachment:
            return _SFApplyMarkToMarkPos(textProcessor, subtable);

        case SFLookupTypeContextPositioning:
            return _SFApplyContextSubtable(textProcessor, subtable);

        case SFLookupTypeChainedContextPositioning:
            return _SFApplyChainContextSubtable(textProcessor, subtable);

        case SFLookupTypeExtensionPositioning:
            return _SFApplyExtensionSubtable(textProcessor, subtable);
    }

    return SFFalse;
}

static void _SFApplyValueRecord(SFTextProcessorRef textProcessor,
    SFData valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFAlbumRef album = textProcessor->_album;
    SFOffset offset = 0;
    SFInt16 value;

    if (SFValueFormat_XPlacement(valueFormat)) {
        SFInt32 glyphX = SFAlbumGetX(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

        SFAlbumSetX(album, inputIndex, glyphX + value);
    }

    if (SFValueFormat_YPlacement(valueFormat)) {
        SFInt32 glyphY = SFAlbumGetY(album, inputIndex);

        value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

        SFAlbumSetY(album, inputIndex, glyphY + value);
    }

    switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            if (SFValueFormat_XAdvance(valueFormat)) {
                SFAdvance advance = SFAlbumGetAdvance(album, inputIndex);

                value = (SFInt16)SFValueRecord_NextValue(valueRecord, offset);

                SFAlbumSetAdvance(album, inputIndex, advance + value);
            }
            break;
    }

    /*
     * TODO: Add support for device tables.
     */
}

static SFBoolean _SFApplySinglePos(SFTextProcessorRef textProcessor, SFData singlePos)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFSinglePos_Format(singlePos);

    switch (format) {
        case 1: {
            SFData coverageTable = SFSinglePosF1_CoverageTable(singlePos);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 valueFormat = SFSinglePosF1_ValueFormat(singlePos);
                SFData valueRecord = SFSinglePosF1_ValueRecord(singlePos);

                _SFApplyValueRecord(textProcessor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
            break;
        }

        case 2: {
            SFData coverageTable = SFSinglePosF2_CoverageTable(singlePos);
            SFUInt16 valueFormat = SFSinglePosF2_ValueFormat(singlePos);
            SFUInt16 valueCount = SFSinglePosF2_ValueCount(singlePos);
            SFUInteger valueIndex;

            valueIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (valueIndex < valueCount) {
                SFUInteger valueSize = SFValueRecord_Size(valueFormat);
                SFData valueRecord = SFSinglePosF2_ValueRecord(singlePos, valueIndex, valueSize);

                _SFApplyValueRecord(textProcessor, valueRecord, valueFormat, inputIndex);

                return SFTrue;
            }
            break;
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

static SFData _SFSearchPairRecord(SFData pairSetTable, SFUInteger recordSize, SFGlyphID glyph)
{
    SFUInt16 valueCount = SFPairSet_PairValueCount(pairSetTable);
    SFData recordArray = SFPairSet_PairValueRecordArray(pairSetTable);

    void *item = bsearch(&glyph, recordArray, valueCount, recordSize, _SFPairRecordGlyphComparison);
    if (!item) {
        return NULL;
    }

    return (SFData)item;
}

static SFBoolean _SFApplyPairPos(SFTextProcessorRef textProcessor, SFData pairPos)
{
    SFLocatorRef locator = &textProcessor->_locator;
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
                didPosition = _SFApplyPairPosF1(textProcessor, pairPos, firstIndex, secondIndex, &shouldSkip);
                break;

            case 2:
                didPosition = _SFApplyPairPosF2(textProcessor, pairPos, firstIndex, secondIndex, &shouldSkip);
                break;
        }
    }

    if (shouldSkip) {
        SFLocatorJumpTo(locator, secondIndex);
    }

    return didPosition;
}

static SFBoolean _SFApplyPairPosF1(SFTextProcessorRef textProcessor, SFData pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFAlbumRef album = textProcessor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    SFData coverageTable;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    coverageTable = SFPairPosF1_CoverageTable(pairPos);
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SFPairPosF1_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = SFPairPosF1_ValueFormat2(pairPos);
        SFUInt16 pairSetCount = SFPairPosF1_PairSetCount(pairPos);
        SFUInteger value1Size = SFValueRecord_Size(valueFormat1);
        SFUInteger value2Size = SFValueRecord_Size(valueFormat2);
        SFUInteger recordSize = SFPairValueRecord_Size(value1Size, value2Size);
        SFUInteger pairSetIndex;

        for (pairSetIndex = 0; pairSetIndex < pairSetCount; pairSetIndex++) {
            SFData pairSetTable = SFPairPosF1_PairSetTable(pairPos, pairSetIndex);
            SFData pairRecord;

            pairRecord = _SFSearchPairRecord(pairSetTable, recordSize, secondGlyph);

            if (pairRecord) {
                if (value1Size) {
                    SFData value1 = SFPairValueRecord_Value1(pairRecord);
                    _SFApplyValueRecord(textProcessor, value1, valueFormat1, firstIndex);
                }

                if (value2Size) {
                    SFData value2 = SFPairValueRecord_Value2(pairRecord, value1Size);
                    _SFApplyValueRecord(textProcessor, value2, valueFormat2, secondIndex);

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

static SFBoolean _SFApplyPairPosF2(SFTextProcessorRef textProcessor, SFData pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFAlbumRef album = textProcessor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    SFOffset coverageOffset;
    SFData coverageTable;
    SFUInteger coverageIndex;

    *outShouldSkip = SFFalse;

    coverageOffset = SFPairPosF2_CoverageOffset(pairPos);
    coverageTable = SFData_Subdata(pairPos, coverageOffset);
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, firstGlyph);

    if (coverageIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = SFPairPosF2_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = SFPairPosF2_ValueFormat2(pairPos);
        SFData classDef1Table = SFPairPosF2_ClassDef1Table(pairPos);
        SFData classDef2Table = SFPairPosF2_ClassDef2Table(pairPos);
        SFUInt16 class1Count = SFPairPosF2_Class1Count(pairPos);
        SFUInt16 class2Count = SFPairPosF2_Class2Count(pairPos);
        SFUInt16 class1Value;
        SFUInt16 class2Value;

        class1Value = SFOpenTypeSearchGlyphClass(classDef1Table, firstGlyph);
        class2Value = SFOpenTypeSearchGlyphClass(classDef2Table, secondGlyph);

        if (class1Value < class1Count && class2Value < class2Count) {
            SFUInteger value1Size = SFValueRecord_Size(valueFormat1);
            SFUInteger value2Size = SFValueRecord_Size(valueFormat2);
            SFUInteger class2Size = SFClass2Record_Value(value1Size, value2Size);
            SFUInteger class1Size = SFClass1Record_Size(class2Count, class2Size);
            SFData class1Record = SFPairPosF2_Class1Record(pairPos, class1Value, class1Size);
            SFData class2Record = SFClass1Record_Class2Record(class1Record, class2Value, class2Size);

            if (value1Size) {
                SFData value1 = SFClass2Record_Value1(class2Record);
                _SFApplyValueRecord(textProcessor, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                SFData value2 = SFClass2Record_Value2(class2Record, value1Size);
                _SFApplyValueRecord(textProcessor, value2, valueFormat2, secondIndex);

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

static SFPoint _SFConvertAnchorToPoint(SFData anchorTable)
{
    SFUInt16 format;
    SFPoint point;

    format = SFAnchor_Format(anchorTable);

    switch (format) {
        case 1:
            point.x = SFAnchorF1_XCoordinate(anchorTable);
            point.y = SFAnchorF1_YCoordinate(anchorTable);
            break;

            /* TODO: Support remaining anchor formats. */

        default:
            point.x = 0;
            point.y = 0;
            break;
    }

    return point;
}

static void _SFSearchCursiveAnchors(SFData cursivePos, SFGlyphID inputGlyph,
    SFData *refExitAnchorTable, SFData *refEntryAnchorTable)
{
    SFData coverageTable = SFCursivePos_CoverageTable(cursivePos);
    SFUInt16 entryExitCount = SFCursivePos_EntryExitCount(cursivePos);
    SFUInteger entryExitIndex;

    entryExitIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

    if (entryExitIndex < entryExitCount) {
        SFData entryExitRecord = SFCursivePos_EntryExitRecord(cursivePos, entryExitIndex);
        SFOffset exitAnchorOffset = SFEntryExitRecord_ExitAnchorOffset(entryExitRecord);
        SFOffset entryAnchorOffset = SFEntryExitRecord_EntryAnchorOffset(entryExitRecord);

        if (refExitAnchorTable && exitAnchorOffset) {
            *refExitAnchorTable = SFData_Subdata(cursivePos, exitAnchorOffset);
        }

        if (refEntryAnchorTable && entryAnchorOffset) {
            *refEntryAnchorTable = SFData_Subdata(cursivePos, entryAnchorOffset);
        }
    }
}

static SFBoolean _SFApplyCursivePos(SFTextProcessorRef textProcessor, SFData cursivePos)
{
    SFUInt16 format = SFCursivePos_Format(cursivePos);

    switch (format) {
        case 1:
            return _SFApplyCursivePosF1(textProcessor, cursivePos);
    }

    return SFFalse;
}

static SFBoolean _SFApplyCursivePosF1(SFTextProcessorRef textProcessor, SFData cursivePos)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger firstIndex = locator->index;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFData exitAnchorTable = NULL;

    _SFSearchCursiveAnchors(cursivePos, firstGlyph, &exitAnchorTable, NULL);

    /* Proceed only if exit anchor of first glyph exists. */
    if (exitAnchorTable) {
        SFUInteger secondIndex = SFLocatorGetAfter(locator, firstIndex);

        if (secondIndex != SFInvalidIndex) {
            SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
            SFData entryAnchorTable = NULL;

            _SFSearchCursiveAnchors(cursivePos, secondGlyph, NULL, &entryAnchorTable);

            /* Proceed only if entry anchor of second glyph exists. */
            if (entryAnchorTable) {
                return _SFApplyCursiveAnchors(textProcessor, exitAnchorTable, entryAnchorTable, firstIndex, secondIndex);
            }
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyCursiveAnchors(SFTextProcessorRef textProcessor,
    SFData exitAnchorTable, SFData entryAnchorTable, SFUInteger firstIndex, SFUInteger secondIndex)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFPoint exitPoint = _SFConvertAnchorToPoint(exitAnchorTable);
    SFPoint entryPoint = _SFConvertAnchorToPoint(entryAnchorTable);
    SFGlyphTraits traits;
    SFInt32 offset;
    SFAdvance advance;

    traits = SFGlyphTraitCursive;

    switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
            /*
             * PROCESS:
             *      - Set advance of first glyph in such a way that it ends at exit x.
             *      - Set x of second glyph in such a way that it starts at entry x while preserving
             *        its advance.
             *      - Set y of second glyph in such a way that entry y and exit y meet.
             */

            /* Set advance of first glyph. */
            offset = SFAlbumGetX(album, firstIndex);
            SFAlbumSetAdvance(album, firstIndex, offset + exitPoint.x);

            /* Preserve advance of second glyph. */
            offset = SFAlbumGetX(album, secondIndex);
            advance = SFAlbumGetAdvance(album, secondIndex);
            SFAlbumSetAdvance(album, secondIndex, advance - offset + -entryPoint.x);

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
             *      - In case of RTL, the direction of a glyph is reversed. So, it starts from
             *        advance and ends at zero.
             * PROCESS:
             *      - Set advance of second glyph in such a way that it ends at entry x.
             *      - Set x of first glyph in such a way that it starts at exit x while preserving
             *        its advance.
             *      - Set y of first glyph in such a way that entry y and exit y meet.
             */

            /* Set advance of second glyph. */
            offset = SFAlbumGetX(album, secondIndex);
            SFAlbumSetAdvance(album, secondIndex, offset + entryPoint.x);

            /* Preserve advance of first glyph. */
            offset = SFAlbumGetX(album, firstIndex);
            advance = SFAlbumGetAdvance(album, firstIndex);
            SFAlbumSetAdvance(album, firstIndex, advance - offset + -exitPoint.x);

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
    }

    /* Update the details of first glyph. */
    SFAlbumSetCursiveOffset(album, firstIndex, (SFUInt16)(secondIndex - firstIndex));
    SFAlbumInsertTraits(album, firstIndex, traits);

    /* Update the details of second glyph. */
    SFAlbumSetCursiveOffset(album, secondIndex, 0);
    SFAlbumInsertTraits(album, secondIndex, traits);

    return SFTrue;
}

static SFUInteger _SFGetPreviousBaseGlyphIndex(SFTextProcessorRef textProcessor)
{
    SFLocatorRef locator = &textProcessor->_locator;
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

static SFBoolean _SFApplyMarkToBasePos(SFTextProcessorRef textProcessor, SFData markBasePos)
{
    SFAlbumRef album = textProcessor->_album;
    SFUInteger inputIndex = textProcessor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkBasePos_Format(markBasePos);

    switch (format) {
        case 1: {
            SFData markCoverageTable = SFMarkBasePos_MarkCoverageTable(markBasePos);
            SFUInteger markIndex;

            markIndex = SFOpenTypeSearchCoverageIndex(markCoverageTable, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousBaseGlyphIndex(textProcessor);
                SFGlyphID prevGlyph;

                /* Proceed only if there is a previous base glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFData baseCoverageTable = SFMarkBasePos_BaseCoverageTable(markBasePos);
                    SFUInteger baseIndex;

                    prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    baseIndex = SFOpenTypeSearchCoverageIndex(baseCoverageTable, prevGlyph);

                    if (baseIndex != SFInvalidIndex) {
                        return _SFApplyMarkToBaseArrays(textProcessor, markBasePos, markIndex, baseIndex, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToBaseArrays(SFTextProcessorRef textProcessor, SFData markBasePos,
    SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFData markArrayTable;
    SFUInt16 classValue;
    SFData markAnchorTable;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

    classCount = SFMarkBasePos_ClassCount(markBasePos);
    markArrayTable = SFMarkBasePos_MarkArrayTable(markBasePos);

    /* Get mark anchor and its class value. */
    markAnchorTable = _SFMarkArrayGetAnchorTable(markArrayTable, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchorTable && classValue < classCount) {
        SFData baseArrayTable = SFMarkBasePos_BaseArrayTable(markBasePos);
        SFUInt16 baseCount;

        baseCount = SFBaseArray_BaseCount(baseArrayTable);

        /* Validate base index. */
        if (baseIndex < baseCount) {
            SFData baseRecord = SFBaseArray_BaseRecord(baseArrayTable, baseIndex, classCount);
            SFOffset baseAnchorOffset = SFBaseArray_BaseAnchorOffset(baseRecord, classValue);
            SFData baseAnchorTable = SFData_Subdata(baseArrayTable, baseAnchorOffset);
            SFPoint markPoint;
            SFPoint basePoint;

            /* Get mark and base points from their respective anchors. */
            markPoint = _SFConvertAnchorToPoint(markAnchorTable);
            basePoint = _SFConvertAnchorToPoint(baseAnchorTable);

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

static SFUInteger _SFGetPreviousLigatureGlyphIndex(SFTextProcessorRef textProcessor, SFUInteger *outComponent)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
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
        SFUInteger nextIndex;

        /*
         * REMARKS:
         *      The glyphs acting as components of a ligature are not removed from the album, but
         *      their trait is set to SFGlyphTraitPlaceholder.
         *
         * PROCESS:
         *      1) Start loop from ligature index to input index.
         *      2) If a placeholder glyph is found, it is a component of the ligature.
         *      3) Increase component counter for each placeholder.
         */
        for (nextIndex = ligatureIndex + 1; nextIndex < inputIndex; nextIndex++) {
            if (SFAlbumGetTraits(album, nextIndex) & SFGlyphTraitPlaceholder) {
                (*outComponent)++;
            }
        }
    }

    /* Restore the old lookup flag. */
    SFLocatorSetLookupFlag(locator, lookupFlag);

    return ligatureIndex;
}

static SFBoolean _SFApplyMarkToLigPos(SFTextProcessorRef textProcessor, SFData markLigPos)
{
    SFAlbumRef album = textProcessor->_album;
    SFUInteger inputIndex = textProcessor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkLigPos_Format(markLigPos);

    switch (format) {
        case 1: {
            SFData markCoverageTable = SFMarkLigPos_MarkCoverageTable(markLigPos);
            SFUInteger markIndex;

            markIndex = SFOpenTypeSearchCoverageIndex(markCoverageTable, inputGlyph);

            if (markIndex != SFInvalidIndex) {
                SFUInteger prevIndex;
                SFUInteger ligComponent;
                SFGlyphID prevGlyph;

                prevIndex = _SFGetPreviousLigatureGlyphIndex(textProcessor, &ligComponent);

                /* Proceed only if there is a previous ligature glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFData ligCoverageTable = SFMarkLigPos_LigatureCoverageTable(markLigPos);
                    SFUInteger ligIndex;

                    prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    ligIndex = SFOpenTypeSearchCoverageIndex(ligCoverageTable, prevGlyph);

                    if (ligIndex != SFInvalidIndex) {
                        return _SFApplyMarkToLigArrays(textProcessor, markLigPos, markIndex, ligIndex, ligComponent, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToLigArrays(SFTextProcessorRef textProcessor, SFData markLigPos,
    SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFData markArrayTable;
    SFUInt16 classValue;
    SFData markAnchorTable;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

    classCount = SFMarkLigPos_ClassCount(markLigPos);
    markArrayTable = SFMarkLigPos_MarkArrayTable(markLigPos);

    /* Get mark anchor and its class value. */
    markAnchorTable = _SFMarkArrayGetAnchorTable(markArrayTable, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchorTable && classValue < classCount) {
        SFData ligArrayTable = SFMarkLigPos_LigatureArrayTable(markLigPos);
        SFUInt16 ligCount = SFLigatureArray_LigatureCount(ligArrayTable);

        /* Validate ligature index. */
        if (ligIndex < ligCount) {
            SFData ligAttachTable = SFLigatureArray_LigatureAttachTable(ligArrayTable, ligIndex);
            SFUInteger compCount = SFLigatureAttach_ComponentCount(ligAttachTable);
            SFData compRecord;
            SFOffset ligAnchorOffset;
            SFData ligAnchor;
            SFPoint markPoint;
            SFPoint ligPoint;

            /* Use last component in case of error. */
            if (ligComponent >= compCount) {
                ligComponent = compCount - 1;
            }

            compRecord = SFLigatureAttach_ComponentRecord(ligAttachTable, ligComponent, classCount);
            ligAnchorOffset = SFComponentRecord_LigatureAnchorOffset(compRecord, classValue);
            ligAnchor = SFData_Subdata(ligAttachTable, ligAnchorOffset);

            /* Get mark and ligature points from their respective anchors. */
            markPoint = _SFConvertAnchorToPoint(markAnchorTable);
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

    return SFFalse;
}

static SFUInteger _SFGetPreviousMarkGlyphIndex(SFTextProcessorRef textProcessor)
{
    SFLocatorRef locator = &textProcessor->_locator;
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

static SFBoolean _SFApplyMarkToMarkPos(SFTextProcessorRef textProcessor, SFData markMarkPos)
{
    SFAlbumRef album = textProcessor->_album;
    SFUInteger inputIndex = textProcessor->_locator.index;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
    SFUInt16 format;

    format = SFMarkMarkPos_Format(markMarkPos);

    switch (format) {
        case 1: {
            SFData mark1CoverageTable = SFMarkMarkPos_Mark1CoverageTable(markMarkPos);
            SFUInteger mark1Index;

            mark1Index = SFOpenTypeSearchCoverageIndex(mark1CoverageTable, inputGlyph);

            if (mark1Index != SFInvalidIndex) {
                SFUInteger prevIndex = _SFGetPreviousMarkGlyphIndex(textProcessor);
                SFGlyphID prevGlyph;

                /* Proceed only if there is a previous mark glyph. */
                if (prevIndex != SFInvalidIndex) {
                    SFData mark2Coverage = SFMarkMarkPos_Mark2CoverageTable(markMarkPos);
                    SFUInteger mark2Index;

                    prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                    mark2Index = SFOpenTypeSearchCoverageIndex(mark2Coverage, prevGlyph);

                    if (mark2Index != SFInvalidIndex) {
                        return _SFApplyMarkToMarkArrays(textProcessor, markMarkPos, mark1Index, mark2Index, prevIndex);
                    }
                }
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyMarkToMarkArrays(SFTextProcessorRef textProcessor, SFData markMarkPos,
    SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInt16 classCount;
    SFData mark1ArrayTable;
    SFUInt16 classValue;
    SFData mark1AnchorTable;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < inputIndex);

    classCount = SFMarkMarkPos_ClassCount(markMarkPos);
    mark1ArrayTable = SFMarkMarkPos_Mark1ArrayTable(markMarkPos);

    /* Get mark anchor and its class value. */
    mark1AnchorTable = _SFMarkArrayGetAnchorTable(mark1ArrayTable, mark1Index, &classValue);
    /* Validate mark anchor and its class value. */
    if (mark1AnchorTable && classValue < classCount) {
        SFData mark2ArrayTable = SFMarkMarkPos_Mark2ArrayTable(markMarkPos);
        SFUInt16 mark2Count = SFMark2Array_Mark2Count(mark2ArrayTable);

        /* Validate mark 2 index. */
        if (mark2Index < mark2Count) {
            SFData mark2Record = SFMark2Array_Mark2Record(mark2ArrayTable, mark2Index, classCount);
            SFOffset mark2AnchorOffset = SFMark2Record_Mark2AnchorOffset(mark2Record, classValue);
            SFData mark2AnchorTable = SFData_Subdata(mark2ArrayTable, mark2AnchorOffset);
            SFPoint mark1Point;
            SFPoint mark2Point;

            /* Get mark and base points from their respective anchors. */
            mark1Point = _SFConvertAnchorToPoint(mark1AnchorTable);
            mark2Point = _SFConvertAnchorToPoint(mark2AnchorTable);

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

static SFData _SFMarkArrayGetAnchorTable(SFData markArray, SFUInteger markIndex, SFUInt16 *outClass)
{
    SFUInt16 markCount = SFMarkArray_MarkCount(markArray);

    if (markIndex < markCount) {
        SFData markRecord = SFMarkArray_MarkRecord(markArray, markIndex);
        SFUInt16 classValue = SFMarkRecord_Class(markRecord);
        SFOffset anchorOffset = SFMarkRecord_MarkAnchorOffset(markRecord);
        SFData anchorTable = SFData_Subdata(markArray, anchorOffset);

        *outClass = classValue;
        return anchorTable;
    }

    *outClass = 0;
    return NULL;
}

static void _SFResolveLeftCursiveSegment(SFTextProcessorRef textProcessor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For left-to-right cursively attached segment, first glyph is positioned on BASELINE
     *      pushing next glyphs DOWNWARD.
     */

    SFAlbumRef album = textProcessor->_album;
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
        SFInt32 inputY;
        SFInt32 nextY;

        switch (textProcessor->_textDirection) {
            case SFTextDirectionLeftToRight:
            case SFTextDirectionRightToLeft: {
                inputY = SFAlbumGetY(album, inputIndex);
                nextY = SFAlbumGetY(album, nextIndex);

                SFAlbumSetY(album, nextIndex, nextY + inputY);
                break;
            }
        }

        _SFResolveLeftCursiveSegment(textProcessor, nextIndex);

        /* Mark this glyph as resolved. */
        SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitResolved);
    }
}

static void _SFResolveRightCursiveSegment(SFTextProcessorRef textProcessor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For right-to-left cursively attached segment, last glyph is positioned on BASELINE,
     *      pushing previous glyphs UPWARD.
     */

    SFAlbumRef album = textProcessor->_album;
    SFUInteger offset;

    /* The glyph MUST be cursive and right-to-left. */
    SFAssert(SFAlbumGetTraits(album, inputIndex) & (SFGlyphTraitCursive | SFGlyphTraitRightToLeft));
    /* The glyph must NOT be resolved yet. */
    SFAssert(!(SFAlbumGetTraits(album, inputIndex) & SFGlyphTraitResolved));

    offset = SFAlbumGetCursiveOffset(album, inputIndex);

    if (offset) {
        SFUInteger nextIndex = inputIndex + offset;
        SFInt32 inputY;
        SFInt32 nextY;

        _SFResolveRightCursiveSegment(textProcessor, nextIndex);

        switch (textProcessor->_textDirection) {
            case SFTextDirectionLeftToRight:
            case SFTextDirectionRightToLeft: {
                inputY = SFAlbumGetY(album, inputIndex);
                nextY = SFAlbumGetY(album, nextIndex);

                SFAlbumSetY(album, inputIndex, inputY + nextY);
                break;
            }
        }

        /* Mark this glyph as resolved. */
        SFAlbumInsertTraits(album, inputIndex, SFGlyphTraitResolved);
    }
}

static void _SFResolveCursivePositions(SFTextProcessorRef textProcessor, SFLocatorRef locator)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorReset(locator, 0, album->glyphCount);

    while (SFLocatorMoveNext(locator)) {
        SFUInteger locatorIndex = locator->index;
        SFGlyphTraits traits = SFAlbumGetTraits(album, locatorIndex);

        if ((traits & (SFGlyphTraitCursive | SFGlyphTraitResolved)) == SFGlyphTraitCursive) {
            if (traits & SFGlyphTraitRightToLeft) {
                _SFResolveRightCursiveSegment(textProcessor, locator->index);
            } else {
                _SFResolveLeftCursiveSegment(textProcessor, locator->index);
            }
        }
    }
}

static void _SFResolveMarkPositions(SFTextProcessorRef textProcessor, SFLocatorRef locator)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorReset(locator, 0, album->glyphCount);

    while (SFLocatorMoveNext(locator)) {
        SFUInteger inputIndex = locator->index;
        SFGlyphTraits traits = SFAlbumGetTraits(album, inputIndex);

        if (traits & SFGlyphTraitAttached) {
            SFUInt16 attachmentOffset = SFAlbumGetAttachmentOffset(album, inputIndex);
            SFUInteger attachmentIndex = inputIndex - attachmentOffset;
            SFInt32 markX = SFAlbumGetX(album, inputIndex);
            SFInt32 markY = SFAlbumGetY(album, inputIndex);
            SFUInteger index;

            /* Put the mark glyph OVER attached glyph. */
            markX += SFAlbumGetX(album, attachmentIndex);;
            markY += SFAlbumGetY(album, attachmentIndex);;

            /* Close the gap between the mark glyph and previous glyph. */
            switch (textProcessor->_textDirection) {
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
            }

            /* Update the position of mark glyph. */
            SFAlbumSetX(album, inputIndex, markX);
            SFAlbumSetY(album, inputIndex, markY);
        }
    }
}

SF_PRIVATE void _SFResolveAttachments(SFTextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocator locator;

    SFLocatorInitialize(&locator, album, NULL);

    _SFResolveCursivePositions(textProcessor, &locator);
    _SFResolveMarkPositions(textProcessor, &locator);
}
