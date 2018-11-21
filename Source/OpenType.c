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
#include "Common.h"
#include "Data.h"
#include "Variations.h"
#include "OpenType.h"

SF_INTERNAL Data SearchScriptTable(Data scriptListTable, SFTag scriptTag)
{
    Data scriptTable = NULL;
    SFUInt16 scriptCount;
    SFUInt16 index;

    scriptCount = ScriptList_ScriptCount(scriptListTable);

    for (index = 0; index < scriptCount; index++) {
        Data scriptRecord = ScriptList_ScriptRecord(scriptListTable, index);
        SFTag scriptRecTag = ScriptRecord_ScriptTag(scriptRecord);
        SFOffset scriptOffset;

        if (scriptRecTag == scriptTag) {
            scriptOffset = ScriptRecord_ScriptOffset(scriptRecord);
            scriptTable = Data_Subdata(scriptListTable, scriptOffset);
            break;
        }
    }

    return scriptTable;
}

SF_INTERNAL Data SearchLangSysTable(Data scriptTable, SFTag languageTag)
{
    Data langSysTable = NULL;

    if (languageTag == SFTagMake('d', 'f', 'l', 't')) {
        SFOffset langSysOffset = Script_DefaultLangSysOffset(scriptTable);

        if (langSysOffset) {
            langSysTable = Data_Subdata(scriptTable, langSysOffset);
        }
    } else {
        SFUInt16 langSysCount = Script_LangSysCount(scriptTable);
        SFUInt16 index;

        for (index = 0; index < langSysCount; index++) {
            Data langSysRecord = Script_LangSysRecord(scriptTable, index);
            SFTag langSysTag = LangSysRecord_LangSysTag(langSysRecord);
            SFOffset langSysOffset;

            if (langSysTag == languageTag) {
                langSysOffset = LangSysRecord_LangSysOffset(langSysRecord);
                langSysTable = Data_Subdata(scriptTable, langSysOffset);
                break;
            }
        }
    }

    return langSysTable;
}

SF_INTERNAL Data SearchDefaultFeatureTable(Data langSysTable,
    Data featureListTable, SFTag featureTag, SFUInt16 *featureIndex)
{
    SFUInt16 featureCount = LangSys_FeatureCount(langSysTable);
    SFUInt16 arrayIndex;

    for (arrayIndex = 0; arrayIndex < featureCount; arrayIndex++) {
        SFUInt16 recordIndex = LangSys_FeatureIndex(langSysTable, arrayIndex);
        Data featureRecord = FeatureList_FeatureRecord(featureListTable, recordIndex);
        SFTag recordTag = FeatureRecord_FeatureTag(featureRecord);

        if (recordTag == featureTag) {
            SFOffset featureOffset = FeatureRecord_FeatureOffset(featureRecord);
            Data featureTable = Data_Subdata(featureListTable, featureOffset);

            if (featureIndex) {
                *featureIndex = recordIndex;
            }

            return featureTable;
        }
    }

    return NULL;
}

static int UInt16ItemsComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    Data ref2 = (Data)item2;
    SFUInt16 val2 = Data_UInt16(ref2, 0);

    return (int)val1 - (int)val2;
}

static SFUInteger BinarySearchUInt16(Data uint16Array, SFUInteger length, SFUInt16 value)
{
    void *item = bsearch(&value, uint16Array, length, sizeof(SFUInt16), UInt16ItemsComparison);
    if (!item) {
        return SFInvalidIndex;
    }

    return (SFUInteger)((Data)item - uint16Array) / sizeof(SFUInt16);
}

static int GlyphRangeComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    Data ref2 = (Data)item2;
    SFUInt16 rangeStart = GlyphRange_Start(ref2);
    SFUInt16 rangeEnd = GlyphRange_End(ref2);

    if (val1 < rangeStart) {
        return -1;
    }

    if (val1 > rangeEnd) {
        return 1;
    }

    return 0;
}

static Data BinarySearchGlyphRange(Data rangeArray, SFUInteger length, SFUInt16 value)
{
    return bsearch(&value, rangeArray, length, GlyphRange_Size(), GlyphRangeComparison);
}

SF_INTERNAL SFUInteger SearchCoverageIndex(Data coverageTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The coverage table must NOT be null. */
    SFAssert(coverageTable != NULL);

    format = Coverage_Format(coverageTable);

    switch (format) {
        case 1: {
            SFUInt16 glyphCount = CoverageF1_GlyphCount(coverageTable);
            Data glyphArray = CoverageF1_GlyphArray(coverageTable);

            return BinarySearchUInt16(glyphArray, glyphCount, glyphID);
        }

        case 2: {
            SFUInt16 rangeCount = CoverageF2_RangeCount(coverageTable);
            Data rangeArray = CoverageF2_GlyphRangeArray(coverageTable);
            Data rangeRecord;

            rangeRecord = BinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                SFUInt16 startGlyphID = RangeRecord_StartGlyphID(rangeRecord);
                SFUInt16 startCoverageIndex = RangeRecord_StartCoverageIndex(rangeRecord);

                return (SFUInteger)(startCoverageIndex) + (glyphID - startGlyphID);
            }
            break;
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFUInt16 SearchGlyphClass(Data classDefTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The class definition table must NOT be null. */
    SFAssert(classDefTable != NULL);

    format = ClassDef_Format(classDefTable);

    switch (format) {
        case 1: {
            SFGlyphID startGlyphID = ClassDefF1_StartGlyphID(classDefTable);
            SFUInt16 glyphCount = ClassDefF1_GlyphCount(classDefTable);
            SFUInteger valueIndex;

            valueIndex = glyphID - startGlyphID;

            if (valueIndex < glyphCount) {
                Data classArray = ClassDefF1_ClassValueArray(classDefTable);
                return UInt16Array_Value(classArray, valueIndex);
            }
            break;
        }

        case 2: {
            SFUInt16 rangeCount = ClassDefF2_ClassRangeCount(classDefTable);
            Data rangeArray = ClassDefF2_GlyphRangeArray(classDefTable);
            Data rangeRecord;

            rangeRecord = BinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                return ClassRangeRecord_Class(rangeRecord);
            }
            break;
        }
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;
}

SF_INTERNAL SFInt32 GetDevicePixels(Data deviceTable, SFUInt16 ppemSize)
{
    SFUInt16 startSize = Device_StartSize(deviceTable);
    SFUInt16 endSize = Device_EndSize(deviceTable);
    SFUInt16 deltaFormat = Device_DeltaFormat(deviceTable);

    if (ppemSize >= startSize && ppemSize <= endSize) {
        SFUInt16 sizeIndex = ppemSize - startSize;

        switch (deltaFormat) {
            /* Signed 2-bit value, 8 values per UInt16 */
            case 1: {
                SFUInt16 valueIndex = sizeIndex >> 3;
                SFUInt16 leftShift = 16 + (2 * (sizeIndex & 0x0007));
                SFUInt16 deltaValue = Device_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 30;
            }

            /* Signed 4-bit value, 4 values per UInt16 */
            case 2: {
                SFUInt16 valueIndex = sizeIndex >> 2;
                SFUInt16 leftShift = 16 + (4 * (sizeIndex & 0x0003));
                SFUInt16 deltaValue = Device_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 28;
            }

            /* Signed 8-bit value, 2 values per UInt16 */
            case 3: {
                SFUInt16 valueIndex = sizeIndex >> 1;
                SFUInt16 leftShift = 16 + (8 * (sizeIndex & 0x0001));
                SFUInt16 deltaValue = Device_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 24;
            }
        }
    }

    return 0;
}

SF_INTERNAL double CalculateScalarForRegion(Data regionListTable, SFUInt16 regionIndex,
    const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 axisCount = VarRegionList_AxisCount(regionListTable);
    SFUInt16 regionCount = VarRegionList_RegionCount(regionListTable);
    double regionScalar = 1.0;

    if (regionIndex < regionCount) {
        Data regionRecord = VarRegionList_VarRegionRecord(regionListTable, regionIndex, axisCount);
        SFUInt16 axisIndex;

        /* Reference: https://docs.microsoft.com/en-us/typography/opentype/spec/otvaroverview#algorithm-for-interpolation-of-instance-values */
        for (axisIndex = 0; axisIndex < axisCount; axisIndex++) {
            Data axisCoords = VarRegionRecord_RegionAxisCoords(regionRecord, axisIndex);
            SFInt16 startCoord = RegionAxisCoords_StartCoord(axisCoords);
            SFInt16 peakCoord = RegionAxisCoords_PeakCoord(axisCoords);
            SFInt16 endCoord = RegionAxisCoords_EndCoord(axisCoords);
            SFInt16 instanceCoord = (axisIndex < coordCount ? coordArray[axisIndex] : 0);
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

static double CalculateVariationAdjustment(Data varDataTable, Data regionListTable,
    SFUInt16 rowIndex, const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 itemCount = ItemVarData_ItemCount(varDataTable);
    SFUInt16 shortDeltaCount = ItemVarData_ShortDeltaCount(varDataTable);
    SFUInt16 regionCount = ItemVarData_RegionIndexCount(varDataTable);
    double adjustment = 0.0;

    if (rowIndex < itemCount) {
        SFUInteger recordSize = DeltaSetRecord_Size(shortDeltaCount, regionCount);
        Data rowsArray = ItemVarData_DeltaSetRowsArray(varDataTable, regionCount);
        Data deltaSet = DeltaSetRowsArray_DeltaSetRecord(rowsArray, rowIndex, recordSize);
        SFUInt16 valueIndex;

        for (valueIndex = 0; valueIndex < regionCount; valueIndex++) {
            SFUInt16 regionIndex = ItemVarData_RegionIndexItem(varDataTable, valueIndex);
            SFInt16 delta;

            if (valueIndex < shortDeltaCount) {
                delta = DeltaSetRecord_I16Delta(deltaSet, valueIndex);
            } else {
                delta = DeltaSetRecord_I8Delta(deltaSet, shortDeltaCount, valueIndex - shortDeltaCount);
            }

            adjustment += CalculateScalarForRegion(regionListTable, regionIndex, coordArray, coordCount) * delta;
        }
    }

    return adjustment;
}

static double GetDeltaFromVariationStore(Data varStoreTable,
    SFUInt16 dataIndex, SFUInt16 rowIndex, const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 format = ItemVarStore_Format(varStoreTable);

    switch (format) {
        case 1: {
            Data regionList = ItemVarStore_VarRegionListTable(varStoreTable);
            SFUInt16 dataCount = ItemVarStore_ItemVarDataCount(varStoreTable);

            if (dataIndex < dataCount) {
                Data varDataTable = ItemVarStore_ItemVarDataTable(varStoreTable, dataIndex);
                return CalculateVariationAdjustment(varDataTable, regionList, rowIndex, coordArray, coordCount);
            }
            break;
        }
    }

    return 0;
}

SF_INTERNAL SFInt32 GetVariationPixels(Data varIndexTable, Data varStoreTable,
    const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 outerIndex = VarIndex_DeltaSetOuterIndex(varIndexTable);
    SFUInt16 innerIndex = VarIndex_DeltaSetInnerIndex(varIndexTable);
    SFUInt16 deltaFormat = VarIndex_DeltaFormat(varIndexTable);

    if (deltaFormat == 0x8000) {
        double delta = GetDeltaFromVariationStore(varStoreTable, outerIndex, innerIndex, coordArray, coordCount);
        return (delta >= 0.0 ? (SFInt32)(delta + 0.5) : (SFInt32)(delta - 0.5));
    }

    return 0;
}

static SFBoolean MatchCondition(Data condTable, const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 format = Condition_Format(condTable);

    switch (format) {
        case 1: {
            SFUInt16 axisIndex = Condition_AxisIndex(condTable);
            SFInt16 minValue = Condition_FilterRangeMinValue(condTable);
            SFInt16 maxValue = Condition_FilterRangeMaxValue(condTable);
            SFInt16 coordValue = (axisIndex < coordCount ? coordArray[axisIndex] : 0);

            if (coordValue >= minValue && coordValue <= maxValue) {
                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean MatchConditionSet(Data condSetTable, const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt16 condCount = ConditionSet_ConditionCount(condSetTable);
    SFUInt16 condIndex;

    for (condIndex = 0; condIndex < condCount; condIndex++) {
        Data condTable = ConditionSet_ConditionTable(condSetTable, condIndex);

        if (!MatchCondition(condTable, coordArray, coordCount)) {
            return SFFalse;
        }
    }

    return SFTrue;
}

SF_INTERNAL Data SearchFeatureSubstitutionTable(Data featureVarsTable,
    const SFInt16 *coordArray, SFUInteger coordCount)
{
    SFUInt32 recordCount = FeatureVars_FeatureVarCount(featureVarsTable);
    SFUInt32 recordIndex;

    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        Data varRecord = FeatureVars_FeatureVarRecord(featureVarsTable, recordIndex);
        SFUInt32 condSetOffset = FeatureVarRecord_ConditionSetOffset(varRecord);
        Data condSetTable = Data_Subdata(featureVarsTable, condSetOffset);

        if (MatchConditionSet(condSetTable, coordArray, coordCount)) {
            SFUInt32 featureSubstOffset = FeatureVarRecord_FeatureSubstOffset(varRecord);
            return Data_Subdata(featureVarsTable, featureSubstOffset);
        }
    }

    return NULL;
}

SF_INTERNAL Data SearchAlternateFeatureTable(Data featureSubstTable, SFUInt16 featureIndex)
{
    SFUInt16 substCount = FeatureSubst_SubstCount(featureSubstTable);
    SFUInt16 substIndex;

    for (substIndex = 0; substIndex < substCount; substIndex++) {
        Data substRec = FeatureSubst_FeatureSubstRecord(featureSubstTable, substIndex);
        SFUInt16 recFeatureIndex = FeatureSubstRecord_FeatureIndex(substRec);

        if (recFeatureIndex == featureIndex) {
            SFUInt32 altFeatureOffset = FeatureSubstRecord_AltFeatureOffset(substRec);
            return Data_Subdata(featureSubstTable, altFeatureOffset);
        }

        /* Stop searching if a higher feature index value is encountered. */
        if (recFeatureIndex > featureIndex) {
            break;
        }
    }

    return NULL;
}
