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

#ifndef _SF_INTERNAL_BASE_H
#define _SF_INTERNAL_BASE_H

#include <SFBase.h>
#include <SFConfig.h>

/**
 * A value that indicates maximum limit of SFUInt8
 */
#define SFUInt8Max          UINT8_MAX

/**
 * A value that indicates maximum limit of SFUInt16
 */
#define SFUInt16Max         UINT16_MAX

/**
 * A value that indicates maximum limit of SFUInt32
 */
#define SFUInt32Max         UINT32_MAX

/**
 * A value representing an invalid code point.
 */
#define SFCodepointInvalid  UINT32_MAX

/**
 * A type to represent a range of sequential items.
 */
typedef struct _SFRange {
    SFUInteger start; /** An value representing the starting location of the range. */
    SFUInteger count; /** A value representing the number of items in the range. */
} SFRange;

extern const SFRange SFRangeEmpty;

#endif
