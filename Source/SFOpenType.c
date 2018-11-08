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
#include <stdlib.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFVariations.h"
#include "SFOpenType.h"

static int _SFUInt16ItemsComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 val2 = SFData_UInt16(ref2, 0);

    return (int)val1 - (int)val2;
}

static SFUInteger _SFBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    void *item = bsearch(&value, uint16Array, length, sizeof(SFUInt16), _SFUInt16ItemsComparison);
    if (!item) {
        return SFInvalidIndex;
    }

    return (SFUInteger)((SFData)item - uint16Array) / sizeof(SFUInt16);
}

static int _SFGlyphRangeComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 rangeStart = SFGlyphRange_Start(ref2);
    SFUInt16 rangeEnd = SFGlyphRange_End(ref2);

    if (val1 < rangeStart) {
        return -1;
    }

    if (val1 > rangeEnd) {
        return 1;
    }

    return 0;
}

static SFData _SFBinarySearchGlyphRange(SFData rangeArray, SFUInteger length, SFUInt16 value)
{
    return bsearch(&value, rangeArray, length, SFGlyphRange_Size(), _SFGlyphRangeComparison);
}

SF_INTERNAL SFUInteger SFOpenTypeSearchCoverageIndex(SFData coverageTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The coverage table must NOT be null. */
    SFAssert(coverageTable != NULL);

    format = SFCoverage_Format(coverageTable);

    switch (format) {
        case 1: {
            SFUInt16 glyphCount = SFCoverageF1_GlyphCount(coverageTable);
            SFData glyphArray = SFCoverageF1_GlyphArray(coverageTable);

            return _SFBinarySearchUInt16(glyphArray, glyphCount, glyphID);
        }

        case 2: {
            SFUInt16 rangeCount = SFCoverageF2_RangeCount(coverageTable);
            SFData rangeArray = SFCoverageF2_GlyphRangeArray(coverageTable);
            SFData rangeRecord;

            rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                SFUInt16 startGlyphID = SFRangeRecord_StartGlyphID(rangeRecord);
                SFUInt16 startCoverageIndex = SFRangeRecord_StartCoverageIndex(rangeRecord);

                return (SFUInteger)(startCoverageIndex) + (glyphID - startGlyphID);
            }
            break;
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFUInt16 SFOpenTypeSearchGlyphClass(SFData classDefTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The class definition table must NOT be null. */
    SFAssert(classDefTable != NULL);

    format = SFClassDef_Format(classDefTable);

    switch (format) {
        case 1: {
            SFGlyphID startGlyphID = SFClassDefF1_StartGlyphID(classDefTable);
            SFUInt16 glyphCount = SFClassDefF1_GlyphCount(classDefTable);
            SFUInteger valueIndex;

            valueIndex = glyphID - startGlyphID;

            if (valueIndex < glyphCount) {
                SFData classArray = SFClassDefF1_ClassValueArray(classDefTable);
                return SFUInt16Array_Value(classArray, valueIndex);
            }
            break;
        }

        case 2: {
            SFUInt16 rangeCount = SFClassDefF2_ClassRangeCount(classDefTable);
            SFData rangeArray = SFClassDefF2_GlyphRangeArray(classDefTable);
            SFData rangeRecord;

            rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                return SFClassRangeRecord_Class(rangeRecord);
            }
            break;
        }
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;
}

SF_INTERNAL SFInt32 SFOpenTypeGetDevicePixels(SFData deviceTable, SFUInt16 ppemSize)
{
    SFUInt16 startSize = SFDevice_StartSize(deviceTable);
    SFUInt16 endSize = SFDevice_EndSize(deviceTable);
    SFUInt16 deltaFormat = SFDevice_DeltaFormat(deviceTable);

    if (ppemSize >= startSize && ppemSize <= endSize) {
        SFUInt16 sizeIndex = ppemSize - startSize;

        switch (deltaFormat) {
            /* Signed 2-bit value, 8 values per UInt16 */
            case 1: {
                SFUInt16 valueIndex = sizeIndex >> 3;
                SFUInt16 leftShift = 16 + (2 * (sizeIndex & 0x0007));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 30;
            }

            /* Signed 4-bit value, 4 values per UInt16 */
            case 2: {
                SFUInt16 valueIndex = sizeIndex >> 2;
                SFUInt16 leftShift = 16 + (4 * (sizeIndex & 0x0003));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 28;
            }

            /* Signed 8-bit value, 2 values per UInt16 */
            case 3: {
                SFUInt16 valueIndex = sizeIndex >> 1;
                SFUInt16 leftShift = 16 + (8 * (sizeIndex & 0x0001));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 24;
            }
        }
    }

    return 0;
}

static double CalculateScalarForRegion(SFData regionListTable, SFUInt16 regionIndex,
    SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 axisCount = SFVarRegionList_AxisCount(regionListTable);
    SFUInt16 regionCount = SFVarRegionList_RegionCount(regionListTable);
    double regionScalar = 1.0;

    if (regionIndex < regionCount) {
        SFData regionRecord = SFVarRegionList_VarRegionRecord(regionListTable, regionIndex, axisCount);
        SFUInt16 axisIndex;

        for (axisIndex = 0; axisIndex < axisCount; axisIndex++) {
            SFData axisCoords = SFVarRegionRecord_RegionAxisCoords(regionRecord, axisIndex);
            SFInt16 startCoord = SFRegionAxisCoords_StartCoord(axisCoords);
            SFInt16 peakCoord = SFRegionAxisCoords_PeakCoord(axisCoords);
            SFInt16 endCoord = SFRegionAxisCoords_EndCoord(axisCoords);
            SFInt32 instanceCoord = (axisIndex < coordCount ? coordArray[axisIndex] : 0);
            double axisScalar;

            if (startCoord > peakCoord || peakCoord > endCoord) {
                axisScalar = 1.0;
            } else if (startCoord < 0 && endCoord > 0 && peakCoord != 0) {
                axisScalar = 1.0;
            } else if (peakCoord == 0) {
                axisScalar = 1.0;
            } else if (instanceCoord < startCoord || instanceCoord > endCoord) {
                axisScalar = 0.0;
            } else {
                if (instanceCoord == peakCoord) {
                    axisScalar = 1.0;
                } else if (instanceCoord < peakCoord) {
                    axisScalar = (double)(instanceCoord - startCoord) / (peakCoord - startCoord);
                } else {
                    axisScalar = (double)(endCoord - instanceCoord) / (endCoord - peakCoord);
                }
            }

            regionScalar *= axisScalar;
        }
    }

    return regionScalar;
}

static SFInt32 CalculateVariationAdjustment(SFData varDataTable, SFData regionListTable,
    SFUInt16 rowIndex, SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 itemCount = SFItemVarData_ItemCount(varDataTable);
    SFUInt16 shortDeltaCount = SFItemVarData_ShortDeltaCount(varDataTable);
    SFUInt16 regionCount = SFItemVarData_RegionIndexCount(varDataTable);
    double adjustment = 0.0;

    if (rowIndex < itemCount) {
        SFUInteger recordSize = SFDeltaSetRecord_Size(shortDeltaCount, regionCount);
        SFData rowsArray = SFItemVarData_DeltaSetRowsArray(varDataTable, regionCount);
        SFData deltaSet = SFDeltaSetRowsArray_DeltaSetRecord(rowsArray, rowIndex, recordSize);
        SFUInt16 valueIndex;

        for (valueIndex = 0; valueIndex < regionCount; valueIndex++) {
            SFUInt16 regionIndex = SFItemVarData_RegionIndexItem(varDataTable, valueIndex);
            SFInt16 delta;

            if (valueIndex < shortDeltaCount) {
                delta = SFDeltaSetRecord_I16Delta(deltaSet, valueIndex);
            } else {
                delta = SFDeltaSetRecord_I8Delta(deltaSet, shortDeltaCount, valueIndex - shortDeltaCount);
            }

            adjustment += CalculateScalarForRegion(regionListTable, regionIndex, coordArray, coordCount) * delta;
        }
    }

    return (SFInt32)((adjustment * 0x4000) + 0.5);
}

static SFInt32 GetDeltaFromVariationStore(SFData varStoreTable,
    SFUInt16 dataIndex, SFUInt16 rowIndex, SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 format = SFItemVarStore_Format(varStoreTable);

    switch (format) {
        case 1: {
            SFData regionList = SFItemVarStore_VarRegionListTable(varStoreTable);
            SFUInt16 dataCount = SFItemVarStore_ItemVarDataCount(varStoreTable);

            if (dataIndex < dataCount) {
                SFData varDataTable = SFItemVarStore_ItemVarDataTable(varStoreTable, dataIndex);
                return CalculateVariationAdjustment(varDataTable, regionList, rowIndex, coordArray, coordCount);
            }
            break;
        }
    }

    return 0;
}

SF_INTERNAL SFInt32 SFOpenTypeGetVariationPixels(SFData varIndexTable, SFData varStoreTable,
    SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 outerIndex = SFVarIndex_DeltaSetOuterIndex(varIndexTable);
    SFUInt16 innerIndex = SFVarIndex_DeltaSetInnerIndex(varIndexTable);
    SFUInt16 deltaFormat = SFVarIndex_DeltaFormat(varIndexTable);

    if (deltaFormat == 0x8000) {
        return GetDeltaFromVariationStore(varStoreTable, outerIndex, innerIndex, coordArray, coordCount);
    }

    return 0;
}

static SFBoolean MatchCondition(SFData condTable, SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 format = SFCondition_Format(condTable);

    switch (format) {
        case 1: {
            SFUInt16 axisIndex = SFCondition_AxisIndex(condTable);
            SFInt16 minValue = SFCondition_FilterRangeMinValue(condTable);
            SFInt16 maxValue = SFCondition_FilterRangeMaxValue(condTable);
            SFInt32 coordValue = (axisIndex < coordCount ? coordArray[axisIndex] : 0);

            if (coordValue >= minValue && coordValue <= maxValue) {
                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean MatchConditionSet(SFData condSetTable, SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt16 condCount = SFConditionSet_ConditionCount(condSetTable);
    SFUInt16 condIndex;

    for (condIndex = 0; condIndex < condCount; condIndex++) {
        SFData condTable = SFConditionSet_ConditionTable(condSetTable, condIndex);

        if (!MatchCondition(condTable, coordArray, coordCount)) {
            return SFFalse;
        }
    }

    return SFTrue;
}

SF_INTERNAL SFData SFOpenTypeSearchFeatureSubstitutionTable(SFData featureVarsTable,
    SFInt32 *coordArray, SFUInteger coordCount)
{
    SFUInt32 recordCount = SFFeatureVars_FeatureVarCount(featureVarsTable);
    SFUInt32 recordIndex;

    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        SFData varRecord = SFFeatureVars_FeatureVarRecord(featureVarsTable, recordIndex);
        SFUInt32 condSetOffset = SFFeatureVarRecord_ConditionSetOffset(varRecord);
        SFData condSetTable = SFData_Subdata(featureVarsTable, condSetOffset);

        if (MatchConditionSet(condSetTable, coordArray, coordCount)) {
            SFUInt32 featureSubstOffset = SFFeatureVarRecord_FeatureSubstOffset(varRecord);
            return SFData_Subdata(featureVarsTable, featureSubstOffset);
        }
    }

    return NULL;
}
