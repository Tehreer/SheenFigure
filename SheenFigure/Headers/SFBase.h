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

#ifndef _SF_BASE_H
#define _SF_BASE_H

#include <stdint.h>

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
 * A type to represent a 16-bit unsigned integer.
 */
typedef uint16_t        SFUInt16;

/**
 * A type to represent a 32-bit unsigned integer.
 */
typedef uint32_t        SFUInt32;

/**
 * A signed integer type whose width is equal to the width of the machine word.
 */
typedef intptr_t        SFInteger;

/**
 * An unsigned integer type whose width is equal to the width of the machine word.
 */
typedef uintptr_t       SFUInteger;

/**
 * A value that indicates an invalid unsigned index.
 */
#define SFInvalidIndex  (SFUInteger)(-1)

/**
 * A type to represent a boolean value.
 */
enum {
    SFFalse = 0, /**< A value representing the false state. */
    SFTrue  = 1  /**< A value representing the true state. */
};
typedef SFUInt8         SFBoolean;

/**
 * A type to represent a code point of unicode code space.
 */
typedef SFUInt32        SFCodepoint;

/**
 * A type to represent a glyph's identifier.
 */
typedef SFUInt16        SFGlyphID;

/**
 * A type to represent a glyph's advance.
 */
typedef SFInt32         SFAdvance;

/**
 * A type to represent a tag of 4 characters.
 */
typedef SFUInt32        SFTag;

#define SFTagMake(a, b, c, d)   \
(SFTag)                         \
(                               \
   ((SFUInt8)a << 24)           \
 | ((SFUInt8)b << 16)           \
 | ((SFUInt8)c <<  8)           \
 | ((SFUInt8)d <<  0)           \
)

/**
 * A type to represent an ordered pair of integer x- and y-coordinates that defines a point in a
 * two-dimensional plane.
 */
typedef struct SFPoint {
    SFInt32 x; /**< The x-coordinate of the point. */
    SFInt32 y; /**< The y-coordinate of the point. */
} SFPoint;

enum {
    SFStringEncodingUTF8 = 0,  /**< An 8-bit representation of Unicode code points. */
    SFStringEncodingUTF16 = 1, /**< 16-bit UTF encoding in native endianness. */
    SFStringEncodingUTF32 = 2  /**< 32-bit UTF encoding in native endianness. */
};
typedef SFUInt32 SFStringEncoding;

enum {
    SFTextDirectionLeftToRight = 0,
    SFTextDirectionRightToLeft = 1
};
typedef SFUInt32 SFTextDirection;

enum {
    SFTextModeForward = 0,
    SFTextModeBackward = 1
};
typedef SFUInt32 SFTextMode;

/**
 * Returns the default direction of a script.
 *
 * @param scriptTag
 *      The tag of the script whose default direction is returned.
 */
SFTextDirection SFScriptGetDefaultDirection(SFTag scriptTag);

#endif
