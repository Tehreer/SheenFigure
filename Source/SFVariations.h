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

#endif
