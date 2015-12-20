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

#ifndef SF_TYPES_H
#define SF_TYPES_H

#include <stdint.h>

/**
 * A type to represent a boolean value.
 */
typedef enum {
    SFFalse = 0, /**< A value representing the false state. */
    SFTrue  = 1  /**< A value representing the true state. */
} SFBoolean;

/**
 * A type to represent an 8-bit signed integer.
 */
typedef int8_t          SFInt8;

/**
 * A type to represent a 16-bit signed integer.
 */
typedef int16_t         SFInt16;

/**
 * A type to represent a 32-bit signed integer.
 */
typedef int32_t         SFInt32;

/**
 * A type to represent an 8-bit unsigned integer.
 */
typedef uint8_t         SFUInt8;

/**
 * A value that indicates maximum limit of SFUInt8
 */
#define SFUInt8Max      UINT8_MAX

/**
 * A type to represent a 16-bit unsigned integer.
 */
typedef uint16_t        SFUInt16;

/**
 * A value that indicates maximum limit of SFUInt16
 */
#define SFUInt16Max     UINT16_MAX

/**
 * A type to represent a 32-bit unsigned integer.
 */
typedef uint32_t        SFUInt32;

/**
 * A value that indicates maximum limit of SFUInt32
 */
#define SFUInt32Max     UINT32_MAX

/**
 * A signed integer type whose width is equal to the width of the machine word.
 */
typedef intptr_t        SFInteger;

/**
 * An unsigned integer type whose width is equal to the width of the machine word.
 */
typedef uintptr_t       SFUInteger;

typedef size_t          SFIndex;

/**
 * A value that indicates an invalid unsigned index.
 */
#define SFInvalidIndex  (SFUInteger)(-1)

/**
 * A type to represent a code point of unicode code space.
 */
typedef SFUInt32        SFCodePoint;

/**
 * A type to represent a glyph identifier.
 */
typedef SFUInt16        SFGlyphID;

/**
 * A type to represent a tag of 4 characters.
 */
#define SF_TAG          SFUInt32

#define SFTagMake(a, b, c, d)   \
(SF_TAG)                        \
(                               \
   ((SFUInt8)a << 24)           \
 | ((SFUInt8)b << 16)           \
 | ((SFUInt8)c <<  8)           \
 | ((SFUInt8)d <<  0)           \
)

#define SF_TAG__MAKE(a, b, c, d) \
(SF_TAG)                         \
(                                \
 ((((SFUInt8)a << 24) | (SFUInt8)b << 16) | (SFUInt8)c << 8) | (SFUInt8)d \
)

/**
 * A type to represent range.
 */
typedef struct SFRange {
    SFUInteger start;  /**< The starting location of the range. */
    SFUInteger length; /**< The length of the range. */
} SFRange;
typedef SFRange *SFRangeRef;

/**
 * A type to represent an ordered pair of x- and y-coordinates that defines a
 * point.
 */
typedef struct SFPoint {
    SFInteger x; /**< The x-coordinate of the point. */
    SFInteger y; /**< The y-coordinate of the point. */
} SFPoint;

/**
 * A type to represent an ordered pair of width and height that defines a size.
 */
typedef struct SFSize {
    SFUInteger width;  /**< The width of the size. */
    SFUInteger height; /**< The height of the size. */
} SFSize;

/**
 * A type to represent a frame.
 */
typedef struct SFFrame {
    SFPoint position; /**< The position of the frame. */
    SFSize size;      /**< The size of the frame. */
} SFFrame;

#endif
