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

#ifndef _SF_GDEF_INTERNAL_H
#define _SF_GDEF_INTERNAL_H

#include "SFBase.h"
#include "SFData.h"

enum {
    SFGlyphClassValueNone = 0,
    SFGlyphClassValueBase = 1,     /**< Single character, spacing glyph */
    SFGlyphClassValueLigature = 2, /**< Multiple character, spacing glyph */
    SFGlyphClassValueMark = 3,     /**< Non-spacing combining glyph */
    SFGlyphClassValueComponent = 4 /**< Part of single character, spacing glyph */
};
typedef SFUInt16 SFGlyphClassValue;

/*******************************************GSUB HEADER********************************************/

#define SFGDEF_Version(data)                            SFData_UInt32(data, 0)
#define SFGDEF_GlyphClassDefOffset(data)                SFData_UInt16(data, 4)
#define SFGDEF_AttachListOffset(data)                   SFData_UInt16(data, 6)
#define SFGDEF_LigCarretListOffset(data)                SFData_UInt16(data, 8)
#define SFGDEF_MarkAttachClassDefOffset(data)           SFData_UInt16(data, 10)
#define SFGDEF_MarkGlyphSetsDefOffset(data)             SFData_UInt16(data, 12)

/**************************************************************************************************/

/**************************************ATTACHMENT LIST TABLE***************************************/

#define SFAttachList_CoverageOffset(data)               SFData_UInt16(data, 0)
#define SFAttachList_GlyphCount(data)                   SFData_UInt16(data, 2)
#define SFAttachList_AttachPointOffset(data)            SFData_UInt16(data, 4)

#define SFAttachPoint_PointCount(data)                  SFData_UInt16(data, 0)
#define SFAttachPoint_PointIndex(data, index)           SFData_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/************************************LIGATURE CARET LIST TABLE*************************************/

#define SFLigCarretList_CoverageOffset(data)            SFData_UInt16(data, 0)
#define SFLigCarretList_LigGlyphCount(data)             SFData_UInt16(data, 2)
#define SFLigCarretList_LigGlyphOffset(data, index)     SFData_UInt16(data, 4 + ((index) * 2))

#define SFLigGlyph_CaretCount(data)                     SFData_UInt16(data, 0)
#define SFLigGlyph_CaretValueOffset(data, index)        SFData_UInt16(data, 2 + ((index) * 2))

#define SFCaretValue_Format(data)                       SFData_UInt16(data, 0)

#define SFCaretValueF1_Coordinate(data)                 SFData_Int16(data, 2)

#define SFCaretValueF2_CaretValuePoint(data)            SFData_UInt16(data, 2)

#define SFCaretValueF3_Coordinate(data)                 SFData_Int16(data, 2)
#define SFCaretValueF3_DeviceTableOffset(data)          SFData_UInt16(data, 4)

/**************************************************************************************************/

/**************************************MARK GLYPH SETS TABLE***************************************/

#define SFMarkGlyphSets_Format(data)                    SFData_UInt16(data, 0)
#define SFMarkGlyphSets_MarkSetCount(data)              SFData_UInt16(data, 2)
#define SFMarkGlyphSets_CoverageOffset(data, index)     SFData_UInt32(data, 4 + ((index) * 4))

/**************************************************************************************************/

#endif
