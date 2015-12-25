/*
 * Copyright (C) 2014 Muhammad Tayyab Akram
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

#ifndef SF_GDEF_INTERNAL_H
#define SF_GDEF_INTERNAL_H

#include <SFTypes.h>
#include "SFData.h"

enum {
    SFGlyphClassValueNone = 0,
    SFGlyphClassValueBase = 1,     /**< Single character, spacing glyph */
    SFGlyphClassValueLigature = 2, /**< Multiple character, spacing glyph */
    SFGlyphClassValueMark = 3,     /**< Non-spacing combining glyph */
    SFGlyphClassValueComponent = 4 /**< Part of single character, spacing glyph */
};
typedef SFUInt16 SFGlyphClassValue;

#define SF_GDEF__VERSION(t)                             SF_DATA__READ_UINT32(t, 0)
#define SF_GDEF__GLYPH_CLASS_DEF_OFFSET(t)              SF_DATA__READ_UINT16(t, 4)
#define SF_GDEF__ATTACH_LIST_OFFSET(t)                  SF_DATA__READ_UINT16(t, 6)
#define SF_GDEF__LIG_CARET_LIST_OFFSET(t)               SF_DATA__READ_UINT16(t, 8)
#define SF_GDEF__MARK_ATTACH_CLASS_DEF_OFFSET(t)        SF_DATA__READ_UINT16(t, 10)
#define SF_GDEF__MARK_GLYPH_SETS_DEF_OFFSET(t)          SF_DATA__READ_UINT16(t, 12)

#define SF_ATTACH_LIST__COVERAGE(t)                     SF_DATA__READ_UINT16(t, 0)
#define SF_ATTACH_LIST__GLYPH_COUNT(t)                  SF_DATA__READ_UINT16(t, 2)
#define SF_ATTACH_LIST__ATTACH_POINT(t)                 SF_DATA__READ_UINT16(t, 4)

#define SF_ATTACH_POINT__POINT_COUNT(t)                 SF_DATA__READ_UINT16(t, 0)
#define SF_ATTACH_POINT__POINT_INDEX(t, i)              SF_DATA__READ_UINT16(t, 2 + ((i) * 2))

#define SF_LIG_CARET_LIST__COVERAGE(t)                  SF_DATA__READ_UINT16(t, 0)
#define SF_LIG_CARET_LIST__LIG_GLYPH_COUNT(t)           SF_DATA__READ_UINT16(t, 2)
#define SF_LIG_CARET_LIST__LIG_GLYPH(t, i)              SF_DATA__READ_UINT16(t, 4 + ((i) * 2))

#define SF_LIG_GLYPH__CARET_COUNT(t)                    SF_DATA__READ_UINT16(t, 0)
#define SF_LIG_GLYPH__CARET_VALUE(t, i)                 SF_DATA__READ_UINT16(t, 2 + ((i) * 2))

#define SF_CARET_VALUE_FORMAT(t)                        SF_DATA__READ_UINT16(t, 0)

#define SF_CARET_VALUE_F1__COORDINATE(t) ((SFInt16)SF_DATA__READ_UINT16(t, 2))

#define SF_CARET_VALUE_F2__CARET_VALUE_POINT(t)    SF_DATA__READ_UINT16(t, 2)

#define SF_CARET_VALUE_F3__COORDINATE(t) ((SFInt16)SF_DATA__READ_UINT16(t, 2))
#define SF_CARET_VALUE_F3__DEVICE_TABLE(t)         SF_DATA__READ_UINT16(t, 4)

#define SF_MARK_GLYPH_SETS__FORMAT(t)                   SF_DATA__READ_UINT16(t, 0)
#define SF_MARK_GLYPH_SETS__MARK_SET_COUNT(t)           SF_DATA__READ_UINT16(t, 2)
#define SF_MARK_GLYPH_SETS__COVERAGE(t, i)              SF_DATA__READ_UINT32(t, 4 + ((i) * 4))

#endif
