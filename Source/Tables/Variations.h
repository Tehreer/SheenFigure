/*
 * Copyright (C) 2018-2019 Muhammad Tayyab Akram
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

#ifndef _SF_TABLES_VARIATIONS_H
#define _SF_TABLES_VARIATIONS_H

#include <SFConfig.h>

#include "Data.h"
#include "SFBase.h"

/**************************************VARIATION REGION LIST***************************************/

#define VarRegionList_AxisCount(data)                   Data_UInt16(data, 0)
#define VarRegionList_RegionCount(data)                 Data_UInt16(data, 2)
#define VarRegionList_VarRegionRecord(data, index, axisCount) \
                                                        Data_Subdata(data, 4 + ((index) * ((axisCount) * 6)))

#define VarRegionRecord_RegionAxisCoords(data, index)   Data_Subdata(data, (index) * 6)

#define RegionAxisCoords_StartCoord(data)               Data_Int16(data, 0)
#define RegionAxisCoords_PeakCoord(data)                Data_Int16(data, 2)
#define RegionAxisCoords_EndCoord(data)                 Data_Int16(data, 4)

/**************************************************************************************************/

/************************************ITEM VARIATION STORE TABLE************************************/

#define ItemVarStore_Format(data)                       Data_UInt16(data, 0)
#define ItemVarStore_VarRegionListOffset(data)          Data_UInt32(data, 2)
#define ItemVarStore_ItemVarDataCount(data)             Data_UInt16(data, 6)
#define ItemVarStore_ItemVarDataOffset(data, index)     Data_UInt32(data, 8 + ((index) * 4))
#define ItemVarStore_VarRegionListTable(data) \
    Data_Subdata(data, ItemVarStore_VarRegionListOffset(data))
#define ItemVarStore_ItemVarDataTable(data, index) \
    Data_Subdata(data, ItemVarStore_ItemVarDataOffset(data, index))

/**************************************************************************************************/

/***********************************ITEM VARIATION DATA SUBTABLE***********************************/

#define ItemVarData_ItemCount(data)                     Data_UInt16(data, 0)
#define ItemVarData_ShortDeltaCount(data)               Data_UInt16(data, 2)
#define ItemVarData_RegionIndexCount(data)              Data_UInt16(data, 4)
#define ItemVarData_RegionIndexItem(data, index)        Data_UInt16(data, 6 + ((index) * 2))
#define ItemVarData_DeltaSetRowsArray(data, riCount)    Data_Subdata(data, 6 + ((riCount) * 2))

#define DeltaSetRowsArray_DeltaSetRecord(data, index, dsrSize) \
                                                        Data_Subdata(data, (index) * (dsrSize))

#define DeltaSetRecord_Size(sdCount, riCount)           ((sdCount) + (riCount))
#define DeltaSetRecord_I16Delta(data, index)            Data_Int16(data, (index) * 2)
#define DeltaSetRecord_I8Delta(data, sdCount, index)    Data_Int8(data, ((sdCount) * 2) + (index))

/**************************************************************************************************/

SF_INTERNAL double CalculateScalarForRegion(Data regionListTable, SFUInt16 regionIndex,
    const SFInt16 *coordArray, SFUInteger coordCount);

SF_INTERNAL double GetDeltaFromVariationStore(Data varStoreTable,
    SFUInt16 dataIndex, SFUInt16 rowIndex, const SFInt16 *coordArray, SFUInteger coordCount);

#endif
