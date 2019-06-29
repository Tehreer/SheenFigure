/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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

#ifndef _SF_TABLES_GPOS_H
#define _SF_TABLES_GPOS_H

#include "Data.h"

enum {
    LookupTypeSingleAdjustment = 1,
    LookupTypePairAdjustment = 2,
    LookupTypeCursiveAttachment = 3,
    LookupTypeMarkToBaseAttachment = 4,
    LookupTypeMarkToLigatureAttachment = 5,
    LookupTypeMarkToMarkAttachment = 6,
    LookupTypeContextPositioning = 7,
    LookupTypeChainedContextPositioning = 8,
    LookupTypeExtensionPositioning = 9
};

/*******************************************GPOS HEADER********************************************/

#define GPOS_Version(data)                          Data_UInt32(data, 0)
#define GPOS_ScriptListOffset(data)                 Data_UInt16(data, 4)
#define GPOS_FeatureListOffset(data)                Data_UInt16(data, 6)
#define GPOS_LookupListOffset(data)                 Data_UInt16(data, 8)

/**************************************************************************************************/

/*******************************************VALUE RECORD*******************************************/

#define ValueFormat_XPlacement(format)              ((format) & 0x0001 ? 1 : 0)
#define ValueFormat_YPlacement(format)              ((format) & 0x0002 ? 1 : 0)
#define ValueFormat_XAdvance(format)                ((format) & 0x0004 ? 1 : 0)
#define ValueFormat_YAdvance(format)                ((format) & 0x0008 ? 1 : 0)
#define ValueFormat_XPlaDevice(format)              ((format) & 0x0010 ? 1 : 0)
#define ValueFormat_YPlaDevice(format)              ((format) & 0x0020 ? 1 : 0)
#define ValueFormat_XAdvDevice(format)              ((format) & 0x0040 ? 1 : 0)
#define ValueFormat_YAdvDevice(format)              ((format) & 0x0080 ? 1 : 0)
#define ValueFormat_Reserved(format)                ((format) & 0xFF00 ? 1 : 0)
#define ValueFormat_ValueCount(format)              \
(                                                   \
   ValueFormat_XPlacement(format)                   \
 + ValueFormat_YPlacement(format)                   \
 + ValueFormat_XAdvance(format)                     \
 + ValueFormat_YAdvance(format)                     \
 + ValueFormat_XPlaDevice(format)                   \
 + ValueFormat_YPlaDevice(format)                   \
 + ValueFormat_XAdvDevice(format)                   \
 + ValueFormat_YAdvDevice(format)                   \
 + ValueFormat_Reserved(format)                     \
)

#define ValueRecord_Size(format)                    \
(                                                   \
   (format)                                         \
 ? (2 * ValueFormat_ValueCount(format))             \
 : (0)                                              \
)

/**************************************************************************************************/

/******************************SINGLE ADJUSTMENT POSITIONING SUBTABLE******************************/

#define SinglePos_Format(data)                      Data_UInt16(data, 0)

#define SinglePosF1_CoverageOffset(data)            Data_UInt16(data, 2)
#define SinglePosF1_ValueFormat(data)               Data_UInt16(data, 4)
#define SinglePosF1_ValueRecord(data)               Data_Subdata(data, 6)
#define SinglePosF1_CoverageTable(data) \
    Data_Subdata(data, SinglePosF1_CoverageOffset(data))

#define SinglePosF2_CoverageOffset(data)            Data_UInt16(data, 2)
#define SinglePosF2_ValueFormat(data)               Data_UInt16(data, 4)
#define SinglePosF2_ValueCount(data)                Data_UInt16(data, 6)
#define SinglePosF2_ValueRecord(data, index, recordSize) \
                                                    Data_Subdata(data, 8 + ((index) * (recordSize)))
#define SinglePosF2_CoverageTable(data) \
    Data_Subdata(data, SinglePosF2_CoverageOffset(data))

#define PairSet_PairValueCount(data)                Data_UInt16(data, 0)
#define PairSet_PairValueRecord(data, index, recordSize) \
                                                    Data_Subdata(data, 2 + ((index) * (recordSize)))
#define PairSet_PairValueRecordArray(data)          PairSet_PairValueRecord(data, 0, 0)

#define PairValueRecord_Size(value1Size, value2Size) \
                                                    (2 + (value1Size) + (value2Size))
#define PairValueRecord_SecondGlyph(data)           Data_UInt16(data, 0)
#define PairValueRecord_Value1(data)                Data_Subdata(data, 2)
#define PairValueRecord_Value2(data, value1Size)    Data_Subdata(data, 2 + (value1Size))

/**************************************************************************************************/

/*******************************PAIR ADJUSTMENT POSITIONING SUBTABLE*******************************/

#define PairPos_Format(data)                        Data_UInt16(data, 0)

#define PairPosF1_CoverageOffset(data)              Data_UInt16(data, 2)
#define PairPosF1_ValueFormat1(data)                Data_UInt16(data, 4)
#define PairPosF1_ValueFormat2(data)                Data_UInt16(data, 6)
#define PairPosF1_PairSetCount(data)                Data_UInt16(data, 8)
#define PairPosF1_PairSetOffset(data, index)        Data_UInt16(data, 10 + ((index) * 2))
#define PairPosF1_CoverageTable(data) \
    Data_Subdata(data, PairPosF1_CoverageOffset(data))
#define PairPosF1_PairSetTable(data, index) \
    Data_Subdata(data, PairPosF1_PairSetOffset(data, index))

#define PairPosF2_CoverageOffset(data)              Data_UInt16(data, 2)
#define PairPosF2_ValueFormat1(data)                Data_UInt16(data, 4)
#define PairPosF2_ValueFormat2(data)                Data_UInt16(data, 6)
#define PairPosF2_ClassDef1Offset(data)             Data_UInt16(data, 8)
#define PairPosF2_ClassDef2Offset(data)             Data_UInt16(data, 10)
#define PairPosF2_Class1Count(data)                 Data_UInt16(data, 12)
#define PairPosF2_Class2Count(data)                 Data_UInt16(data, 14)
#define PairPosF2_Class1Record(data, index, c1Size) Data_Subdata(data, 16 + ((index) * (c1Size)))
#define PairPosF2_CoverageTable(data) \
    Data_Subdata(data, PairPosF2_CoverageOffset(data))
#define PairPosF2_ClassDef1Table(data) \
    Data_Subdata(data, PairPosF2_ClassDef1Offset(data))
#define PairPosF2_ClassDef2Table(data) \
    Data_Subdata(data, PairPosF2_ClassDef2Offset(data))

#define Class1Record_Size(c2Count, c2Size)          ((c2Count) * (c2Size))
#define Class1Record_Class2Record(data, index, c2Size)\
                                                    Data_Subdata(data, (index) * (c2Size))

#define Class2Record_Size(v1Size, v2Size)           ((v1Size) + (v2Size))
#define Class2Record_Value1(data)                   Data_Subdata(data, 0)
#define Class2Record_Value2(data, v1Size)           Data_Subdata(data, v1Size)

/**************************************************************************************************/

/*****************************CURSIVE ATTACHMENT POSITIONING SUBTABLE******************************/

#define CursivePos_Format(data)                     Data_UInt16(data, 0)
#define CursivePos_CoverageOffset(data)             Data_UInt16(data, 2)
#define CursivePos_EntryExitCount(data)             Data_UInt16(data, 4)
#define CursivePos_EntryExitRecord(data, index)     Data_Subdata(data, 6 + ((index) * 4))
#define CursivePos_CoverageTable(data) \
    Data_Subdata(data, CursivePos_CoverageOffset(data))

#define EntryExitRecord_EntryAnchorOffset(data)     Data_UInt16(data, 0)
#define EntryExitRecord_ExitAnchorOffset(data)      Data_UInt16(data, 2)

/**************************************************************************************************/

/***************************MARK TO BASE ATTACHMENT POSITIONING SUBTABLE***************************/

#define MarkBasePos_Format(data)                    Data_UInt16(data, 0)
#define MarkBasePos_MarkCoverageOffset(data)        Data_UInt16(data, 2)
#define MarkBasePos_BaseCoverageOffset(data)        Data_UInt16(data, 4)
#define MarkBasePos_ClassCount(data)                Data_UInt16(data, 6)
#define MarkBasePos_MarkArrayOffset(data)           Data_UInt16(data, 8)
#define MarkBasePos_BaseArrayOffset(data)           Data_UInt16(data, 10)
#define MarkBasePos_MarkCoverageTable(data) \
    Data_Subdata(data, MarkBasePos_MarkCoverageOffset(data))
#define MarkBasePos_BaseCoverageTable(data) \
    Data_Subdata(data, MarkBasePos_BaseCoverageOffset(data))
#define MarkBasePos_MarkArrayTable(data) \
    Data_Subdata(data, MarkBasePos_MarkArrayOffset(data))
#define MarkBasePos_BaseArrayTable(data) \
    Data_Subdata(data, MarkBasePos_BaseArrayOffset(data))

#define BaseArray_BaseCount(data)                   Data_UInt16(data, 0)
#define BaseArray_BaseRecord(data, index, classCount) \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define BaseArray_BaseAnchorOffset(data, index)     Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define MarkLigPos_Format(data)                     Data_UInt16(data, 0)
#define MarkLigPos_MarkCoverageOffset(data)         Data_UInt16(data, 2)
#define MarkLigPos_LigatureCoverageOffset(data)     Data_UInt16(data, 4)
#define MarkLigPos_ClassCount(data)                 Data_UInt16(data, 6)
#define MarkLigPos_MarkArrayOffset(data)            Data_UInt16(data, 8)
#define MarkLigPos_LigatureArrayOffset(data)        Data_UInt16(data, 10)
#define MarkLigPos_MarkCoverageTable(data) \
    Data_Subdata(data, MarkLigPos_MarkCoverageOffset(data))
#define MarkLigPos_LigatureCoverageTable(data) \
    Data_Subdata(data, MarkLigPos_LigatureCoverageOffset(data))
#define MarkLigPos_MarkArrayTable(data) \
    Data_Subdata(data, MarkLigPos_MarkArrayOffset(data))
#define MarkLigPos_LigatureArrayTable(data) \
    Data_Subdata(data, MarkLigPos_LigatureArrayOffset(data))

#define LigatureArray_LigatureCount(data)           Data_UInt16(data, 0)
#define LigatureArray_LigatureAttachOffset(data, index) \
                                                    Data_UInt16(data, 2 + ((index) * 2))
#define LigatureArray_LigatureAttachTable(data, index) \
    Data_Subdata(data, LigatureArray_LigatureAttachOffset(data, index))

#define LigatureAttach_ComponentCount(data)         Data_UInt16(data, 0)
#define LigatureAttach_ComponentRecord(data, index, classCount) \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define ComponentRecord_LigatureAnchorOffset(data, index) \
                                                    Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define MarkMarkPos_Format(data)                    Data_UInt16(data, 0)
#define MarkMarkPos_Mark1CoverageOffset(data)       Data_UInt16(data, 2)
#define MarkMarkPos_Mark2CoverageOffset(data)       Data_UInt16(data, 4)
#define MarkMarkPos_ClassCount(data)                Data_UInt16(data, 6)
#define MarkMarkPos_Mark1ArrayOffset(data)          Data_UInt16(data, 8)
#define MarkMarkPos_Mark2ArrayOffset(data)          Data_UInt16(data, 10)
#define MarkMarkPos_Mark1CoverageTable(data) \
    Data_Subdata(data, MarkMarkPos_Mark1CoverageOffset(data))
#define MarkMarkPos_Mark2CoverageTable(data) \
    Data_Subdata(data, MarkMarkPos_Mark2CoverageOffset(data))
#define MarkMarkPos_Mark1ArrayTable(data) \
    Data_Subdata(data, MarkMarkPos_Mark1ArrayOffset(data))
#define MarkMarkPos_Mark2ArrayTable(data) \
    Data_Subdata(data, MarkMarkPos_Mark2ArrayOffset(data))

#define Mark2Array_Mark2Count(data)                 Data_UInt16(data, 0)
#define Mark2Array_Mark2Record(data, index, classCount) \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define Mark2Record_Mark2AnchorOffset(data, index)  Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*******************************************ANCHOR TABLE*******************************************/

#define Anchor_Format(data)                         Data_UInt16(data, 0)

#define AnchorF1_XCoordinate(data)                  Data_Int16 (data, 2)
#define AnchorF1_YCoordinate(data)                  Data_Int16 (data, 4)

#define AnchorF2_XCoordinate(data)                  Data_Int16 (data, 2)
#define AnchorF2_YCoordinate(data)                  Data_Int16 (data, 4)
#define AnchorF2_AnchorPoint(data)                  Data_UInt16(data, 6)

#define AnchorF3_XCoordinate(data)                  Data_Int16 (data, 2)
#define AnchorF3_YCoordinate(data)                  Data_Int16 (data, 4)
#define AnchorF3_XDeviceOffset(data)                Data_UInt16(data, 6)
#define AnchorF3_YDeviceOffset(data)                Data_UInt16(data, 8)

/**************************************************************************************************/

/********************************************MARK ARRAY********************************************/

#define MarkArray_MarkCount(data)                   Data_UInt16(data, 0)
#define MarkArray_MarkRecord(data, index)           Data_Subdata(data, 2 + ((index) * 4))

#define MarkRecord_Class(data)                      Data_UInt16(data, 0)
#define MarkRecord_MarkAnchorOffset(data)           Data_UInt16(data, 2)

/**************************************************************************************************/

#endif
