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

#ifndef _SF_TABLES_MORX_H
#define _SF_TABLES_MORX_H

#include "Data.h"

/***************************************METAMORPHOSIS TABLE****************************************/

#define Morx_Version(data)                          Data_UInt16(data, 0)
#define Morx_Unused(data)                           Data_UInt16(data, 2)
#define Morx_NChains(data)                          Data_UInt32(data, 4)
#define Morx_FirstChain(data)                       Data_Subdata(data, 8)

/**************************************************************************************************/

/***************************************METAMORPHOSIS CHAIN****************************************/

#define MorxChain_DefaultFlags(data)                Data_UInt32(data, 0)
#define MorxChain_ChainLength(data)                 Data_UInt32(data, 4)
#define MorxChain_NFeatureEntries(data)             Data_UInt32(data, 8)
#define MorxChain_NSubtables(data)                  Data_UInt32(data, 12)
#define MorxChain_Feature(data, index)              Data_Subdata(data, 16 + ((index) * 12))
#define MorxChain_FirstSubtable(data, featureCount) Data_Subdata(data, 16 + ((featureCount) * 12))
#define MorxChain_NextChain(data) \
    Data_Subdata(data, MorxChain_ChainLength(data))

/**************************************************************************************************/

/******************************************FEATURE TABLE*******************************************/

#define Feature_FeatureType(data)                   Data_UInt16(data, 0)
#define Feature_FeatureSetting(data)                Data_UInt16(data, 2)
#define Feature_EnableFlags(data)                   Data_UInt32(data, 4)
#define Feature_DisableFlags(data)                  Data_UInt32(data, 8)

/**************************************************************************************************/

/**************************************METAMORPHOSIS SUBTABLE**************************************/

#define MorxSubtable_Length(data)                   Data_UInt32(data, 0)
#define MorxSubtable_Coverage(data)                 Data_UInt32(data, 4)
#define MorxSubtable_SubFeatureFlags(data)          Data_UInt32(data, 8)
#define MorxSubtable_TypedSubtable(data)            Data_Subdata(data, 12)
#define MorxSubtable_NextSubtable(data) \
    Data_Subdata(data, MorxSubtable_Length(data))

/**************************************************************************************************/

#endif
