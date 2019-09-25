/*
 * Copyright (C) 2019 Muhammad Tayyab Akram
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

#ifndef _SF_TABLES_AAT_H
#define _SF_TABLES_AAT_H

#include <SFConfig.h>

#include "Data.h"
#include "SFBase.h"

enum {
    ClassCodeEndOfText = 0,
    ClassCodeOutOfBounds = 1,
    ClassCodeDeletedGlyph = 2,
    ClassCodeEndOfLine = 3
};
typedef SFUInt8 ClassCode;

/**************************************BINARY SEARCH HEADER***************************************/

#define BinSrchHeader_UnitSize(data)                    Data_UInt16(data, 0)
#define BinSrchHeader_NUnits(data)                      Data_UInt16(data, 2)
#define BinSrchHeader_SearchRange(data)                 Data_UInt16(data, 4)
#define BinSrchHeader_EntrySelector(data)               Data_UInt16(data, 6)
#define BinSrchHeader_RangeShift(data)                  Data_UInt16(data, 8)

/**************************************************************************************************/

/*******************************************LOOKUP TABLE*******************************************/

#define Lookup_Format(data)                             Data_UInt16(data, 0)

#define LookupF0_ValueArray(data)                       Data_Subdata(data, 2)

#define LookupF2_BinSrchHeader(data)                    Data_Subdata(data, 2)
#define LookupF2_Segments(data)                         Data_Subdata(data, 12)

#define LookupF4_BinSrchHeader(data)                    Data_Subdata(data, 2)
#define LookupF4_Segments(data)                         Data_Subdata(data, 12)

#define LookupF6_BinSrchHeader(data)                    Data_Subdata(data, 2)
#define LookupF6_Entries(data)                          Data_Subdata(data, 12)

#define LookupF8_FirstGlyph(data)                       Data_UInt16(data, 2)
#define LookupF8_GlyphCount(data)                       Data_UInt16(data, 4)
#define LookupF8_ValueArray(data)                       Data_Subdata(data, 6)

#define LookupF10_UnitSize(data)                        Data_UInt16(data, 2)
#define LookupF10_FirstGlyph(data)                      Data_UInt16(data, 4)
#define LookupF10_GlyphCount(data)                      Data_UInt16(data, 6)
#define LookupF10_ValueArray(data)                      Data_Subdata(data, 8)

/**************************************************************************************************/

/******************************************LOOKUP SEGMENT******************************************/

#define LookupSegment_LastGlyph(data)                   Data_UInt16(data, 0)
#define LookupSegment_FirstGlyph(data)                  Data_UInt16(data, 2)
#define LookupSegment_ValueData(data)                   Data_Subdata(data, 4)

/**************************************************************************************************/

/******************************************LOOKUP SINGLE*******************************************/

#define LookupSingle_Glyph(data)                        Data_UInt16(data, 0)
#define LookupSingle_ValueData(data)                    Data_Subdata(data, 2)

/**************************************************************************************************/

/*******************************************STATE TABLE********************************************/

#define State_StateSize(data)                           Data_UInt16(data, 0)
#define State_ClassOffset(data)                         Data_UInt16(data, 2)
#define State_StatesOffset(data)                        Data_UInt16(data, 4)
#define State_EntryOffset(data)                         Data_UInt16(data, 6)
#define State_ClassTable(data) \
    Data_Subdata(data, State_ClassOffset(data))
#define State_StateArray(data) \
    Data_Subdata(data, State_StatesOffset(data))
#define State_EntryTable(data) \
    Data_Subdata(data, State_EntryOffset(data))

/**************************************************************************************************/

/******************************************CLASS SUBTABLE******************************************/

#define Class_FirstGlyph(data)                          Data_UInt16(data, 0)
#define Class_NGlyphs(data)                             Data_UInt16(data, 2)
#define Class_Code(data, index)                         Data_UInt8(data, 4 + (index))

/**************************************************************************************************/

/******************************************ENTRY SUBTABLE******************************************/

#define Entry_NewStateOffset(data)                      Data_UInt16(data, 0)
#define Entry_Flags(data)                               Data_UInt16(data, 2)
#define Entry_NewStateTable(data) \
    Data_Subdata(data, StateHeader_ClassOffset(data))

/**************************************************************************************************/

/***************************************EXTENDED STATE TABLE***************************************/

#define XState_NClasses(data)                           Data_UInt32(data, 0)
#define XState_ClassOffset(data)                        Data_UInt32(data, 4)
#define XState_StatesOffset(data)                       Data_UInt32(data, 8)
#define XState_EntryOffset(data)                        Data_UInt32(data, 12)
#define XState_ClassTable(data) \
    Data_Subdata(data, XState_ClassOffset(data))
#define XState_StateArray(data) \
    Data_Subdata(data, XState_StatesOffset(data))
#define XState_EntryTable(data) \
    Data_Subdata(data, XState_EntryOffset(data))

/**************************************************************************************************/

SF_INTERNAL Data GetLookupValueData(Data lookupTable, SFGlyphID glyphID, SFUInt16 valueSize);

#endif
