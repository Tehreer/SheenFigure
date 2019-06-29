/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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

#ifndef _SF_TABLES_GDEF_H
#define _SF_TABLES_GDEF_H

#include <stddef.h>

#include "Data.h"
#include "SFBase.h"

enum {
    GlyphClassValueNone = 0,
    GlyphClassValueBase = 1,     /**< Single character, spacing glyph */
    GlyphClassValueLigature = 2, /**< Multiple character, spacing glyph */
    GlyphClassValueMark = 3,     /**< Non-spacing combining glyph */
    GlyphClassValueComponent = 4 /**< Part of single character, spacing glyph */
};
typedef SFUInt16 GlyphClassValue;

/*******************************************GDEF HEADER********************************************/

#define GDEF_Version(data)                              Data_UInt32(data, 0)
#define GDEF_GlyphClassDefOffset(data)                  Data_UInt16(data, 4)
#define GDEF_AttachListOffset(data)                     Data_UInt16(data, 6)
#define GDEF_LigCaretListOffset(data)                   Data_UInt16(data, 8)
#define GDEF_MarkAttachClassDefOffset(data)             Data_UInt16(data, 10)
#define GDEFv12_MarkGlyphSetsDefOffset(data)            Data_UInt16(data, 12)
#define GDEFv13_ItemVarStoreOffset(data)                Data_UInt32(data, 14)
#define GDEF_GlyphClassDefTable(data) \
    Data_Subdata(data, GDEF_GlyphClassDefOffset(data))
#define GDEF_AttachListTable(data) \
    Data_Subdata(data, GDEF_AttachListOffset(data))
#define GDEF_LigCaretListTable(data) \
    Data_Subdata(data, GDEF_LigCaretListOffset(data))
#define GDEF_MarkAttachClassDefTable(data) \
    Data_Subdata(data, GDEF_MarkAttachClassDefOffset(data))
#define GDEFv12_MarkGlyphSetsDefTable(data) \
    Data_Subdata(data, GDEFv12_MarkGlyphSetsDefOffset(data))
#define GDEFv13_ItemVarStoreTable(data) \
    Data_Subdata(data, GDEFv13_ItemVarStoreOffset(data))

#define GDEF_MarkGlyphSetsDefTable(data) \
    (GDEF_Version(data) >= 0x00010002 ? GDEFv12_MarkGlyphSetsDefTable(data) : NULL)
#define GDEF_ItemVarStoreTable(data) \
    (GDEF_Version(data) >= 0x00010003 ? GDEFv13_ItemVarStoreTable(data) : NULL)

/**************************************************************************************************/

/**************************************ATTACHMENT LIST TABLE***************************************/

#define AttachList_CoverageOffset(data)                 Data_UInt16(data, 0)
#define AttachList_GlyphCount(data)                     Data_UInt16(data, 2)
#define AttachList_AttachPointOffset(data, index)       Data_UInt16(data, 4 + ((index) * 2))
#define AttachList_CoverageTable(data) \
    Data_Subdata(data, AttachList_CoverageOffset(data))
#define AttachList_AttachPointTable(data, index) \
    Data_Subdata(data, AttachList_AttachPointOffset(data, index))

#define AttachPoint_PointCount(data)                    Data_UInt16(data, 0)
#define AttachPoint_PointIndex(data, index)             Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/************************************LIGATURE CARET LIST TABLE*************************************/

#define LigCarretList_CoverageOffset(data)              Data_UInt16(data, 0)
#define LigCarretList_LigGlyphCount(data)               Data_UInt16(data, 2)
#define LigCarretList_LigGlyphOffset(data, index)       Data_UInt16(data, 4 + ((index) * 2))
#define LigCarretList_CoverageTable(data) \
    Data_Subdata(data, LigCarretList_CoverageOffset(data))
#define LigCarretList_LigGlyphIndex(data, index) \
    Data_Subdata(data, LigCarretList_LigGlyphOffset(data, index))

#define LigGlyph_CaretCount(data)                       Data_UInt16(data, 0)
#define LigGlyph_CaretValueOffset(data, index)          Data_UInt16(data, 2 + ((index) * 2))
#define LigGlyph_CaretValueTable(data, index) \
    Data_Subdata(data, LigGlyph_CaretValueOffset(data, index))

#define CaretValue_Format(data)                         Data_UInt16(data, 0)

#define CaretValueF1_Coordinate(data)                   Data_Int16(data, 2)

#define CaretValueF2_CaretValuePoint(data)              Data_UInt16(data, 2)

#define CaretValueF3_Coordinate(data)                   Data_Int16 (data, 2)
#define CaretValueF3_DeviceOffset(data)                 Data_UInt16(data, 4)
#define CaretValueF3_DeviceTable(data) \
    Data_Subdata(data, CaretValueF3_DeviceOffset(data))

/**************************************************************************************************/

/**************************************MARK GLYPH SETS TABLE***************************************/

#define MarkGlyphSets_Format(data)                      Data_UInt16(data, 0)
#define MarkGlyphSets_MarkSetCount(data)                Data_UInt16(data, 2)
#define MarkGlyphSets_CoverageOffset(data, index)       Data_UInt32(data, 4 + ((index) * 4))
#define MarkGlyphSets_CoverageTable(data, index) \
    Data_Subdata(data, MarkGlyphSets_CoverageOffset(data, index))

/**************************************************************************************************/

#endif
