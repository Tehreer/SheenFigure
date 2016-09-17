/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_COMMON_H
#define _SF_INTERNAL_COMMON_H

#include "SFBase.h"
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

#define SFTagRecord_Size()                              (6)
#define SFTagRecord_Tag(data)                           SFData_UInt32(data, 0)
#define SFTagRecord_Offset(data)                        SFData_UInt16(data, 4)

#define _SFGlyphRange_Size()                            (6)
#define _SFGlyphRange_Start(data)                       SFData_UInt16(data, 0)
#define _SFGlyphRange_End(data)                         SFData_UInt16(data, 2)
#define _SFGlyphRange_Value(data)                       SFData_UInt16(data, 4)

/**********************************************ARRAYS**********************************************/

#define SFUInt16Array_Value(data, index)                SFData_UInt16(data, (index) * 2)

#define SFGlyphArray_Value(data, index)                 SFUInt16Array_Value(data, index)

#define SFTagRecordArray_Value(data, index)             SFData_Subdata(data, (index) * SFTagRecord_Size())

/**************************************************************************************************/

#define SFHeader_Version(data)                          SFData_UInt32(data, 0)
#define SFHeader_ScriptListOffset(data)                 SFData_UInt16(data, 4)
#define SFHeader_FeatureListOffset(data)                SFData_UInt16(data, 6)
#define SFHeader_LookupListOffset(data)                 SFData_UInt16(data, 8)

/****************************************SCRIPT LIST TABLE*****************************************/

#define SFScriptList_ScriptCount(data)                  SFData_UInt16(data, 0)
#define SFScriptList_ScriptRecord(data, index)          SFData_Subdata(data, 2 + ((index) * SFTagRecord_Size()))

#define SFScriptRecord_ScriptTag(data)                  SFTagRecord_Tag(data)
#define SFScriptRecord_ScriptOffset(data)               SFTagRecord_Offset(data)

/**************************************************************************************************/

/*******************************************SCRIPT TABLE*******************************************/

#define SFScript_DefaultLangSysOffset(data)             SFData_UInt16(data, 0)
#define SFScript_LangSysCount(data)                     SFData_UInt16(data, 2)
#define SFScript_LangSysRecord(data, index)             SFData_Subdata(data, 4 + ((index) * SFTagRecord_Size()))

#define SFLangSysRecord_LangSysTag(data)                SFTagRecord_Tag(data)
#define SFLangSysRecord_LangSysOffset(data)             SFTagRecord_Offset(data)

/**************************************************************************************************/

/**************************************LANGUAGE SYSTEM TABLE***************************************/

#define SFLangSys_LookupOrderOffset(data)               SFData_UInt16(data, 0)
#define SFLangSys_ReqFeatureIndex(data)                 SFData_UInt16(data, 2)
#define SFLangSys_FeatureCount(data)                    SFData_UInt16(data, 4)
#define SFLangSys_FeatureIndex(data, index)             SFData_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/****************************************FEATURE LIST TABLE****************************************/

#define SFFeatureList_FeatureCount(data)                SFData_UInt16(data, 0)
#define SFFeatureList_FeatureRecord(data, index)        SFData_Subdata(data, 2 + ((index) * SFTagRecord_Size()))

#define SFFeatureRecord_FeatureTag(data)                SFTagRecord_Tag(data)
#define SFFeatureRecord_FeatureOffset(data)             SFTagRecord_Offset(data)

/**************************************************************************************************/

/******************************************FEATURE TABLE*******************************************/

#define SFFeature_FeatureParamsOffset(data)             SFData_UInt16(data, 0)
#define SFFeature_LookupCount(data)                     SFData_UInt16(data, 2)
#define SFFeature_LookupListIndex(data, index)          SFData_UInt16(data, 4 + ((index) * 2))

/**************************************************************************************************/

/****************************************LOOKUP LIST TABLE*****************************************/

#define SFLookupList_LookupCount(data)                  SFData_UInt16(data, 0)
#define SFLookupList_LookupOffset(data, index)          SFData_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/*******************************************LOOKUP TABLE*******************************************/

#define SFLookup_LookupType(data)                       SFData_UInt16(data, 0)
#define SFLookup_LookupFlag(data)                       SFData_UInt16(data, 2)
#define SFLookup_SubtableCount(data)                    SFData_UInt16(data, 4)
#define SFLookup_SubtableOffset(data, index)            SFData_UInt16(data, 6 + ((index) * 2))
#define SFLookup_MarkFilteringSet(data, subtableCount)  SFData_UInt16(data, 8 + ((subtableCount) * 2))

/**************************************************************************************************/

/******************************************COVERAGE TABLE******************************************/

#define SFCoverage_Format(data)                         SFData_UInt16(data, 0)

#define SFCoverageF1_GlyphCount(data)                   SFData_UInt16(data, 2)
#define SFCoverageF1_GlyphArray(data)                   SFData_Subdata(data, 4)

#define SFCoverageF2_RangeCount(data)                   SFData_UInt16(data, 2)
#define SFCoverageF2_RangeRecord(data, index)           SFData_Subdata(data, 4 + ((index) * _SFGlyphRange_Size()))

#define SFRangeRecord_Start(data)                       _SFGlyphRange_Start(data)
#define SFRangeRecord_End(data)                         _SFGlyphRange_End(data)
#define SFRangeRecord_StartCoverageIndex(data)          _SFGlyphRange_Value(data)

/**************************************************************************************************/

/**************************************CLASS DEFINITION TABLE**************************************/

#define SFClassDef_Format(data)                         SFData_UInt16(data, 0)

#define SFClassDefF1_StartGlyph(data)                   SFData_UInt16(data, 2)
#define SFClassDefF1_GlyphCount(data)                   SFData_UInt16(data, 4)
#define SFClassDefF1_ClassValueArray(data)              SFData_Subdata(data, 6)

#define SFClassDefF2_ClassRangeCount(data)              SFData_UInt16(data, 2)
#define SFClassDefF2_ClassRangeRecord(data, index)      SFData_Subdata(data, 4 + ((index) * _SFGlyphRange_Size()))
#define SFClassDefF2_RangeRecordArray(data)             SFClassDefF2_ClassRangeRecord(data, 0)

#define SFClassRangeRecord_Start(data)                  _SFGlyphRange_Start(data)
#define SFClassRangeRecord_End(data)                    _SFGlyphRange_End(data)
#define SFClassRangeRecord_Class(data)                  _SFGlyphRange_Value(data)

/**************************************************************************************************/

/*******************************************DEVICE TABLE*******************************************/

#define SFDevice_StartSize(data)                        SFData_UInt16(data, 0)
#define SFDevice_EndSize(data)                          SFData_UInt16(data, 2)
#define SFDevice_DeltaFormat(data)                      SFData_UInt16(data, 4)
#define SFDevice_DeltaValue(data, index)                SFData_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/******************************************LOOKUP RECORD*******************************************/

#define SFLookupRecord_SequenceIndex(data)              SFData_UInt16(data, 0)
#define SFLookupRecord_LookupListIndex(data)            SFData_UInt16(data, 2)

/**************************************************************************************************/

/***********************************CHAINING CONTEXTUAL SUBTABLE***********************************/

#define SFChainContext_Format(data)                     SFData_UInt16(data, 0)

#define SFChainContextF3_BacktrackRecord(data)          SFData_Subdata(data, 2)

#define SFBacktrackRecord_GlyphCount(data)              SFData_UInt16(data, 0)
#define SFBacktrackRecord_Value(data, index)            SFData_UInt16(data, 2 + ((index)  * 2))
#define SFBacktrackRecord_InputRecord(data, glyphCount) SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFInputRecord_GlyphCount(data)                  SFData_UInt16(data, 0)
#define SFInputRecord_Value(data, index)                SFData_UInt16(data, 2 + ((index)  * 2))
#define SFInputRecord_LookaheadRecord(data, glyphCount) SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFLookaheadRecord_GlyphCount(data)              SFData_UInt16(data, 0)
#define SFLookaheadRecord_Value(data, index)            SFData_UInt16(data, 2 + ((index)  * 2))
#define SFLookaheadRecord_ContextRecord(data, glyphCount) \
                                                        SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFContextRecord_LookupCount(data)               SFData_UInt16(data, 0)
#define SFContextRecord_LookupRecord(data, index)       SFData_Subdata(data, 2 + ((index) * 4))

/**************************************************************************************************/

/****************************************EXTENSION SUBTABLE****************************************/

#define SFExtension_Format(data)                        SFData_UInt16(data, 0)

#define SFExtensionF1_LookupType(data)                  SFData_UInt16(data, 2)
#define SFExtensionF1_ExtensionOffset(data)             SFData_UInt32(data, 4)

/**************************************************************************************************/

#endif
