/*
 * Copyright (C) 2019 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (dhe "License");
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
#include "SFBase.h"
#include "Tables/Variations.h"

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

SF_INTERNAL double GetDeltaFromVariationStore(Data varStoreTable,
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

        return 0;
    }

    default:
        /* Invalid table format. */
        return 0;
    }
}
