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

/*******************************************GPOS HEADER********************************************/

#define SFGPOS_Version(data)                        SFData_UInt32(data, 0)
#define SFGPOS_ScriptListOffset(data)               SFData_UInt16(data, 4)
#define SFGPOS_FeatureListOffset(data)              SFData_UInt16(data, 6)
#define SFGPOS_LookupListOffset(data)               SFData_UInt16(data, 8)

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

#define SFValueRecord_NextValue(data, offset)       SFData_UInt16(data, offset), (offset += 2)
#define SFValueRecord_Size(format)                  \
(                                                   \
   (format)                                         \
 ? (2 * SFValueFormat_ValueCount(format))           \
 : (0)                                              \
)

/**************************************************************************************************/

/******************************SINGLE ADJUSTMENT POSITIONING SUBTABLE******************************/

#define SFSinglePos_Format(data)                    SFData_UInt16(data, 0)

#define SFSinglePosF1_CoverageOffset(data)          SFData_UInt16(data, 2)
#define SFSinglePosF1_ValueFormat(data)             SFData_UInt16(data, 4)
#define SFSinglePosF1_ValueRecord(data)             SFData_Subdata(data, 6)
#define SFSinglePosF1_CoverageTable(data) \
    SFData_Subdata(data, SFSinglePosF1_CoverageOffset(data))

#define SFSinglePosF2_CoverageOffset(data)          SFData_UInt16(data, 2)
#define SFSinglePosF2_ValueFormat(data)             SFData_UInt16(data, 4)
#define SFSinglePosF2_ValueCount(data)              SFData_UInt16(data, 6)
#define SFSinglePosF2_ValueRecord(data, index, recordSize)  \
                                                    SFData_Subdata(data, 8 + ((index) * (recordSize)))
#define SFSinglePosF2_CoverageTable(data) \
    SFData_Subdata(data, SFSinglePosF2_CoverageOffset(data))

#define SFPairSet_PairValueCount(data)              SFData_UInt16(data, 0)
#define SFPairSet_PairValueRecord(data, index, recordSize)  \
                                                    SFData_Subdata(data, 2 + ((index) * (recordSize)))
#define SFPairSet_PairValueRecordArray(data)        SFPairSet_PairValueRecord(data, 0, 0)

#define SFPairValueRecord_Size(value1Size, value2Size)      \
                                                    (2 + (value1Size) + (value2Size))
#define SFPairValueRecord_SecondGlyph(data)         SFData_UInt16(data, 0)
#define SFPairValueRecord_Value1(data)              SFData_Subdata(data, 2)
#define SFPairValueRecord_Value2(data, value1Size)  SFData_Subdata(data, 2 + (value1Size))

/**************************************************************************************************/

/*******************************PAIR ADJUSTMENT POSITIONING SUBTABLE*******************************/

#define SFPairPos_Format(data)                      SFData_UInt16(data, 0)

#define SFPairPosF1_CoverageOffset(data)            SFData_UInt16(data, 2)
#define SFPairPosF1_ValueFormat1(data)              SFData_UInt16(data, 4)
#define SFPairPosF1_ValueFormat2(data)              SFData_UInt16(data, 6)
#define SFPairPosF1_PairSetCount(data)              SFData_UInt16(data, 8)
#define SFPairPosF1_PairSetOffset(data, index)      SFData_UInt16(data, 10 + ((index) * 2))
#define SFPairPosF1_CoverageTable(data) \
    SFData_Subdata(data, SFPairPosF1_CoverageOffset(data))
#define SFPairPosF1_PairSetTable(data, index) \
    SFData_Subdata(data, SFPairPosF1_PairSetOffset(data, index))

#define SFPairPosF2_CoverageOffset(data)            SFData_UInt16(data, 2)
#define SFPairPosF2_ValueFormat1(data)              SFData_UInt16(data, 4)
#define SFPairPosF2_ValueFormat2(data)              SFData_UInt16(data, 6)
#define SFPairPosF2_ClassDef1Offset(data)           SFData_UInt16(data, 8)
#define SFPairPosF2_ClassDef2Offset(data)           SFData_UInt16(data, 10)
#define SFPairPosF2_Class1Count(data)               SFData_UInt16(data, 12)
#define SFPairPosF2_Class2Count(data)               SFData_UInt16(data, 14)
#define SFPairPosF2_Class1Record(data, index, c1Size)   \
                                                    SFData_Subdata(data, 16 + ((index) * (c1Size)))
#define SFPairPosF2_CoverageTable(data) \
    SFData_Subdata(data, SFPairPosF2_CoverageOffset(data))
#define SFPairPosF2_ClassDef1Table(data) \
    SFData_Subdata(data, SFPairPosF2_ClassDef1Offset(data))
#define SFPairPosF2_ClassDef2Table(data) \
    SFData_Subdata(data, SFPairPosF2_ClassDef2Offset(data))

#define SFClass1Record_Size(c2Count, c2Size)        ((c2Count) * (c2Size))
#define SFClass1Record_Class2Record(data, index, c2Size)\
                                                    SFData_Subdata(data, (index) * (c2Size))

#define SFClass2Record_Value(v1Size, v2Size)        ((v1Size) + (v2Size))
#define SFClass2Record_Value1(data)                 SFData_Subdata(data, 0)
#define SFClass2Record_Value2(data, v1Size)         SFData_Subdata(data, v1Size)

/**************************************************************************************************/

/*****************************CURSIVE ATTACHMENT POSITIONING SUBTABLE******************************/

#define SFCursivePos_Format(data)                   SFData_UInt16(data, 0)
#define SFCursivePos_CoverageOffset(data)           SFData_UInt16(data, 2)
#define SFCursivePos_EntryExitCount(data)           SFData_UInt16(data, 4)
#define SFCursivePos_EntryExitRecord(data, index)   SFData_Subdata(data, 6 + ((index) * 4))
#define SFCursivePos_CoverageTable(data) \
    SFData_Subdata(data, SFCursivePos_CoverageOffset(data))

#define SFEntryExitRecord_EntryAnchorOffset(data)   SFData_UInt16(data, 0)
#define SFEntryExitRecord_ExitAnchorOffset(data)    SFData_UInt16(data, 2)

/**************************************************************************************************/

/***************************MARK TO BASE ATTACHMENT POSITIONING SUBTABLE***************************/

#define SFMarkBasePos_Format(data)                  SFData_UInt16(data, 0)
#define SFMarkBasePos_MarkCoverageOffset(data)      SFData_UInt16(data, 2)
#define SFMarkBasePos_BaseCoverageOffset(data)      SFData_UInt16(data, 4)
#define SFMarkBasePos_ClassCount(data)              SFData_UInt16(data, 6)
#define SFMarkBasePos_MarkArrayOffset(data)         SFData_UInt16(data, 8)
#define SFMarkBasePos_BaseArrayOffset(data)         SFData_UInt16(data, 10)
#define SFMarkBasePos_MarkCoverageTable(data) \
    SFData_Subdata(data, SFMarkBasePos_MarkCoverageOffset(data))
#define SFMarkBasePos_BaseCoverageTable(data) \
    SFData_Subdata(data, SFMarkBasePos_BaseCoverageOffset(data))
#define SFMarkBasePos_MarkArrayTable(data) \
    SFData_Subdata(data, SFMarkBasePos_MarkArrayOffset(data))
#define SFMarkBasePos_BaseArrayTable(data) \
    SFData_Subdata(data, SFMarkBasePos_BaseArrayOffset(data))

#define SFBaseArray_BaseCount(data)                 SFData_UInt16(data, 0)
#define SFBaseArray_BaseRecord(data, index, classCount) \
                                                    SFData_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFBaseArray_BaseAnchorOffset(data, index)   SFData_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define SFMarkLigPos_Format(data)                   SFData_UInt16(data, 0)
#define SFMarkLigPos_MarkCoverageOffset(data)       SFData_UInt16(data, 2)
#define SFMarkLigPos_LigatureCoverageOffset(data)   SFData_UInt16(data, 4)
#define SFMarkLigPos_ClassCount(data)               SFData_UInt16(data, 6)
#define SFMarkLigPos_MarkArrayOffset(data)          SFData_UInt16(data, 8)
#define SFMarkLigPos_LigatureArrayOffset(data)      SFData_UInt16(data, 10)
#define SFMarkLigPos_MarkCoverageTable(data) \
    SFData_Subdata(data, SFMarkLigPos_MarkCoverageOffset(data))
#define SFMarkLigPos_LigatureCoverageTable(data) \
    SFData_Subdata(data, SFMarkLigPos_LigatureCoverageOffset(data))
#define SFMarkLigPos_MarkArrayTable(data) \
    SFData_Subdata(data, SFMarkLigPos_MarkArrayOffset(data))
#define SFMarkLigPos_LigatureArrayTable(data) \
    SFData_Subdata(data, SFMarkLigPos_LigatureArrayOffset(data))

#define SFLigatureArray_LigatureCount(data)         SFData_UInt16(data, 0)
#define SFLigatureArray_LigatureAttachOffset(data, index)   \
                                                    SFData_UInt16(data, 2 + ((index) * 2))
#define SFLigatureArray_LigatureAttachTable(data, index) \
    SFData_Subdata(data, SFLigatureArray_LigatureAttachOffset(data, index))

#define SFLigatureAttach_ComponentCount(data)       SFData_UInt16(data, 0)
#define SFLigatureAttach_ComponentRecord(data, index, classCount) \
                                                    SFData_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFComponentRecord_LigatureAnchorOffset(data, index) \
                                                    SFData_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*************************MARK TO LIGATURE ATTACHMENT POSITIONING SUBTABLE*************************/

#define SFMarkMarkPos_Format(data)                  SFData_UInt16(data, 0)
#define SFMarkMarkPos_Mark1CoverageOffset(data)     SFData_UInt16(data, 2)
#define SFMarkMarkPos_Mark2CoverageOffset(data)     SFData_UInt16(data, 4)
#define SFMarkMarkPos_ClassCount(data)              SFData_UInt16(data, 6)
#define SFMarkMarkPos_Mark1ArrayOffset(data)        SFData_UInt16(data, 8)
#define SFMarkMarkPos_Mark2ArrayOffset(data)        SFData_UInt16(data, 10)
#define SFMarkMarkPos_Mark1CoverageTable(data) \
    SFData_Subdata(data, SFMarkMarkPos_Mark1CoverageOffset(data))
#define SFMarkMarkPos_Mark2CoverageTable(data) \
    SFData_Subdata(data, SFMarkMarkPos_Mark2CoverageOffset(data))
#define SFMarkMarkPos_Mark1ArrayTable(data) \
    SFData_Subdata(data, SFMarkMarkPos_Mark1ArrayOffset(data))
#define SFMarkMarkPos_Mark2ArrayTable(data) \
    SFData_Subdata(data, SFMarkMarkPos_Mark2ArrayOffset(data))

#define SFMark2Array_Mark2Count(data)               SFData_UInt16(data, 0)
#define SFMark2Array_Mark2Record(data, index, classCount)   \
                                                    SFData_Subdata(data, 2 + ((index) * (2 * (classCount))))

#define SFMark2Record_Mark2AnchorOffset(data, index)        \
                                                    SFData_UInt16(data, 0 + ((index) * 2))

/**************************************************************************************************/

/*******************************************ANCHOR TABLE*******************************************/

#define SFAnchor_Format(data)                       SFData_UInt16(data, 0)

#define SFAnchorF1_XCoordinate(data)                SFData_Int16 (data, 2)
#define SFAnchorF1_YCoordinate(data)                SFData_Int16 (data, 4)

#define SFAnchorF2_XCoordinate(data)                SFData_Int16 (data, 2)
#define SFAnchorF2_YCoordinate(data)                SFData_Int16 (data, 4)
#define SFAnchorF2_AnchorPoint(data)                SFData_UInt16(data, 6)

#define SFAnchorF3_XCoordinate(data)                SFData_Int16 (data, 2)
#define SFAnchorF3_YCoordinate(data)                SFData_Int16 (data, 4)
#define SFAnchorF3_XDeviceOffset(data)              SFData_UInt16(data, 6)
#define SFAnchorF3_YDeviceOffset(data)              SFData_UInt16(data, 8)

/**************************************************************************************************/

/********************************************MARK ARRAY********************************************/

#define SFMarkArray_MarkCount(data)                 SFData_UInt16(data, 0)
#define SFMarkArray_MarkRecord(data, index)         SFData_Subdata(data, 2 + ((index) * 4))

#define SFMarkRecord_Class(data)                    SFData_UInt16(data, 0)
#define SFMarkRecord_MarkAnchorOffset(data)         SFData_UInt16(data, 2)

/**************************************************************************************************/

#endif
