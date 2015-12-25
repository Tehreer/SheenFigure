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

#ifndef SF_GPOS_INTERNAL_H
#define SF_GPOS_INTERNAL_H

#include "SFData.h"

enum {
    SFLookupTypeSingleAdjustment = 1,
    SFLookupTypePairAdjustment = 2,
    SFLookupTypeCursiveAttachment = 3,
    SFLookupTypeMarkToBaseAttachment = 4,
    SFLookupTypeMarkToLigatureAttachment = 5,
    SFLookupTypeMarkToMarkAttachment = 6,
    SFLookupTypeContextPositioning = 7,
    SFLookupTypeChainedContextPositioning = 8,
    SFLookupTypeExtensionPositioning = 9
};

/*********************************GPOS HEADER*********************************/

#define SF_GPOS_VERSION(t)                          SF_DATA__READ_UINT32(t, 0)
#define SF_GPOS_SCRIPT_LIST(t)                      SF_DATA__READ_UINT16(t, 4)
#define SF_GPOS_FEATURE_LIST(t)                     SF_DATA__READ_UINT16(t, 6)
#define SF_GPOS_LOOKUP_LIST(t)                      SF_DATA__READ_UINT16(t, 8)

/*****************************************************************************/

/********************************VALUE RECORD*********************************/

#define SF_VALUE_FORMAT__X_PLACEMENT(f)             ((f) & 0x0001 ? 1 : 0)
#define SF_VALUE_FORMAT__Y_PLACEMENT(f)             ((f) & 0x0002 ? 1 : 0)
#define SF_VALUE_FORMAT__X_ADVANCE(f)               ((f) & 0x0004 ? 1 : 0)
#define SF_VALUE_FORMAT__Y_ADVANCE(f)               ((f) & 0x0010 ? 1 : 0)
#define SF_VALUE_FORMAT__X_PLA_DEVICE(f)            ((f) & 0x0020 ? 1 : 0)
#define SF_VALUE_FORMAT__Y_PLA_DEVICE(f)            ((f) & 0x0040 ? 1 : 0)
#define SF_VALUE_FORMAT__X_ADV_DEVICE(f)            ((f) & 0x0080 ? 1 : 0)
#define SF_VALUE_FORMAT__Y_ADV_DEVICE(f)            ((f) & 0xF000 ? 1 : 0)
#define SF_VALUE_FORMAT__VALUE_COUNT(f)             \
(                                                   \
   SF_VALUE_FORMAT__X_PLACEMENT(f)                  \
 + SF_VALUE_FORMAT__Y_PLACEMENT(f)                  \
 + SF_VALUE_FORMAT__X_ADVANCE(f)                    \
 + SF_VALUE_FORMAT__Y_ADVANCE(f)                    \
 + SF_VALUE_FORMAT__X_PLA_DEVICE(f)                 \
 + SF_VALUE_FORMAT__Y_PLA_DEVICE(f)                 \
 + SF_VALUE_FORMAT__X_ADV_DEVICE(f)                 \
 + SF_VALUE_FORMAT__Y_ADV_DEVICE(f)                 \
)

#define SF_VALUE_RECORD__NEXT_VALUE(d, o)           SF_DATA__READ_UINT16(d, o), (o += 2)
#define SF_VALUE_RECORD__SIZE(f)                    \
(                                                   \
   (f) ? 2 * SF_VALUE_FORMAT__VALUE_COUNT(f) : 0    \
)

/*****************************************************************************/

/*******************SINGLE ADJUSTMENT POSITIONING SUBTABLE********************/

#define SF_SINGLE_POS_FORMAT(d)                     SF_DATA__READ_UINT16(d, 0)
#define SF_SINGLE_POS_F1__COVERAGE(d)               SF_DATA__READ_UINT16(d, 2)
#define SF_SINGLE_POS_F1__VALUE_FORMAT(d)           SF_DATA__READ_UINT16(d, 4)
#define SF_SINGLE_POS_F1__VALUE_RECORD(d)           SF_DATA__SUBDATA(d, 6)

#define SF_SINGLE_POS_F2__COVERAGE(d)               SF_DATA__READ_UINT16(d, 2)
#define SF_SINGLE_POS_F2__VALUE_FORMAT(d)           SF_DATA__READ_UINT16(d, 4)
#define SF_SINGLE_POS_F2__VALUE_COUNT(d)            SF_DATA__READ_UINT16(d, 6)
#define SF_SINGLE_POS_F2__VALUE_RECORD(d, i, s)     SF_DATA__SUBDATA(d, 8 + ((i) * (s)))

#define SF_PAIR_SET__PAIR_VALUE_COUNT(d)            SF_DATA__READ_UINT16(d, 0)
#define SF_PAIR_SET__PAIR_VALUE_RECORD(d, i, s)     SF_DATA__SUBDATA(d, 2 + ((i) * (s)))
#define SF_PAIR_SET__PAIR_VALUE_RECORD_ARRAY(d)     SF_PAIR_SET__PAIR_VALUE_RECORD(d, 0, 0)

#define SF_PAIR_VALUE_RECORD__SIZE(s1, s2)          (2 + s1 + s2)
#define SF_PAIR_VALUE_RECORD__SECOND_GLYPH(d)       SF_DATA__READ_UINT16(d, 0)
#define SF_PAIR_VALUE_RECORD__VALUE1(d)             SF_DATA__SUBDATA(d, 2)
#define SF_PAIR_VALUE_RECORD__VALUE2(d, s1)         SF_DATA__SUBDATA(d, 2 + (s1))

/*****************************************************************************/

/********************PAIR ADJUSTMENT POSITIONING SUBTABLE*********************/

#define SF_PAIR_POS_FORMAT(d)                       SF_DATA__READ_UINT16(d, 0)
#define SF_PAIR_POS_F1__COVERAGE(d)                 SF_DATA__READ_UINT16(d, 2)
#define SF_PAIR_POS_F1__VALUE_FORMAT1(d)            SF_DATA__READ_UINT16(d, 4)
#define SF_PAIR_POS_F1__VALUE_FORMAT2(d)            SF_DATA__READ_UINT16(d, 6)
#define SF_PAIR_POS_F1__PAIR_SET_COUNT(d)           SF_DATA__READ_UINT16(d, 8)
#define SF_PAIR_POS_F1__PAIR_SET(d, i)              SF_DATA__READ_UINT16(d, 10 + ((i) * 2))

#define SF_PAIR_POS_F2__COVERAGE(d)                 SF_DATA__READ_UINT16(d, 2)
#define SF_PAIR_POS_F2__VALUE_FORMAT1(d)            SF_DATA__READ_UINT16(d, 4)
#define SF_PAIR_POS_F2__VALUE_FORMAT2(d)            SF_DATA__READ_UINT16(d, 6)
#define SF_PAIR_POS_F2__CLASS_DEF1(d)               SF_DATA__READ_UINT16(d, 8)
#define SF_PAIR_POS_F2__CLASS_DEF2(d)               SF_DATA__READ_UINT16(d, 10)
#define SF_PAIR_POS_F2__CLASS1_COUNT(d)             SF_DATA__READ_UINT16(d, 12)
#define SF_PAIR_POS_F2__CLASS2_COUNT(d)             SF_DATA__READ_UINT16(d, 14)
#define SF_PAIR_POS_F2__CLASS1_RECORD(d, i, c1s)    SF_DATA__SUBDATA(d, 16 + ((i) * (c1s)))

#define SF_CLASS1_RECORD__SIZE(c2c, c2s)            (c2c * c2s)
#define SF_CLASS1_RECORD__CLASS2_RECORD(d, i, c2s)  SF_DATA__SUBDATA(d, (i) * (c2s))

#define SF_CLASS2_RECORD__SIZE(s1, s2)              (s1 + s2)
#define SF_CLASS2_RECORD__VALUE1(d)                 SF_DATA__SUBDATA(d, 0)
#define SF_CLASS2_RECORD__VALUE2(d, s1)             SF_DATA__SUBDATA(d, s1)

/*****************************************************************************/

/*******************CURSIVE ATTACHMENT POSITIONING SUBTABLE*******************/

#define SF_CURSIVE_POS__FORMAT(d)                   SF_DATA__READ_UINT16(d, 0)
#define SF_CURSIVE_POS__COVERAGE(d)                 SF_DATA__READ_UINT16(d, 2)
#define SF_CURSIVE_POS__ENTRY_EXIT_COUNT(d)         SF_DATA__READ_UINT16(d, 4)
#define SF_CURSIVE_POS__ENTRY_EXIT_RECORD(d, i)     SF_DATA__SUBDATA(d, 6 + ((i) * 4))

#define SF_ENTRY_EXIT_RECORD__ENTRY_ANCHOR(d)       SF_DATA__READ_UINT16(d, 0)
#define SF_ENTRY_EXIT_RECORD__EXIT_ANCHOR(d)        SF_DATA__READ_UINT16(d, 2)

/*****************************************************************************/

/****************MARK TO BASE ATTACHMENT POSITIONING SUBTABLE*****************/

#define SF_MARK_BASE_POS__FORMAT(d)                 SF_DATA__READ_UINT16(d, 0)
#define SF_MARK_BASE_POS__MARK_COVERAGE(d)          SF_DATA__READ_UINT16(d, 2)
#define SF_MARK_BASE_POS__BASE_COVERAGE(d)          SF_DATA__READ_UINT16(d, 4)
#define SF_MARK_BASE_POS__CLASS_COUNT(d)            SF_DATA__READ_UINT16(d, 6)
#define SF_MARK_BASE_POS__MARK_ARRAY(d)             SF_DATA__READ_UINT16(d, 8)
#define SF_MARK_BASE_POS__BASE_ARRAY(d)             SF_DATA__READ_UINT16(d, 10)

#define SF_BASE_ARRAY__BASE_COUNT(d)                SF_DATA__READ_UINT16(d, 0)
#define SF_BASE_ARRAY__BASE_RECORD(d, i, cc)        SF_DATA__SUBDATA(d, 2 + ((i) * (2 * (cc))))

#define SF_BASE_RECORD__BASE_ANCHOR(d, i)           SF_DATA__READ_UINT16(d, 0 + ((i) * 2))

/*****************************************************************************/

/**************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE***************/

#define SF_MARK_LIG_POS__FORMAT(d)                  SF_DATA__READ_UINT16(d, 0)
#define SF_MARK_LIG_POS__MARK_COVERAGE(d)           SF_DATA__READ_UINT16(d, 2)
#define SF_MARK_LIG_POS__LIGATURE_COVERAGE(d)       SF_DATA__READ_UINT16(d, 4)
#define SF_MARK_LIG_POS__CLASS_COUNT(d)             SF_DATA__READ_UINT16(d, 6)
#define SF_MARK_LIG_POS__MARK_ARRAY(d)              SF_DATA__READ_UINT16(d, 8)
#define SF_MARK_LIG_POS__LIGATURE_ARRAY(d)          SF_DATA__READ_UINT16(d, 10)

#define SF_LIGATURE_ARRAY__LIGATURE_COUNT(d)        SF_DATA__READ_UINT16(d, 0)
#define SF_LIGATURE_ARRAY__LIGATURE_ATTACH(d, i)    SF_DATA__READ_UINT16(d, 2 + ((i) * 2))

#define SF_LIGATURE_ATTACH__COMPONENT_COUNT(d)      SF_DATA__READ_UINT16(d, 0)
#define SF_LIGATURE_ATTACH__COMPONENT_RECORD(d, i, cc) \
                                                    SF_DATA__SUBDATA(d, 2 + ((i) * (2 * (cc))))

#define SF_COMPONENT_RECORD__LIGATURE_ANCHOR(d, i)  SF_DATA__READ_UINT16(d, 0 + ((i) * 2))

/*****************************************************************************/

/**************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE***************/

#define SF_MARK_MARK_POS__FORMAT(d)                 SF_DATA__READ_UINT16(d, 0)
#define SF_MARK_MARK_POS__MARK1_COVERAGE(d)         SF_DATA__READ_UINT16(d, 2)
#define SF_MARK_MARK_POS__MARK2_COVERAGE(d)         SF_DATA__READ_UINT16(d, 4)
#define SF_MARK_MARK_POS__CLASS_COUNT(d)            SF_DATA__READ_UINT16(d, 6)
#define SF_MARK_MARK_POS__MARK1_ARRAY(d)            SF_DATA__READ_UINT16(d, 8)
#define SF_MARK_MARK_POS__MARK2_ARRAY(d)            SF_DATA__READ_UINT16(d, 10)

#define SF_MARK2_ARRAY__MARK2_COUNT(d)              SF_DATA__READ_UINT16(d, 0)
#define SF_MARK2_ARRAY__MARK2_RECORD(d, i, cc)      SF_DATA__SUBDATA(d, 2 + ((i) * (2 * (cc))))

#define SF_MARK2_RECORD__MARK2_ANCHOR(d, i)         SF_DATA__READ_UINT16(d, 2 + ((i) * 2))

/*****************************************************************************/

/********************************ANCHOR TABLE*********************************/

#define SF_ANCHOR_FORMAT(d)                         SF_DATA__READ_UINT16(d, 0)

#define SF_ANCHOR_F1__X_COORDINATE(d)               SF_DATA__READ_INT16 (d, 2)
#define SF_ANCHOR_F1__Y_COORDINATE(d)               SF_DATA__READ_INT16 (d, 4)

#define SF_ANCHOR_F2__X_COORDINATE(d)               SF_DATA__READ_INT16 (d, 2)
#define SF_ANCHOR_F2__Y_COORDINATE(d)               SF_DATA__READ_INT16 (d, 4)
#define SF_ANCHOR_F2__ANCHOR_POINT(d)               SF_DATA__READ_UINT16(d, 6)

#define SF_ANCHOR_F3__X_COORDINATE(d)               SF_DATA__READ_INT16 (d, 2)
#define SF_ANCHOR_F3__Y_COORDINATE(d)               SF_DATA__READ_INT16 (d, 4)
#define SF_ANCHOR_F3__ANCHOR_POINT(d)               SF_DATA__READ_UINT16(d, 6)
#define SF_ANCHOR_F3__X_DEVICE(d)                   SF_DATA__READ_UINT16(d, 8)
#define SF_ANCHOR_F3__Y_DEVICE(d)                   SF_DATA__READ_UINT16(d, 10)

/*****************************************************************************/

/*********************************MARK ARRAY**********************************/

#define SF_MARK_ARRAY__MARK_COUNT(d)                SF_DATA__READ_UINT16(d, 0)
#define SF_MARK_ARRAY__MARK_RECORD(d, i)            SF_DATA__SUBDATA(d, 2 + ((i) * 4))

#define SF_MARK_RECORD__CLASS(d)                    SF_DATA__READ_UINT16(d, 0)
#define SF_MARK_RECORD__MARK_ANCHOR(d)              SF_DATA__READ_UINT16(d, 2)

/*****************************************************************************/

#endif
