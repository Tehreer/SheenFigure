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

#ifndef SF_GSUB_INTERNAL_H
#define SF_GSUB_INTERNAL_H

#include "SFData.h"
#include "SFCommon.h"

enum {
    SFLookupTypeSingle = 1,
    SFLookupTypeMultiple = 2,
    SFLookupTypeAlternate = 3,
    SFLookupTypeLigature = 4,
    SFLookupTypeContext = 5,
    SFLookupTypeChainingContext = 6,
    SFLookupTypeExtension = 7,
    SFLookupTypeReverseChainingContext = 8
};

/*********************************GSUB HEADER*********************************/

#define SF_GSUB__VERSION(d)                         SF_HEADER__VERSION(d)
#define SF_GSUB__SCRIPT_LIST(d)                     SF_HEADER__SCRIPT_LIST(d)
#define SF_GSUB__FEATURE_LIST(d)                    SF_HEADER__FEATURE_LIST(d)
#define SF_GSUB__LOOKUP_LIST(d)                     SF_HEADER__LOOKUP_LIST(d)

/*****************************************************************************/

/************************SINGLE SUBSTITUTION SUBTABLE*************************/

#define SF_SINGLE_SUBST_FORMAT(d)                   SF_DATA__READ_UINT16(d, 0)

#define SF_SINGLE_SUBST_F1__COVERAGE(d)             SF_DATA__READ_UINT16(d, 2)
#define SF_SINGLE_SUBST_F1__DELTA_GLYPH_ID(d)       SF_DATA__READ_INT16 (d, 4)

#define SF_SINGLE_SUBST_F2__COVERAGE(d)             SF_DATA__READ_UINT16(d, 2)
#define SF_SINGLE_SUBST_F2__GLYPH_COUNT(d)          SF_DATA__READ_UINT16(d, 4)
#define SF_SINGLE_SUBST_F2__SUBSTITUTE(d, i)        SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

/*****************************************************************************/

/***********************MULTIPLE SUBSTITUTION SUBTABLE************************/

#define SF_MULTIPLE_SUBST_FORMAT(d)                 SF_DATA__READ_UINT16(d, 0)
#define SF_MULTIPLE_SUBST_F1__COVERAGE(d)           SF_DATA__READ_UINT16(d, 2)
#define SF_MULTIPLE_SUBST_F1__SEQUENCE_COUNT(d)     SF_DATA__READ_UINT16(d, 4)
#define SF_MULTIPLE_SUBST_F1__SEQUENCE(d, i)        SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

#define SF_SEQUENCE__GLYPH_COUNT(d)                 SF_DATA__READ_UINT16(d, 0)
#define SF_SEQUENCE__SUBSTITUTE(d, i)               SF_DATA__READ_UINT16(d, 2 + ((i) * 2))

/*****************************************************************************/

/***********************ALTERNATE SUBSTITUTION SUBTABLE***********************/

#define SF_ALTERNATE_SUBST_FORMAT(d)                SF_DATA__READ_UINT16(d, 0)
#define SF_ALTERNATE_SUBST_F1__COVERATE(d)          SF_DATA__READ_UINT16(d, 2)
#define SF_ALTERNATE_SUBST_F1__ALTERNATE_SET_COUNT(d) \
                                                    SF_DATA__READ_UINT16(d, 4)
#define SF_ALTERNATE_SUBST_F1__ALTERNATE_SET(d, i)  SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

#define SF_ALTERNATE_SET__GLYPH_COUNT(d)            SF_DATA__READ_UINT16(d, 0)
#define SF_ALTERNATE_SET__ALTERNATE(d, i)           SF_DATA__READ_UINT16(d, 2 * ((i) * 2))

/*****************************************************************************/

/***********************LIGATURE SUBSTITUTION SUBTABLE************************/

#define SF_LIGATURE_SUBST_FORMAT(d)                 SF_DATA__READ_UINT16(d, 0)
#define SF_LIGATURE_SUBST_F1__COVERAGE(d)           SF_DATA__READ_UINT16(d, 2)
#define SF_LIGATURE_SUBST_F1__LIG_SET_COUNT(d)      SF_DATA__READ_UINT16(d, 4)
#define SF_LIGATURE_SUBST_F1__LIGATURE_SET(d, i)    SF_DATA__READ_UINT16(d, 6 + ((i) * 2))

#define SF_LIGATURE_SET__LIGATURE_COUNT(d)          SF_DATA__READ_UINT16(d, 0)
#define SF_LIGATURE_SET__LIGATURE(d, i)             SF_DATA__READ_UINT16(d, 2 + ((i) * 2))

#define SF_LIGATURE__LIG_GLYPH(d)                   SF_DATA__READ_UINT16(d, 0)
#define SF_LIGATURE__COMP_COUNT(d)                  SF_DATA__READ_UINT16(d, 2)
#define SF_LIGATURE__COMPONENT(d, i)                SF_DATA__READ_UINT16(d, 4 + ((i) * 2))

/*****************************************************************************/

#endif
