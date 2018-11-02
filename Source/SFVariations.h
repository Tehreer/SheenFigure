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

/************************************ITEM VARIATION STORE TABLE************************************/

#define SFItemVarStore_Format(data)                     SFData_UInt16(data, 0)
#define SFItemVarStore_VarRegionListOffset(data)        SFData_UInt32(data, 2)
#define SFItemVarStore_ItemVarDataCount(data)           SFData_UInt16(data, 6)
#define SFItemVarStore_ItemVarDataOffset(data, index)   SFData_UInt32(data, 8 + ((index) * 4))
#define SFItemVarStore_VarRegionListTable(data) \
    SFData_Subdata(data, SFItemVarStore_VarRegionListOffset(data))
#define SFItemVarStore_ItemVarDataTable(data, index) \
    SFData_Subdata(data, SFItemVarStore_ItemVarDataOffset(data, index))

/**************************************************************************************************/

/***********************************ITEM VARIATION DATA SUBTABLE***********************************/

#define SFItemVarData_ItemCount(data)                   SFData_UInt16(data, 0)
#define SFItemVarData_ShortDeltaCount(data)             SFData_UInt16(data, 2)
#define SFItemVarData_RegionIndexCount(data)            SFData_UInt16(data, 4)
#define SFItemVarData_RegionIndexItem(data, index)      SFData_UInt16(data, 6 + ((index) * 2))
#define SFItemVarData_DeltaSetRowsArray(data, riCount)  SFData_Subdata(data, 6 + ((riCount) * 2))

#define SFDeltaSetRowsArray_DeltaSetRecord(data, index, dsrSize) \
                                                        SFData_Subdata(data, (index) * (dsrSize))

#define SFDeltaSetRecord_Size(sdCount, riCount)         ((sdCount) + (riCount))
#define SFDeltaSetRecord_I16Delta(data, index)          SFData_Int16(data, (index) * 2)
#define SFDeltaSetRecord_I8Delta(data, sdCount, index)  SFData_Int8(data, ((sdCount) * 2) + (index))

/**************************************************************************************************/

#endif
