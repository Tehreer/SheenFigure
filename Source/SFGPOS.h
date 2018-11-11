/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_GPOS_H
#define _SF_INTERNAL_GPOS_H

#include "SFData.h"

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

#define SFGPOS_Version(data)                        Data_UInt32(data, 0)
#define SFGPOS_ScriptListOffset(data)               Data_UInt16(data, 4)
#define SFGPOS_FeatureListOffset(data)              Data_UInt16(data, 6)
#define SFGPOS_LookupListOffset(data)               Data_UInt16(data, 8)

/**************************************************************************************************/

/*******************************************VALUE RECORD*******************************************/

#define SFValueFormat_XPlacement(format)            ((format) & 0x0001 ? 1 : 0)
#define SFValueFormat_YPlacement(format)            ((format) & 0x0002 ? 1 : 0)
#define SFValueFormat_XAdvance(format)              ((format) & 0x0004 ? 1 : 0)
#define SFValueFormat_YAdvance(format)              ((format) & 0x0008 ? 1 : 0)
#define SFValueFormat_XPlaDevice(format)            ((format) & 0x0010 ? 1 : 0)
#define SFValueFormat_YPlaDevice(format)            ((format) & 0x0020 ? 1 : 0)
#define SFValueFormat_XAdvDevice(format)            ((format) & 0x0040 ? 1 : 0)
#define SFValueFormat_YAdvDevice(format)            ((format) & 0x0080 ? 1 : 0)
#define SFValueFormat_Reserved(format)              ((format) & 0xFF00 ? 1 : 0)
#define SFValueFormat_ValueCount(format)            \
(                                                   \
   SFValueFormat_XPlacement(format)                 \
 + SFValueFormat_YPlacement(format)                 \
 + SFValueFormat_XAdvance(format)                   \
 + SFValueFormat_YAdvance(format)                   \
 + SFValueFormat_XPlaDevice(format)                 \
 + SFValueFormat_YPlaDevice(format)                 \
 + SFValueFormat_XAdvDevice(format)                 \
 + SFValueFormat_YAdvDevice(format)                 \
 + SFValueFormat_Reserved(format)                   \
)

#define SFValueRecord_Size(format)                  \
(                                                   \
   (format)                                         \
 ? (2 * SFValueFormat_ValueCount(format))           \
 : (0)                                              \
)

/**************************************************************************************************/

/******************************SINGLE ADJUSTMENT POSITIONING SUBTABLE******************************/

#define SFSinglePos_Format(data)                    Data_UInt16(data, 0)

#define SFSinglePosF1_CoverageOffset(data)          Data_UInt16(data, 2)
#define SFSinglePosF1_ValueFormat(data)             Data_UInt16(data, 4)
#define SFSinglePosF1_ValueRecord(data)             Data_Subdata(data, 6)
#define SFSinglePosF1_CoverageTable(data) \
    Data_Subdata(data, SFSinglePosF1_CoverageOffset(data))

#define SFSinglePosF2_CoverageOffset(data)          Data_UInt16(data, 2)
#define SFSinglePosF2_ValueFormat(data)             Data_UInt16(data, 4)
#define SFSinglePosF2_ValueCount(data)              Data_UInt16(data, 6)
#define SFSinglePosF2_ValueRecord(data, index, recordSize)  \
                                                    Data_Subdata(data, 8 + ((index) * (recordSize)))
#define SFSinglePosF2_CoverageTable(data) \
    Data_Subdata(data, SFSinglePosF2_CoverageOffset(data))

#define SFPairSet_PairValueCount(data)              Data_UInt16(data, 0)
#define SFPairSet_PairValueRecord(data, index, recordSize)  \
                                                    Data_Subdata(data, 2 + ((index) * (recordSize)))
#define SFPairSet_PairValueRecordArray(data)        SFPairSet_PairValueRecord(data, 0, 0)

#define SFPairValueRecord_Size(value1Size, value2Size)      \
                                                    (2 + (value1Size) + (value2Size))
#define SFPairValueRecord_SecondGlyph(data)         Data_UInt16(data, 0)
#define SFPairValueRecord_Value1(data)              Data_Subdata(data, 2)
#define SFPairValueRecord_Value2(data, value1Size)  Data_Subdata(data, 2 + (value1Size))

/**************************************************************************************************/

/*******************************PAIR ADJUSTMENT POSITIONING SUBTABLE*******************************/

#define SFPairPos_Format(data)                      Data_UInt16(data, 0)

#define SFPairPosF1_CoverageOffset(data)            Data_UInt16(data, 2)
#define SFPairPosF1_ValueFormat1(data)              Data_UInt16(data, 4)
#define SFPairPosF1_ValueFormat2(data)              Data_UInt16(data, 6)
#define SFPairPosF1_PairSetCount(data)              Data_UInt16(data, 8)
#define SFPairPosF1_PairSetOffset(data, index)      Data_UInt16(data, 10 + ((index) * 2))
#define SFPairPosF1_CoverageTable(data) \
    Data_Subdata(data, SFPairPosF1_CoverageOffset(data))
#define SFPairPosF1_PairSetTable(data, index) \
    Data_Subdata(data, SFPairPosF1_PairSetOffset(data, index))

#define SFPairPosF2_CoverageOffset(data)            Data_UInt16(data, 2)
#define SFPairPosF2_ValueFormat1(data)              Data_UInt16(data, 4)
#define SFPairPosF2_ValueFormat2(data)              Data_UInt16(data, 6)
#define SFPairPosF2_ClassDef1Offset(data)           Data_UInt16(data, 8)
#define SFPairPosF2_ClassDef2Offset(data)           Data_UInt16(data, 10)
#define SFPairPosF2_Class1Count(data)               Data_UInt16(data, 12)
#define SFPairPosF2_Class2Count(data)               Data_UInt16(data, 14)
#define SFPairPosF2_Class1Record(data, index, c1Size)   \
                                                    Data_Subdata(data, 16 + ((index) * (c1Size)))
#define SFPairPosF2_CoverageTable(data) \
    Data_Subdata(data, SFPairPosF2_CoverageOffset(data))
#define SFPairPosF2_ClassDef1Table(data) \
    Data_Subdata(data, SFPairPosF2_ClassDef1Offset(data))
#define SFPairPosF2_ClassDef2Table(data) \
    Data_Subdata(data, SFPairPosF2_ClassDef2Offset(data))

#define SFClass1Record_Size(c2Count, c2Size)        ((c2Count) * (c2Size))
#define SFClass1Record_Class2Record(data, index, c2Size)\
                                                    Data_Subdata(data, (index) * (c2Size))

#define SFClass2Record_Size(v1Size, v2Size)         ((v1Size) + (v2Size))
#define SFClass2Record_Value1(data)                 Data_Subdata(data, 0)
#define SFClass2Record_Value2(data, v1Size)         Data_Subdata(data, v1Size)

/**************************************************************************************************/

/*****************************CURSIVE ATTACHMENT POSITIONING SUBTABLE******************************/

#define SFCursivePos_Format(data)                   Data_UInt16(data, 0)
#define SFCursivePos_CoverageOffset(data)           Data_UInt16(data, 2)
#define SFCursivePos_EntryExitCount(data)           Data_UInt16(data, 4)
#define SFCursivePos_EntryExitRecord(data, index)   Data_Subdata(data, 6 + ((index) * 4))
#define SFCursivePos_CoverageTable(data) \
    Data_Subdata(data, SFCursivePos_CoverageOffset(data))

#define SFEntryExitRecord_EntryAnchorOffset(data)   Data_UInt16(data, 0)
#define SFEntryExitRecord_ExitAnchorOffset(data)    Data_UInt16(data, 2)

/**************************************************************************************************/

/***************************MARK TO BASE ATTACHMENT POSITIONING SUBTABLE***************************/

#define SFMarkBasePos_Format(data)                  Data_UInt16(data, 0)
#define SFMarkBasePos_MarkCoverageOffset(data)      Data_UInt16(data, 2)
#define SFMarkBasePos_BaseCoverageOffset(data)      Data_UInt16(data, 4)
#define SFMarkBasePos_ClassCount(data)              Data_UInt16(data, 6)
#define SFMarkBasePos_MarkArrayOffset(data)         Data_UInt16(data, 8)
#define SFMarkBasePos_BaseArrayOffset(data)         Data_UInt16(data, 10)
#define SFMarkBasePos_MarkCoverageTable(data) \
    Data_Subdata(data, SFMarkBasePos_MarkCoverageOffset(data))
#define SFMarkBasePos_BaseCoverageTable(data) \
    Data_Subdata(data, SFMarkBasePos_BaseCoverageOffset(data))
#define SFMarkBasePos_MarkArrayTable(data) \
    Data_Subdata(data, SFMarkBasePos_MarkArrayOffset(data))
#define SFMarkBasePos_BaseArrayTable(data) \
    Data_Subdata(data, SFMarkBasePos_BaseArrayOffset(data))

#define SFBaseArray_BaseCount(data)                 Data_UInt16(data, 0)
#define SFBaseArray_BaseRecord(data, index, classCount) \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFBaseArray_BaseAnchorOffset(data, index)   Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define SFMarkLigPos_Format(data)                   Data_UInt16(data, 0)
#define SFMarkLigPos_MarkCoverageOffset(data)       Data_UInt16(data, 2)
#define SFMarkLigPos_LigatureCoverageOffset(data)   Data_UInt16(data, 4)
#define SFMarkLigPos_ClassCount(data)               Data_UInt16(data, 6)
#define SFMarkLigPos_MarkArrayOffset(data)          Data_UInt16(data, 8)
#define SFMarkLigPos_LigatureArrayOffset(data)      Data_UInt16(data, 10)
#define SFMarkLigPos_MarkCoverageTable(data) \
    Data_Subdata(data, SFMarkLigPos_MarkCoverageOffset(data))
#define SFMarkLigPos_LigatureCoverageTable(data) \
    Data_Subdata(data, SFMarkLigPos_LigatureCoverageOffset(data))
#define SFMarkLigPos_MarkArrayTable(data) \
    Data_Subdata(data, SFMarkLigPos_MarkArrayOffset(data))
#define SFMarkLigPos_LigatureArrayTable(data) \
    Data_Subdata(data, SFMarkLigPos_LigatureArrayOffset(data))

#define SFLigatureArray_LigatureCount(data)         Data_UInt16(data, 0)
#define SFLigatureArray_LigatureAttachOffset(data, index)   \
                                                    Data_UInt16(data, 2 + ((index) * 2))
#define SFLigatureArray_LigatureAttachTable(data, index) \
    Data_Subdata(data, SFLigatureArray_LigatureAttachOffset(data, index))

#define SFLigatureAttach_ComponentCount(data)       Data_UInt16(data, 0)
#define SFLigatureAttach_ComponentRecord(data, index, classCount) \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFComponentRecord_LigatureAnchorOffset(data, index) \
                                                    Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define SFMarkMarkPos_Format(data)                  Data_UInt16(data, 0)
#define SFMarkMarkPos_Mark1CoverageOffset(data)     Data_UInt16(data, 2)
#define SFMarkMarkPos_Mark2CoverageOffset(data)     Data_UInt16(data, 4)
#define SFMarkMarkPos_ClassCount(data)              Data_UInt16(data, 6)
#define SFMarkMarkPos_Mark1ArrayOffset(data)        Data_UInt16(data, 8)
#define SFMarkMarkPos_Mark2ArrayOffset(data)        Data_UInt16(data, 10)
#define SFMarkMarkPos_Mark1CoverageTable(data) \
    Data_Subdata(data, SFMarkMarkPos_Mark1CoverageOffset(data))
#define SFMarkMarkPos_Mark2CoverageTable(data) \
    Data_Subdata(data, SFMarkMarkPos_Mark2CoverageOffset(data))
#define SFMarkMarkPos_Mark1ArrayTable(data) \
    Data_Subdata(data, SFMarkMarkPos_Mark1ArrayOffset(data))
#define SFMarkMarkPos_Mark2ArrayTable(data) \
    Data_Subdata(data, SFMarkMarkPos_Mark2ArrayOffset(data))

#define SFMark2Array_Mark2Count(data)               Data_UInt16(data, 0)
#define SFMark2Array_Mark2Record(data, index, classCount)   \
                                                    Data_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFMark2Record_Mark2AnchorOffset(data, index)        \
                                                    Data_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*******************************************ANCHOR TABLE*******************************************/

#define SFAnchor_Format(data)                       Data_UInt16(data, 0)

#define SFAnchorF1_XCoordinate(data)                Data_Int16 (data, 2)
#define SFAnchorF1_YCoordinate(data)                Data_Int16 (data, 4)

#define SFAnchorF2_XCoordinate(data)                Data_Int16 (data, 2)
#define SFAnchorF2_YCoordinate(data)                Data_Int16 (data, 4)
#define SFAnchorF2_AnchorPoint(data)                Data_UInt16(data, 6)

#define SFAnchorF3_XCoordinate(data)                Data_Int16 (data, 2)
#define SFAnchorF3_YCoordinate(data)                Data_Int16 (data, 4)
#define SFAnchorF3_XDeviceOffset(data)              Data_UInt16(data, 6)
#define SFAnchorF3_YDeviceOffset(data)              Data_UInt16(data, 8)

/**************************************************************************************************/

/********************************************MARK ARRAY********************************************/

#define SFMarkArray_MarkCount(data)                 Data_UInt16(data, 0)
#define SFMarkArray_MarkRecord(data, index)         Data_Subdata(data, 2 + ((index) * 4))

#define SFMarkRecord_Class(data)                    Data_UInt16(data, 0)
#define SFMarkRecord_MarkAnchorOffset(data)         Data_UInt16(data, 2)

/**************************************************************************************************/

#endif
