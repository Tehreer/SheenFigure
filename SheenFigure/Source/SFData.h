/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#ifndef SF_DATA_INTERNAL_H
#define SF_DATA_INTERNAL_H

#include <SFTypes.h>

typedef SFUInt32        SFTag;
typedef SFUInt32        SFFixed;

typedef SFUInt16        SFOffset;

#define SFTagNone       0
#define SFTagInvalid    SFUInt32Max

typedef const SFUInt8 *SFData;

#define _SF_UINT16__SWAP(i)                 \
(SFUInt16)                                  \
(                                           \
   (((SFUInt16)(i) & 0xFF00) >> 8)          \
 | (((SFUInt16)(i) & 0x00FF) << 8)          \
)

#define _SF_UINT32__SWAP(i)                 \
(SFUInt32)                                  \
(                                           \
   (((SFUInt32)(i) & 0xFF000000) >> 24)     \
 | (((SFUInt32)(i) & 0x00FF0000) >>  8)     \
 | (((SFUInt32)(i) & 0x0000FF00) <<  8)     \
 | (((SFUInt32)(i) & 0x000000FF) << 24)     \
)

#define SF_DATA__READ_UINT8(d, o)           d[o]

#define SF_DATA__READ_UINT16(d, o)          \
(SFUInt16)                                  \
(                                           \
   ((SFUInt16)d[(o) + 0] << 8)              \
 | ((SFUInt16)d[(o) + 1] << 0)              \
)

#define SF_DATA__READ_UINT32(d, o)          \
(SFUInt32)                                  \
(                                           \
   ((SFUInt32)d[(o) + 0] << 24)             \
 | ((SFUInt32)d[(o) + 1] << 16)             \
 | ((SFUInt32)d[(o) + 2] <<  8)             \
 | ((SFUInt32)d[(o) + 3] <<  0)             \
)

#define SF_DATA__READ_INT8(d, o)			(SFInt8)SF_DATA__READ_UINT8(d, o)
#define SF_DATA__READ_INT16(d, o)           (SFInt16)SF_DATA__READ_UINT16(d, o)
#define SF_DATA__READ_INT32(d, o)           (SFInt32)SF_DATA__READ_UINT32(d, o)

#define SF_DATA__SUBDATA(d, o)				(&d[(o)])

#endif
