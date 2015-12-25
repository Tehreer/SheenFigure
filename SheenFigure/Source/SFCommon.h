/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#ifndef SF_COMMON_INTERNAL_H
#define SF_COMMON_INTERNAL_H

#include <SFTypes.h>
#include "SFData.h"

typedef SFUInt16 SFLookupType;

enum {
    SFLookupFlagRightToLeft = 0x0001,
    SFLookupFlagIgnoreBaseGlyphs = 0x0002,
    SFLookupFlagIgnoreLigatures = 0x0004,
    SFLookupFlagIgnoreMarks = 0x0008,
    SFLookupFlagUseMarkFilteringSet = 0x0010,
    SFLookupFlagReserved = 0x00E0,
    SFLookupFlagMarkAttachmentType = 0xFF00
};
typedef SFUInt16 SFLookupFlag;

#define SF_TAG_RECORD__SIZE()                       (6)
#define SF_TAG_RECORD__TAG(d)                       SF_DATA__READ_UINT32(d, 0)
#define SF_TAG_RECORD__OFFSET(d)                    SF_DATA__READ_UINT16(d, 4)

#define _SF_GLYPH_RANGE__SIZE()                     (6)
#define _SF_GLYPH_RANGE__START(d)                   SF_DATA__READ_UINT16(d, 0)
#define _SF_GLYPH_RANGE__END(d)                     SF_DATA__READ_UINT16(d, 2)
#define _SF_GLYPH_RANGE__VALUE(d)                   SF_DATA__READ_UINT16(d, 4)

/************************************ARRAYS************************************/

#define SF_UINT16_ARRAY__VALUE(d, i)                SF_DATA__READ_UINT16(d, (i) * 2)

#define SF_GLYPH_ARRAY__VALUE(d, i)                 SF_UINT16_ARRAY__VALUE(d, i)

#define SF_TAG_RECORD_ARRAY__VALUE(d, i)            SF_DATA__SUBDATA(d, (i) * SF_TAG_RECORD__SIZE())

/******************************************************************************/

#define SF_HEADER__VERSION(t)                       SF_DATA__READ_UINT32(t, 0)
#define SF_HEADER__SCRIPT_LIST(t)                   SF_DATA__READ_UINT16(t, 4)
#define SF_HEADER__FEATURE_LIST(t)                  SF_DATA__READ_UINT16(t, 6)
#define SF_HEADER__LOOKUP_LIST(t)                   SF_DATA__READ_UINT16(t, 8)

/******************************SCRIPT LIST TABLE*******************************/

#define SF_SCRIPT_LIST__SCRIPT_COUNT(d)             SF_DATA__READ_UINT16(d, 0)
#define SF_SCRIPT_LIST__SCRIPT_RECORD(d, i)         SF_DATA__SUBDATA(d, 2 + ((i) * SF_TAG_RECORD__SIZE()))

#define SF_SCRIPT_RECORD__SCRIPT_TAG(d)             SF_TAG_RECORD__TAG(d)
#define SF_SCRIPT_RECORD__SCRIPT(d)                 SF_TAG_RECORD__OFFSET(d)

/******************************************************************************/

/*********************************SCRIPT TABLE*********************************/

#define SF_SCRIPT__DEFAULT_LANG_SYS(d)              SF_DATA__READ_UINT16(d, 0)
#define SF_SCRIPT__LANG_SYS_COUNT(d)                SF_DATA__READ_UINT16(d, 2)
#define SF_SCRIPT__LANG_SYS_RECORD(d, i)            SF_DATA__SUBDATA(d, 4 + ((i) * SF_TAG_RECORD__SIZE()))

#define SF_LANG_SYS_RECORD__LANG_SYS_TAG(d)         SF_TAG_RECORD__TAG(d)
#define SF_LANG_SYS_RECORD__LANG_SYS(d)             SF_TAG_RECORD__OFFSET(d)

/******************************************************************************/

/****************************LANGUAGE SYSTEM TABLE*****************************/

#define SF_LANG_SYS__LOOKUP_ORDER(d)                SF_DATA__READ_UINT16(d, 0)
#define SF_LANG_SYS__REQ_FEATURE_INDEX(d)           SF_DATA__READ_UINT16(d, 2)
#define SF_LANG_SYS__FEATURE_COUNT(d)               SF_DATA__READ_UINT16(d, 4)
#define SF_LANG_SYS__FEATURE_INDEX(d, i)            SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

/******************************************************************************/

/******************************FEATURE LIST TABLE******************************/

#define SF_FEATURE_LIST__FEATURE_COUNT(d)           SF_DATA__READ_UINT16(d, 0)
#define SF_FEATURE_LIST__FEATURE_RECORD(d, i)       SF_DATA__SUBDATA(d, 2 + ((i) * SF_TAG_RECORD__SIZE()))

#define SF_FEATURE_RECORD__FEATURE_TAG(d)           SF_TAG_RECORD__TAG(d)
#define SF_FEATURE_RECORD__FEATURE(d)               SF_TAG_RECORD__OFFSET(d)

/******************************************************************************/

/********************************FEATURE TABLE*********************************/

#define SF_FEATURE__FEATURE_PARAMS(d)               SF_DATA__READ_UINT16(d, 0)
#define SF_FEATURE__LOOKUP_COUNT(d)                 SF_DATA__READ_UINT16(d, 2)
#define SF_FEATURE__LOOKUP_LIST_INDEX(d, i)         SF_DATA__READ_UINT16(d, 4 + ((i) * 2))

/******************************************************************************/

/******************************LOOKUP LIST TABLE*******************************/

#define SF_LOOKUP_LIST__LOOKUP_COUNT(d)             SF_DATA__READ_UINT16(d, 0)
#define SF_LOOKUP_LIST__LOOKUP(d, i)                SF_DATA__READ_UINT16(d, 2 + ((i) * 2))

/******************************************************************************/

/*********************************LOOKUP TABLE*********************************/

#define SF_LOOKUP__LOOKUP_TYPE(d)                   SF_DATA__READ_UINT16(d, 0)
#define SF_LOOKUP__LOOKUP_FLAG(d)                   SF_DATA__READ_UINT16(d, 2)
#define SF_LOOKUP__SUB_TABLE_COUNT(d)               SF_DATA__READ_UINT16(d, 4)
#define SF_LOOKUP__SUB_TABLE(d, i)                  SF_DATA__READ_UINT16(d, 6 + ((i) * 2))
#define SF_LOOKUP__MARK_FILTERING_SET(d, sc)        SF_DATA__READ_UINT16(d, 8 + ((sc) * 2))

/******************************************************************************/

/********************************COVERAGE TABLE********************************/

#define SF_COVERAGE_FORMAT(d)                       SF_DATA__READ_UINT16(d, 0)

#define SF_COVERAGE_F1__GLYPH_COUNT(d)              SF_DATA__READ_UINT16(d, 2)
#define SF_COVERAGE_F1__GLYPH_ARRAY(d)              SF_DATA__SUBDATA(d, 4)

#define SF_COVERAGE_F2__RANGE_COUNT(d)              SF_DATA__READ_UINT16(d, 2)
#define SF_COVERAGE_F2__RANGE_RECORD(d, i)          SF_DATA__SUBDATA(d, 4 + ((i) * _SF_GLYPH_RANGE__SIZE()))

#define SF_RANGE_RECORD__START(d)                   _SF_GLYPH_RANGE__START(d)
#define SF_RANGE_RECORD__END(d)                     _SF_GLYPH_RANGE__END(d)
#define SF_RANGE_RECORD__START_COVERAGE_INDEX(d)    _SF_GLYPH_RANGE__VALUE(d)

/******************************************************************************/

/****************************CLASS DEFINITION TABLE****************************/

#define SF_CLASS_DEF_FORMAT(d)                      SF_DATA__READ_UINT16(d, 0)

#define SF_CLASS_DEF_F1__START_GLYPH(d)             SF_DATA__READ_UINT16(d, 2)
#define SF_CLASS_DEF_F1__GLYPH_COUNT(d)             SF_DATA__READ_UINT16(d, 4)
#define SF_CLASS_DEF_F1__CLASS_VALUE_ARRAY(d)       SF_DATA__SUBDATA(d, 6)

#define SF_CLASS_DEF_F2__CLASS_RANGE_COUNT(d)       SF_DATA__READ_UINT16(d, 2)
#define SF_CLASS_DEF_F2__CLASS_RANGE_RECORD(d, i)   SF_DATA__SUBDATA(d, 4 + ((i) * _SF_GLYPH_RANGE__SIZE()))
#define SF_CLASS_DEF_F2__RANGE_RECORD_ARRAY(d)      SF_CLASS_DEF_F2__CLASS_RANGE_RECORD(d, 0)

#define SF_CLASS_RANGE_RECORD__START(d)             _SF_GLYPH_RANGE__START(d)
#define SF_CLASS_RANGE_RECORD__END(d)               _SF_GLYPH_RANGE__END(d)
#define SF_CLASS_RANGE_RECORD__CLASS(d)             _SF_GLYPH_RANGE__VALUE(d)

/******************************************************************************/

/*********************************DEVICE TABLE*********************************/

#define SF_DEVICE__START_SIZE(d)                    SF_DATA__READ_UINT16(d, 0)
#define SF_DEVICE__END_SIZE(d)                      SF_DATA__READ_UINT16(d, 2)
#define SF_DEVICE__DELTA_FORMAT(d)                  SF_DATA__READ_UINT16(d, 4)
#define SF_DEVICE__DELTA_VALUE(d, i)                SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

/******************************************************************************/

/********************************LOOKUP RECORD*********************************/

#define SF_LOOKUP_RECORD__SEQUENCE_INDEX(d)         SF_DATA__READ_UINT16(d, 0)
#define SF_LOOKUP_RECORD__LOOKUP_LIST_INDEX(d)      SF_DATA__READ_UINT16(d, 2)

/******************************************************************************/

/*************************CHAINING CONTEXTUAL SUBTABLE*************************/

#define SF_CHAIN_CONTEXT_FORMAT(d)                  SF_DATA__READ_UINT16(d, 0)
#define SF_CHAIN_CONTEXT_F3__BACKTRACK_RECORD(d)    SF_DATA__SUBDATA(d, 2)

#define SF_BACKTRACK_RECORD__GLYPH_COUNT(d)         SF_DATA__READ_UINT16(d, 0)
#define SF_BACKTRACK_RECORD__COVERAGE(d, i)         SF_DATA__READ_UINT16(d, 2 + ((i)  * 2))
#define SF_BACKTRACK_RECORD__INPUT_RECORD(d, gc)    SF_DATA__SUBDATA(d, 2 + ((gc) * 2))

#define SF_INPUT_RECORD__GLYPH_COUNT(d)             SF_DATA__READ_UINT16(d, 0)
#define SF_INPUT_RECORD__COVERAGE(d, i)             SF_DATA__READ_UINT16(d, 2 + ((i)  * 2))
#define SF_INPUT_RECORD__LOOKAHEAD_RECORD(d, gc)    SF_DATA__SUBDATA(d, 2 + ((gc) * 2))

#define SF_LOOKAHEAD_RECORD__GLYPH_COUNT(d)         SF_DATA__READ_UINT16(d, 0)
#define SF_LOOKAHEAD_RECORD__COVERAGE(d, i)         SF_DATA__READ_UINT16(d, 2 + ((i)  * 2))
#define SF_LOOKAHEAD_RECORD__CONTEXT_RECORD(d, gc)  SF_DATA__SUBDATA(d, 2 + ((gc) * 2))

#define SF_CONTEXT_RECORD__LOOKUP_COUNT(d)          SF_DATA__READ_UINT16(d, 0)
#define SF_CONTEXT_RECORD__LOOKUP_RECORD(d, i)      SF_DATA__SUBDATA(d, 2 + ((i) * 4))

/******************************************************************************/

/******************************EXTENSION SUBTABLE******************************/

#define SF_EXTENSION_FORMAT(d)                      SF_DATA__READ_UINT16(d, 0)
#define SF_EXTENSION_F1__LOOKUP_TYPE(d)             SF_DATA__READ_UINT16(d, 2)
#define SF_EXTENSION_F1__SUBTABLE(d)                SF_DATA__READ_UINT32(d, 4)

/******************************************************************************/

#endif
