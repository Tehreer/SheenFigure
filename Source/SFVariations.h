/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_VARIATIONS_H
#define _SF_INTERNAL_VARIATIONS_H

#include "SFData.h"

/**************************************VARIATION REGION LIST***************************************/

#define SFVarRegionList_AxisCount(data)                 Data_UInt16(data, 0)
#define SFVarRegionList_RegionCount(data)               Data_UInt16(data, 2)
#define SFVarRegionList_VarRegionRecord(data, index, axisCount) \
                                                        Data_Subdata(data, 4 + ((index) * ((axisCount) * 6)))

#define SFVarRegionRecord_RegionAxisCoords(data, index) Data_Subdata(data, (index) * 6)

#define SFRegionAxisCoords_StartCoord(data)             Data_Int16(data, 0)
#define SFRegionAxisCoords_PeakCoord(data)              Data_Int16(data, 2)
#define SFRegionAxisCoords_EndCoord(data)               Data_Int16(data, 4)

/**************************************************************************************************/

/************************************ITEM VARIATION STORE TABLE************************************/

#define SFItemVarStore_Format(data)                     Data_UInt16(data, 0)
#define SFItemVarStore_VarRegionListOffset(data)        Data_UInt32(data, 2)
#define SFItemVarStore_ItemVarDataCount(data)           Data_UInt16(data, 6)
#define SFItemVarStore_ItemVarDataOffset(data, index)   Data_UInt32(data, 8 + ((index) * 4))
#define SFItemVarStore_VarRegionListTable(data) \
    Data_Subdata(data, SFItemVarStore_VarRegionListOffset(data))
#define SFItemVarStore_ItemVarDataTable(data, index) \
    Data_Subdata(data, SFItemVarStore_ItemVarDataOffset(data, index))

/**************************************************************************************************/

/***********************************ITEM VARIATION DATA SUBTABLE***********************************/

#define SFItemVarData_ItemCount(data)                   Data_UInt16(data, 0)
#define SFItemVarData_ShortDeltaCount(data)             Data_UInt16(data, 2)
#define SFItemVarData_RegionIndexCount(data)            Data_UInt16(data, 4)
#define SFItemVarData_RegionIndexItem(data, index)      Data_UInt16(data, 6 + ((index) * 2))
#define SFItemVarData_DeltaSetRowsArray(data, riCount)  Data_Subdata(data, 6 + ((riCount) * 2))

#define SFDeltaSetRowsArray_DeltaSetRecord(data, index, dsrSize) \
                                                        Data_Subdata(data, (index) * (dsrSize))

#define SFDeltaSetRecord_Size(sdCount, riCount)         ((sdCount) + (riCount))
#define SFDeltaSetRecord_I16Delta(data, index)          Data_Int16(data, (index) * 2)
#define SFDeltaSetRecord_I8Delta(data, sdCount, index)  Data_Int8(data, ((sdCount) * 2) + (index))

/**************************************************************************************************/

#endif
