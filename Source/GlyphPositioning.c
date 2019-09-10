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
#include <stddef.h>
#include <stdlib.h>

#include "Data.h"
#include "Locator.h"
#include "SFAssert.h"
#include "SFBase.h"

#include "Tables/GPOS.h"
#include "Tables/OpenType.h"

#include "GlyphManipulation.h"
#include "GlyphPositioning.h"
#include "TextProcessor.h"

static SFBoolean ApplyPairPosF1(TextProcessorRef textProcessor, Data pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);
static SFBoolean ApplyPairPosF2(TextProcessorRef textProcessor, Data pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip);

static SFBoolean ApplyCursiveAnchors(TextProcessorRef textProcessor,
    Data exitAnchor, Data entryAnchor, SFUInteger firstIndex, SFUInteger secondIndex);

static SFBoolean ApplyMarkToBaseArrays(TextProcessorRef textProcessor, Data markBasePos,
    SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex);
static SFBoolean ApplyMarkToLigArrays(TextProcessorRef textProcessor, Data markLigPos,
    SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex);
static SFBoolean ApplyMarkToMarkArrays(TextProcessorRef textProcessor, Data markMarkPos,
    SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex);

static SFInt32 GetXDeltaPixels(TextProcessorRef textProcessor, Data devOrVarIdxTable)
{
    return GetRelevantDeltaPixels(devOrVarIdxTable, textProcessor->_ppemWidth,
                                  textProcessor->_itemVarStore,
                                  textProcessor->_coordArray, textProcessor->_coordCount);
}

static SFInt32 GetYDeltaPixels(TextProcessorRef textProcessor, Data devOrVarIdxTable)
{
    return GetRelevantDeltaPixels(devOrVarIdxTable, textProcessor->_ppemHeight,
                                  textProcessor->_itemVarStore,
                                  textProcessor->_coordArray, textProcessor->_coordCount);
}

static void ApplyValueRecord(TextProcessorRef textProcessor, Data parentTable,
    Data valueRecord, SFUInt16 valueFormat, SFUInteger inputIndex)
{
    SFAlbumRef album = textProcessor->_album;
    Offset16 valueOffset = 0;

    if (ValueFormat_XPlacement(valueFormat)) {
        SFInt16 adjustment = Data_Int16(valueRecord, valueOffset);
        SFAlbumAddX(album, inputIndex, adjustment);
        valueOffset += 2;
    }

    if (ValueFormat_YPlacement(valueFormat)) {
        SFInt16 adjustment = Data_Int16(valueRecord, valueOffset);
        SFAlbumAddY(album, inputIndex, adjustment);
        valueOffset += 2;
    }

    if (ValueFormat_XAdvance(valueFormat)) {
        switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft: {
                SFInt16 adjustment = Data_Int16(valueRecord, valueOffset);
                SFAlbumAddAdvance(album, inputIndex, adjustment);
            }
            break;
        }

        valueOffset += 2;
    }

    if (ValueFormat_YAdvance(valueFormat)) {
        /* TODO: Add support for vertical layout. */
        valueOffset += 2;
    }

    if (ValueFormat_XPlaDevice(valueFormat)) {
        Offset16 deviceOffset = Data_UInt16(valueRecord, valueOffset);

        if (deviceOffset) {
            Data deviceTable = Data_Subdata(parentTable, deviceOffset);
            SFInt32 adjustment;

            adjustment = GetXDeltaPixels(textProcessor, deviceTable);
            SFAlbumAddX(album, inputIndex, adjustment);
        }

        valueOffset += 2;
    }

    if (ValueFormat_YPlaDevice(valueFormat)) {
        Offset16 deviceOffset = Data_UInt16(valueRecord, valueOffset);

        if (deviceOffset) {
            Data deviceTable = Data_Subdata(parentTable, deviceOffset);
            SFInt32 adjustment;

            adjustment = GetYDeltaPixels(textProcessor, deviceTable);
            SFAlbumAddY(album, inputIndex, adjustment);
        }

        valueOffset += 2;
    }

    if (ValueFormat_XAdvDevice(valueFormat)) {
        Offset16 deviceOffset = Data_UInt16(valueRecord, valueOffset);

        switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            if (deviceOffset) {
                Data deviceTable = Data_Subdata(parentTable, deviceOffset);
                SFInt32 adjustment;

                adjustment = GetXDeltaPixels(textProcessor, deviceTable);
                SFAlbumAddAdvance(album, inputIndex, adjustment);
            }
            break;
        }

        valueOffset += 2;
    }

    if (ValueFormat_YAdvDevice(valueFormat)) {
        valueOffset += 2;
    }
}

static SFBoolean ApplySinglePos(TextProcessorRef textProcessor, Data singlePos)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 posFormat;

    posFormat = SinglePos_Format(singlePos);

    switch (posFormat) {
    case 1: {
        Data coverage = SinglePosF1_CoverageTable(singlePos);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex != SFInvalidIndex) {
            SFUInt16 valueFormat = SinglePosF1_ValueFormat(singlePos);
            Data valueRecord = SinglePosF1_ValueRecord(singlePos);

            ApplyValueRecord(textProcessor, singlePos, valueRecord, valueFormat, locator->index);

            return SFTrue;
        }

        return SFFalse;
    }

    case 2: {
        Data coverage = SinglePosF2_CoverageTable(singlePos);
        SFUInt16 valueFormat = SinglePosF2_ValueFormat(singlePos);
        SFUInt16 valueCount = SinglePosF2_ValueCount(singlePos);
        SFGlyphID locGlyph;
        SFUInteger covIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        covIndex = SearchCoverageIndex(coverage, locGlyph);

        if (covIndex < valueCount) {
            SFUInteger valueSize = ValueRecord_Size(valueFormat);
            Data valueRecord = SinglePosF2_ValueRecord(singlePos, covIndex, valueSize);

            ApplyValueRecord(textProcessor, singlePos, valueRecord, valueFormat, locator->index);

            return SFTrue;
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static int PairRecordGlyphComparison(const void *item1, const void *item2)
{
    SFGlyphID *ref1 = (SFGlyphID *)item1;
    Data ref2 = (Data)item2;
    SFUInt16 val1 = *ref1;
    SFGlyphID val2 = PairValueRecord_SecondGlyph(ref2);

    return val1 - val2;
}

static SFBoolean ApplyPairPos(TextProcessorRef textProcessor, Data pairPos)
{
    LocatorRef locator = &textProcessor->_locator;
    SFBoolean didPosition = SFFalse;
    SFBoolean shouldSkip = SFFalse;
    SFUInteger firstIndex;
    SFUInteger secondIndex;

    firstIndex = locator->index;
    secondIndex = LocatorGetAfter(locator, firstIndex, SFTrue);

    /* Proceed only if pair glyph is available. */
    if (secondIndex != SFInvalidIndex) {
        SFUInt16 format = PairPos_Format(pairPos);

        switch (format) {
        case 1:
            didPosition = ApplyPairPosF1(textProcessor, pairPos, firstIndex, secondIndex, &shouldSkip);
            break;

        case 2:
            didPosition = ApplyPairPosF2(textProcessor, pairPos, firstIndex, secondIndex, &shouldSkip);
            break;
        }
    }

    if (shouldSkip) {
        LocatorJumpTo(locator, secondIndex);
    }

    return didPosition;
}

static SFBoolean ApplyPairPosF1(TextProcessorRef textProcessor, Data pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFAlbumRef album = textProcessor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    Data coverage;
    SFUInt16 pairSetCount;
    SFUInteger covIndex;

    *outShouldSkip = SFFalse;

    coverage = PairPosF1_CoverageTable(pairPos);
    pairSetCount = PairPosF1_PairSetCount(pairPos);
    covIndex = SearchCoverageIndex(coverage, firstGlyph);

    if (covIndex < pairSetCount) {
        SFUInt16 valueFormat1 = PairPosF1_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = PairPosF1_ValueFormat2(pairPos);
        Data pairSet = PairPosF1_PairSetTable(pairPos, covIndex);
        SFUInt16 valueCount = PairSet_PairValueCount(pairSet);
        Data recordArray = PairSet_PairValueRecordArray(pairSet);
        SFUInteger value1Size = ValueRecord_Size(valueFormat1);
        SFUInteger value2Size = ValueRecord_Size(valueFormat2);
        SFUInteger recordSize = PairValueRecord_Size(value1Size, value2Size);
        Data pairRecord;

        pairRecord = bsearch(&secondGlyph, recordArray, valueCount, recordSize, PairRecordGlyphComparison);

        if (pairRecord) {
            if (value1Size) {
                Data value1 = PairValueRecord_Value1(pairRecord);
                ApplyValueRecord(textProcessor, pairSet, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                Data value2 = PairValueRecord_Value2(pairRecord, value1Size);
                ApplyValueRecord(textProcessor, pairSet, value2, valueFormat2, secondIndex);

                /*
                 * Pair element should be skipped only if the value record for the second glyph
                 * is AVAILABLE.
                 */
                *outShouldSkip = SFTrue;
            }

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean ApplyPairPosF2(TextProcessorRef textProcessor, Data pairPos,
    SFUInteger firstIndex, SFUInteger secondIndex, SFBoolean *outShouldSkip)
{
    SFAlbumRef album = textProcessor->_album;
    SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
    SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
    Data coverage;
    SFUInteger covIndex;

    *outShouldSkip = SFFalse;

    coverage = PairPosF2_CoverageTable(pairPos);
    covIndex = SearchCoverageIndex(coverage, firstGlyph);

    if (covIndex != SFInvalidIndex) {
        SFUInt16 valueFormat1 = PairPosF2_ValueFormat1(pairPos);
        SFUInt16 valueFormat2 = PairPosF2_ValueFormat2(pairPos);
        Data classDef1 = PairPosF2_ClassDef1Table(pairPos);
        Data classDef2 = PairPosF2_ClassDef2Table(pairPos);
        SFUInt16 class1Count = PairPosF2_Class1Count(pairPos);
        SFUInt16 class2Count = PairPosF2_Class2Count(pairPos);
        SFUInt16 class1Value;
        SFUInt16 class2Value;

        class1Value = SearchGlyphClass(classDef1, firstGlyph);
        class2Value = SearchGlyphClass(classDef2, secondGlyph);

        if (class1Value < class1Count && class2Value < class2Count) {
            SFUInteger value1Size = ValueRecord_Size(valueFormat1);
            SFUInteger value2Size = ValueRecord_Size(valueFormat2);
            SFUInteger class2Size = Class2Record_Size(value1Size, value2Size);
            SFUInteger class1Size = Class1Record_Size(class2Count, class2Size);
            Data class1Record = PairPosF2_Class1Record(pairPos, class1Value, class1Size);
            Data class2Record = Class1Record_Class2Record(class1Record, class2Value, class2Size);

            if (value1Size) {
                Data value1 = Class2Record_Value1(class2Record);
                ApplyValueRecord(textProcessor, pairPos, value1, valueFormat1, firstIndex);
            }

            if (value2Size) {
                Data value2 = Class2Record_Value2(class2Record, value1Size);
                ApplyValueRecord(textProcessor, pairPos, value2, valueFormat2, secondIndex);

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

static SFPoint ConvertAnchorToPoint(TextProcessorRef textProcessor, Data anchor)
{
    SFUInt16 format;
    SFPoint point;

    format = Anchor_Format(anchor);

    switch (format) {
    case 1: {
        point.x = AnchorF1_XCoordinate(anchor);
        point.y = AnchorF1_YCoordinate(anchor);
        break;
    }

    case 2: {
        point.x = AnchorF2_XCoordinate(anchor);
        point.y = AnchorF2_YCoordinate(anchor);
        /* TODO: Add support for contour point. */
        break;
    }

    case 3: {
        Offset16 xDeviceOffset = AnchorF3_XDeviceOffset(anchor);
        Offset16 yDeviceOffset = AnchorF3_YDeviceOffset(anchor);

        point.x = AnchorF3_XCoordinate(anchor);
        point.y = AnchorF3_YCoordinate(anchor);

        if (xDeviceOffset) {
            Data deviceTable = Data_Subdata(anchor, xDeviceOffset);
            point.x += GetXDeltaPixels(textProcessor, deviceTable);
        }

        if (yDeviceOffset) {
            Data deviceTable = Data_Subdata(anchor, yDeviceOffset);
            point.y += GetYDeltaPixels(textProcessor, deviceTable);
        }
        break;
    }

    default:
        point.x = 0;
        point.y = 0;
        break;
    }

    return point;
}

static void SearchCursiveAnchors(Data cursivePos, SFGlyphID glyph,
    Data *refExitAnchor, Data *refEntryAnchor)
{
    Data coverage = CursivePos_CoverageTable(cursivePos);
    SFUInt16 entryExitCount = CursivePos_EntryExitCount(cursivePos);
    SFUInteger entryExitIndex;

    entryExitIndex = SearchCoverageIndex(coverage, glyph);

    if (entryExitIndex < entryExitCount) {
        Data entryExitRecord = CursivePos_EntryExitRecord(cursivePos, entryExitIndex);
        Offset16 exitAnchorOffset = EntryExitRecord_ExitAnchorOffset(entryExitRecord);
        Offset16 entryAnchorOffset = EntryExitRecord_EntryAnchorOffset(entryExitRecord);

        if (refExitAnchor && exitAnchorOffset) {
            *refExitAnchor = Data_Subdata(cursivePos, exitAnchorOffset);
        }

        if (refEntryAnchor && entryAnchorOffset) {
            *refEntryAnchor = Data_Subdata(cursivePos, entryAnchorOffset);
        }
    }
}

static SFBoolean ApplyCursivePos(TextProcessorRef textProcessor, Data cursivePos)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 posFormat;

    posFormat = CursivePos_Format(cursivePos);

    switch (posFormat) {
    case 1: {
        SFUInteger firstIndex = locator->index;
        SFGlyphID firstGlyph = SFAlbumGetGlyph(album, firstIndex);
        Data exitAnchor = NULL;

        SearchCursiveAnchors(cursivePos, firstGlyph, &exitAnchor, NULL);

        /* Proceed only if exit anchor of first glyph exists. */
        if (exitAnchor) {
            SFUInteger secondIndex = LocatorGetAfter(locator, firstIndex, SFTrue);

            if (secondIndex != SFInvalidIndex) {
                SFGlyphID secondGlyph = SFAlbumGetGlyph(album, secondIndex);
                Data entryAnchor = NULL;

                SearchCursiveAnchors(cursivePos, secondGlyph, NULL, &entryAnchor);

                /* Proceed only if entry anchor of second glyph exists. */
                if (entryAnchor) {
                    return ApplyCursiveAnchors(textProcessor, exitAnchor, entryAnchor, firstIndex, secondIndex);
                }
            }
        }
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyCursiveAnchors(TextProcessorRef textProcessor,
    Data exitAnchor, Data entryAnchor, SFUInteger firstIndex, SFUInteger secondIndex)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFPoint exitPoint = ConvertAnchorToPoint(textProcessor, exitAnchor);
    SFPoint entryPoint = ConvertAnchorToPoint(textProcessor, entryAnchor);
    GlyphTraits traits;
    SFInt32 offset;
    SFAdvance advance;

    traits = GlyphTraitCursive;

    switch (textProcessor->_textDirection) {
    case SFTextDirectionLeftToRight:
        /*
         * PROCESS:
         *      - Set advance of first glyph in such a way that it ends at exit x.
         *      - Set x of second glyph in such a way that it starts at entry x while preserving its
         *        advance.
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
        if (locator->filter.lookupFlag & LookupFlagRightToLeft) {
            traits |= GlyphTraitRightToLeft;
            SFAlbumSetY(album, secondIndex, entryPoint.y - exitPoint.y);
        } else {
            SFAlbumSetY(album, secondIndex, exitPoint.y - entryPoint.y);
        }
        break;

    case SFTextDirectionRightToLeft:
        /*
         * REMARKS:
         *      - In case of RTL, the direction of a glyph is reversed. So, it starts from advance
         *        and ends at zero.
         * PROCESS:
         *      - Set advance of second glyph in such a way that it ends at entry x.
         *      - Set x of first glyph in such a way that it starts at exit x while preserving its
         *        advance.
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
        if (locator->filter.lookupFlag & LookupFlagRightToLeft) {
            traits |= GlyphTraitRightToLeft;
            SFAlbumSetY(album, firstIndex, entryPoint.y - exitPoint.y);
        } else {
            SFAlbumSetY(album, firstIndex, exitPoint.y - entryPoint.y);
        }
        break;
    }

    /* Update the details of first glyph. */
    SFAlbumSetCursiveOffset(album, firstIndex, (SFUInt16)(secondIndex - firstIndex));
    SFAlbumInsertHelperTraits(album, firstIndex, traits);

    /* Update the details of second glyph. */
    SFAlbumSetCursiveOffset(album, secondIndex, 0);
    SFAlbumInsertHelperTraits(album, secondIndex, traits);

    return SFTrue;
}

static Data GetMarkArrayFromAnchorTable(Data markArray, SFUInteger markIndex, SFUInt16 *outClass)
{
    SFUInt16 markCount = MarkArray_MarkCount(markArray);

    if (markIndex < markCount) {
        Data markRecord = MarkArray_MarkRecord(markArray, markIndex);
        SFUInt16 classValue = MarkRecord_Class(markRecord);
        Offset16 anchorOffset = MarkRecord_MarkAnchorOffset(markRecord);
        Data markAnchor = Data_Subdata(markArray, anchorOffset);

        *outClass = classValue;
        return markAnchor;
    }

    *outClass = 0;
    return NULL;
}

static SFBoolean ApplyMarkToBasePos(TextProcessorRef textProcessor, Data markBasePos)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 posFormat;

    posFormat = MarkBasePos_Format(markBasePos);

    switch (posFormat) {
    case 1: {
        Data markCoverage = MarkBasePos_MarkCoverageTable(markBasePos);
        SFGlyphID locGlyph;
        SFUInteger markIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        markIndex = SearchCoverageIndex(markCoverage, locGlyph);

        if (markIndex != SFInvalidIndex) {
            SFUInteger prevIndex = LocatorGetPrecedingBaseIndex(locator);
            SFGlyphID prevGlyph;

            /* Proceed only if there is a previous base glyph. */
            if (prevIndex != SFInvalidIndex) {
                Data baseCoverage = MarkBasePos_BaseCoverageTable(markBasePos);
                SFUInteger baseIndex;

                prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                baseIndex = SearchCoverageIndex(baseCoverage, prevGlyph);

                if (baseIndex != SFInvalidIndex) {
                    return ApplyMarkToBaseArrays(textProcessor, markBasePos, markIndex, baseIndex, prevIndex);
                }
            }
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyMarkToBaseArrays(TextProcessorRef textProcessor, Data markBasePos,
    SFUInteger markIndex, SFUInteger baseIndex, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 classCount;
    Data markArray;
    SFUInt16 classValue;
    Data markAnchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < locator->index);

    classCount = MarkBasePos_ClassCount(markBasePos);
    markArray = MarkBasePos_MarkArrayTable(markBasePos);

    /* Get mark anchor and its class value. */
    markAnchor = GetMarkArrayFromAnchorTable(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        Data baseArray = MarkBasePos_BaseArrayTable(markBasePos);
        SFUInt16 baseCount;

        baseCount = BaseArray_BaseCount(baseArray);

        /* Validate base index. */
        if (baseIndex < baseCount) {
            Data baseRecord = BaseArray_BaseRecord(baseArray, baseIndex, classCount);
            Offset16 anchorOffset = BaseArray_BaseAnchorOffset(baseRecord, classValue);
            Data baseAnchor = Data_Subdata(baseArray, anchorOffset);
            SFPoint markPoint;
            SFPoint basePoint;

            /* Get mark and base points from their respective anchors. */
            markPoint = ConvertAnchorToPoint(textProcessor, markAnchor);
            basePoint = ConvertAnchorToPoint(textProcessor, baseAnchor);

            /* Connect mark glyph with base glyph. */
            SFAlbumSetX(album, locator->index, basePoint.x - markPoint.x);
            SFAlbumSetY(album, locator->index, basePoint.y - markPoint.y);
            /* Update the details of mark glyph. */
            SFAlbumSetAttachmentOffset(album, locator->index, (SFUInt16)(locator->index - attachmentIndex));
            SFAlbumInsertHelperTraits(album, locator->index, GlyphTraitAttached);

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean ApplyMarkToLigPos(TextProcessorRef textProcessor, Data markLigPos)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 posFormat;

    posFormat = MarkLigPos_Format(markLigPos);

    switch (posFormat) {
    case 1: {
        Data markCoverage = MarkLigPos_MarkCoverageTable(markLigPos);
        SFGlyphID locGlyph;
        SFUInteger markIndex;

        locGlyph = SFAlbumGetGlyph(album, locator->index);
        markIndex = SearchCoverageIndex(markCoverage, locGlyph);

        if (markIndex != SFInvalidIndex) {
            SFUInteger prevIndex;
            SFUInteger ligComponent;
            SFGlyphID prevGlyph;

            prevIndex = LocatorGetPrecedingLigatureIndex(locator, &ligComponent);

            /* Proceed only if there is a previous ligature glyph. */
            if (prevIndex != SFInvalidIndex) {
                Data ligCoverage = MarkLigPos_LigatureCoverageTable(markLigPos);
                SFUInteger ligIndex;

                prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                ligIndex = SearchCoverageIndex(ligCoverage, prevGlyph);

                if (ligIndex != SFInvalidIndex) {
                    return ApplyMarkToLigArrays(textProcessor, markLigPos, markIndex, ligIndex, ligComponent, prevIndex);
                }
            }
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyMarkToLigArrays(TextProcessorRef textProcessor, Data markLigPos,
    SFUInteger markIndex, SFUInteger ligIndex, SFUInteger ligComponent, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 classCount;
    Data markArray;
    SFUInt16 classValue;
    Data markAnchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < locator->index);

    classCount = MarkLigPos_ClassCount(markLigPos);
    markArray = MarkLigPos_MarkArrayTable(markLigPos);

    /* Get mark anchor and its class value. */
    markAnchor = GetMarkArrayFromAnchorTable(markArray, markIndex, &classValue);
    /* Validate mark anchor and its class value. */
    if (markAnchor && classValue < classCount) {
        Data ligArray = MarkLigPos_LigatureArrayTable(markLigPos);
        SFUInt16 ligCount = LigatureArray_LigatureCount(ligArray);

        /* Validate ligature index. */
        if (ligIndex < ligCount) {
            Data ligAttach = LigatureArray_LigatureAttachTable(ligArray, ligIndex);
            SFUInteger compCount = LigatureAttach_ComponentCount(ligAttach);
            Data compRecord;
            Offset16 anchorOffset;
            Data ligAnchor;
            SFPoint markPoint;
            SFPoint ligPoint;

            /* Use last component in case of error. */
            if (ligComponent >= compCount) {
                ligComponent = compCount - 1;
            }

            compRecord = LigatureAttach_ComponentRecord(ligAttach, ligComponent, classCount);
            anchorOffset = ComponentRecord_LigatureAnchorOffset(compRecord, classValue);
            ligAnchor = Data_Subdata(ligAttach, anchorOffset);

            /* Get mark and ligature points from their respective anchors. */
            markPoint = ConvertAnchorToPoint(textProcessor, markAnchor);
            ligPoint = ConvertAnchorToPoint(textProcessor, ligAnchor);

            /* Connect mark glyph with ligature glyph. */
            SFAlbumSetX(album, locator->index, ligPoint.x - markPoint.x);
            SFAlbumSetY(album, locator->index, ligPoint.y - markPoint.y);
            /* Update the details of mark glyph. */
            SFAlbumSetAttachmentOffset(album, locator->index, (SFUInt16)(locator->index - attachmentIndex));
            SFAlbumInsertHelperTraits(album, locator->index, GlyphTraitAttached);

            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean ApplyMarkToMarkPos(TextProcessorRef textProcessor, Data markMarkPos)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, locator->index);
    SFUInt16 posFormat;

    posFormat = MarkMarkPos_Format(markMarkPos);

    switch (posFormat) {
    case 1: {
        Data mark1Coverage = MarkMarkPos_Mark1CoverageTable(markMarkPos);
        SFUInteger mark1Index;

        mark1Index = SearchCoverageIndex(mark1Coverage, inputGlyph);

        if (mark1Index != SFInvalidIndex) {
            SFUInteger prevIndex = LocatorGetPrecedingMarkIndex(locator);
            SFGlyphID prevGlyph;

            /* Proceed only if there is a previous mark glyph. */
            if (prevIndex != SFInvalidIndex) {
                Data mark2Coverage = MarkMarkPos_Mark2CoverageTable(markMarkPos);
                SFUInteger mark2Index;

                prevGlyph = SFAlbumGetGlyph(album, prevIndex);
                mark2Index = SearchCoverageIndex(mark2Coverage, prevGlyph);

                if (mark2Index != SFInvalidIndex) {
                    return ApplyMarkToMarkArrays(textProcessor, markMarkPos, mark1Index, mark2Index, prevIndex);
                }
            }
        }

        return SFFalse;
    }

    default:
        /* Invalid table format. */
        return SFFalse;
    }
}

static SFBoolean ApplyMarkToMarkArrays(TextProcessorRef textProcessor, Data markMarkPos,
    SFUInteger mark1Index, SFUInteger mark2Index, SFUInteger attachmentIndex)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 classCount;
    Data mark1Array;
    SFUInt16 classValue;
    Data mark1Anchor;

    /* Attachment index MUST be less than input index. */
    SFAssert(attachmentIndex < locator->index);

    classCount = MarkMarkPos_ClassCount(markMarkPos);
    mark1Array = MarkMarkPos_Mark1ArrayTable(markMarkPos);

    /* Get mark anchor and its class value. */
    mark1Anchor = GetMarkArrayFromAnchorTable(mark1Array, mark1Index, &classValue);
    /* Validate mark anchor and its class value. */
    if (mark1Anchor && classValue < classCount) {
        Data mark2Array = MarkMarkPos_Mark2ArrayTable(markMarkPos);
        SFUInt16 mark2Count = Mark2Array_Mark2Count(mark2Array);

        /* Validate mark 2 index. */
        if (mark2Index < mark2Count) {
            Data mark2Record = Mark2Array_Mark2Record(mark2Array, mark2Index, classCount);
            Offset16 anchorOffset = Mark2Record_Mark2AnchorOffset(mark2Record, classValue);
            Data mark2Anchor = Data_Subdata(mark2Array, anchorOffset);
            SFPoint mark1Point;
            SFPoint mark2Point;

            /* Get mark and base points from their respective anchors. */
            mark1Point = ConvertAnchorToPoint(textProcessor, mark1Anchor);
            mark2Point = ConvertAnchorToPoint(textProcessor, mark2Anchor);

            /* Connect mark1 glyph with mark2 glyph. */
            SFAlbumSetX(album, locator->index, mark2Point.x - mark1Point.x);
            SFAlbumSetY(album, locator->index, mark2Point.y - mark1Point.y);
            /* Update the details of mark1 glyph. */
            SFAlbumSetAttachmentOffset(album, locator->index, (SFUInt16)(locator->index - attachmentIndex));
            SFAlbumInsertHelperTraits(album, locator->index, GlyphTraitAttached);

            return SFTrue;
        }
    }

    return SFFalse;
}

SF_PRIVATE SFBoolean ApplyPositioningSubtable(TextProcessorRef textProcessor, LookupType lookupType, Data subtable)
{
    switch (lookupType) {
    case LookupTypeSingleAdjustment:
        return ApplySinglePos(textProcessor, subtable);

    case LookupTypePairAdjustment:
        return ApplyPairPos(textProcessor, subtable);

    case LookupTypeCursiveAttachment:
        return ApplyCursivePos(textProcessor, subtable);

    case LookupTypeMarkToBaseAttachment:
        return ApplyMarkToBasePos(textProcessor, subtable);

    case LookupTypeMarkToLigatureAttachment:
        return ApplyMarkToLigPos(textProcessor, subtable);

    case LookupTypeMarkToMarkAttachment:
        return ApplyMarkToMarkPos(textProcessor, subtable);

    case LookupTypeContextPositioning:
        return ApplyContextSubtable(textProcessor, subtable);

    case LookupTypeChainedContextPositioning:
        return ApplyChainContextSubtable(textProcessor, subtable);

    case LookupTypeExtensionPositioning:
        return ApplyExtensionSubtable(textProcessor, subtable);

    default:
        /* Invalid lookup type. */
        return SFFalse;
    }
}

static void ResolveLeftCursiveSegment(TextProcessorRef textProcessor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For left-to-right cursively attached segment, first glyph is positioned on BASELINE
     *      pushing next glyphs DOWNWARD.
     */

    SFAlbumRef album = textProcessor->_album;
    SFUInteger offset;

    /* The glyph MUST be cursive and right-to-left. */
    SFAssert(SFAlbumGetAllTraits(album, inputIndex) & GlyphTraitCursive);
    /* The glyph must NOT be right-to-left. */
    SFAssert(!(SFAlbumGetAllTraits(album, inputIndex) & GlyphTraitRightToLeft));
    /* The glyph must NOT be resolved yet. */
    SFAssert(!(SFAlbumGetAllTraits(album, inputIndex) & GlyphTraitResolved));

    offset = SFAlbumGetCursiveOffset(album, inputIndex);

    if (offset) {
        SFUInteger nextIndex = inputIndex + offset;
        SFInt32 inputY;
        SFInt32 nextY;

        switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            inputY = SFAlbumGetY(album, inputIndex);
            nextY = SFAlbumGetY(album, nextIndex);

            SFAlbumSetY(album, nextIndex, nextY + inputY);
            break;
        }

        ResolveLeftCursiveSegment(textProcessor, nextIndex);

        /* Mark this glyph as resolved. */
        SFAlbumInsertHelperTraits(album, inputIndex, GlyphTraitResolved);
    }
}

static void ResolveRightCursiveSegment(TextProcessorRef textProcessor, SFUInteger inputIndex)
{
    /*
     * REMARKS:
     *      For right-to-left cursively attached segment, last glyph is positioned on BASELINE,
     *      pushing previous glyphs UPWARD.
     */

    SFAlbumRef album = textProcessor->_album;
    SFUInteger offset;

    /* The glyph MUST be cursive and right-to-left. */
    SFAssert(SFAlbumGetAllTraits(album, inputIndex) & (GlyphTraitCursive | GlyphTraitRightToLeft));
    /* The glyph must NOT be resolved yet. */
    SFAssert(!(SFAlbumGetAllTraits(album, inputIndex) & GlyphTraitResolved));

    offset = SFAlbumGetCursiveOffset(album, inputIndex);

    if (offset) {
        SFUInteger nextIndex = inputIndex + offset;
        SFInt32 inputY;
        SFInt32 nextY;

        ResolveRightCursiveSegment(textProcessor, nextIndex);

        switch (textProcessor->_textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            inputY = SFAlbumGetY(album, inputIndex);
            nextY = SFAlbumGetY(album, nextIndex);

            SFAlbumSetY(album, inputIndex, inputY + nextY);
            break;
        }

        /* Mark this glyph as resolved. */
        SFAlbumInsertHelperTraits(album, inputIndex, GlyphTraitResolved);
    }
}

static void ResolveCursivePositions(TextProcessorRef textProcessor, LocatorRef locator)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorReset(locator, 0, album->glyphCount);

    while (LocatorMoveNext(locator)) {
        SFUInteger locatorIndex = locator->index;
        GlyphTraits traits = SFAlbumGetAllTraits(album, locatorIndex);

        if ((traits & (GlyphTraitCursive | GlyphTraitResolved)) == GlyphTraitCursive) {
            if (traits & GlyphTraitRightToLeft) {
                ResolveRightCursiveSegment(textProcessor, locator->index);
            } else {
                ResolveLeftCursiveSegment(textProcessor, locator->index);
            }
        }
    }
}

static void ResolveMarkPositions(TextProcessorRef textProcessor, LocatorRef locator)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorReset(locator, 0, album->glyphCount);

    while (LocatorMoveNext(locator)) {
        SFUInteger locIndex = locator->index;
        GlyphTraits traits = SFAlbumGetAllTraits(album, locIndex);

        if (traits & GlyphTraitAttached) {
            SFUInteger attachmentIndex = locIndex - SFAlbumGetAttachmentOffset(album, locIndex);
            SFInt32 markX = SFAlbumGetX(album, locIndex);
            SFInt32 markY = SFAlbumGetY(album, locIndex);
            SFUInteger index;

            /* Put the mark glyph OVER attached glyph. */
            markX += SFAlbumGetX(album, attachmentIndex);
            markY += SFAlbumGetY(album, attachmentIndex);

            /* Close the gap between the mark glyph and previous glyph. */
            switch (textProcessor->_textDirection) {
            case SFTextDirectionLeftToRight:
                for (index = attachmentIndex; index < locIndex; index++) {
                    markX -= SFAlbumGetAdvance(album, index);
                }
                break;

            case SFTextDirectionRightToLeft:
                for (index = attachmentIndex + 1; index <= locIndex; index++) {
                    markX += SFAlbumGetAdvance(album, index);
                }
                break;
            }

            /* Update the position of mark glyph. */
            SFAlbumSetX(album, locIndex, markX);
            SFAlbumSetY(album, locIndex, markY);
        }
    }
}

SF_PRIVATE void ResolveAttachments(TextProcessorRef textProcessor)
{
    SFAlbumRef album = textProcessor->_album;
    Locator locator;

    LocatorInitialize(&locator, album, NULL);

    ResolveCursivePositions(textProcessor, &locator);
    ResolveMarkPositions(textProcessor, &locator);
}
