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

#ifndef _SF_INTERNAL_DATA_H
#define _SF_INTERNAL_DATA_H

#include "SFBase.h"

typedef SFUInt16        SFOffset;
typedef const SFUInt8 * SFData;

#define SFData_UInt8(data, offset)          (data)[offset]

#define SFData_UInt16(data, offset)         \
(SFUInt16)                                  \
(                                           \
   ((SFUInt16)(data)[(offset) + 0] << 8)    \
 | ((SFUInt16)(data)[(offset) + 1] << 0)    \
)

#define SFData_UInt32(data, offset)         \
(SFUInt32)                                  \
(                                           \
   ((SFUInt32)(data)[(offset) + 0] << 24)   \
 | ((SFUInt32)(data)[(offset) + 1] << 16)   \
 | ((SFUInt32)(data)[(offset) + 2] <<  8)   \
 | ((SFUInt32)(data)[(offset) + 3] <<  0)   \
)

#define SFData_Int8(data, offset)           (SFInt8)SFData_UInt8(data, offset)
#define SFData_Int16(data, offset)          (SFInt16)SFData_UInt16(data, offset)
#define SFData_Int32(data, offset)          (SFInt32)SFData_UInt32(data, offset)

#define SFData_Subdata(data, offset)        (&(data)[offset])

#endif
