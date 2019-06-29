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

#ifndef _SF_TABLES_OPEN_TYPE_H
#define _SF_TABLES_OPEN_TYPE_H

#include <SFConfig.h>

#include "SFBase.h"
#include "Data.h"

typedef SFUInt16 LookupType;

enum {
    LookupFlagRightToLeft = 0x0001,
    LookupFlagIgnoreBaseGlyphs = 0x0002,
    LookupFlagIgnoreLigatures = 0x0004,
    LookupFlagIgnoreMarks = 0x0008,
    LookupFlagUseMarkFilteringSet = 0x0010,
    LookupFlagReserved = 0x00E0,
    LookupFlagMarkAttachmentType = 0xFF00
};
typedef SFUInt16 LookupFlag;

#define TagRecord_Size()                                (6)
#define TagRecord_Tag(data)                             Data_UInt32(data, 0)
#define TagRecord_Offset(data)                          Data_UInt16(data, 4)

#define GlyphRange_Size()                               (6)
#define GlyphRange_Start(data)                          Data_UInt16(data, 0)
#define GlyphRange_End(data)                            Data_UInt16(data, 2)
#define GlyphRange_Value(data)                          Data_UInt16(data, 4)

/**********************************************ARRAYS**********************************************/

#define UInt16Array_Value(data, index)                  Data_UInt16(data, (index) * 2)

#define GlyphArray_Value(data, index)                   UInt16Array_Value(data, index)

#define TagRecordArray_Value(data, index)               Data_Subdata(data, (index) * TagRecord_Size())

/**************************************************************************************************/

#define Header_Version(data)                            Data_UInt32(data, 0)
#define Header_ScriptListOffset(data)                   Data_UInt16(data, 4)
#define Header_FeatureListOffset(data)                  Data_UInt16(data, 6)
#define Header_LookupListOffset(data)                   Data_UInt16(data, 8)
#define Header_ScriptListTable(data) \
    Data_Subdata(data, Header_ScriptListOffset(data))
#define Header_FeatureListTable(data) \
    Data_Subdata(data, Header_FeatureListOffset(data))
#define Header_LookupListTable(data) \
    Data_Subdata(data, Header_LookupListOffset(data))

#define HeaderV11_FeatureVariationsOffset(data)         Data_UInt32(data, 10)
#define HeaderV11_FeatureVariationsTable(data) \
    Data_Subdata(data, HeaderV11_FeatureVariationsOffset(data))

/****************************************SCRIPT LIST TABLE*****************************************/

#define ScriptList_ScriptCount(data)                    Data_UInt16(data, 0)
#define ScriptList_ScriptRecord(data, index)            Data_Subdata(data, 2 + ((index) * TagRecord_Size()))

#define ScriptRecord_ScriptTag(data)                    TagRecord_Tag(data)
#define ScriptRecord_ScriptOffset(data)                 TagRecord_Offset(data)

/**************************************************************************************************/

/*******************************************SCRIPT TABLE*******************************************/

#define Script_DefaultLangSysOffset(data)               Data_UInt16(data, 0)
#define Script_LangSysCount(data)                       Data_UInt16(data, 2)
#define Script_LangSysRecord(data, index)               Data_Subdata(data, 4 + ((index) * TagRecord_Size()))

#define LangSysRecord_LangSysTag(data)                  TagRecord_Tag(data)
#define LangSysRecord_LangSysOffset(data)               TagRecord_Offset(data)

/**************************************************************************************************/

/**************************************LANGUAGE SYSTEM TABLE***************************************/

#define LangSys_LookupOrderOffset(data)                 Data_UInt16(data, 0)
#define LangSys_ReqFeatureIndex(data)                   Data_UInt16(data, 2)
#define LangSys_FeatureCount(data)                      Data_UInt16(data, 4)
#define LangSys_FeatureIndex(data, index)               Data_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/****************************************FEATURE LIST TABLE****************************************/

#define FeatureList_FeatureCount(data)                  Data_UInt16(data, 0)
#define FeatureList_FeatureRecord(data, index)          Data_Subdata(data, 2 + ((index) * TagRecord_Size()))

#define FeatureRecord_FeatureTag(data)                  TagRecord_Tag(data)
#define FeatureRecord_FeatureOffset(data)               TagRecord_Offset(data)

/**************************************************************************************************/

/******************************************FEATURE TABLE*******************************************/

#define Feature_FeatureParamsOffset(data)               Data_UInt16(data, 0)
#define Feature_LookupCount(data)                       Data_UInt16(data, 2)
#define Feature_LookupListIndex(data, index)            Data_UInt16(data, 4 + ((index) * 2))

/**************************************************************************************************/

/****************************************LOOKUP LIST TABLE*****************************************/

#define LookupList_LookupCount(data)                    Data_UInt16(data, 0)
#define LookupList_LookupOffset(data, index)            Data_UInt16(data, 2 + ((index) * 2))
#define LookupList_LookupTable(data, index) \
    Data_Subdata(data, LookupList_LookupOffset(data, index))

/**************************************************************************************************/

/*******************************************LOOKUP TABLE*******************************************/

#define Lookup_LookupType(data)                         Data_UInt16(data, 0)
#define Lookup_LookupFlag(data)                         Data_UInt16(data, 2)
#define Lookup_SubtableCount(data)                      Data_UInt16(data, 4)
#define Lookup_SubtableOffset(data, index)              Data_UInt16(data, 6 + ((index) * 2))
#define Lookup_MarkFilteringSet(data, subtableCount)    Data_UInt16(data, 8 + ((subtableCount) * 2))
#define Lookup_SubtableData(data, index) \
    Data_Subdata(data, Lookup_SubtableOffset(data, index))

/**************************************************************************************************/

/******************************************COVERAGE TABLE******************************************/

#define Coverage_Format(data)                           Data_UInt16(data, 0)

#define CoverageF1_GlyphCount(data)                     Data_UInt16(data, 2)
#define CoverageF1_GlyphArray(data)                     Data_Subdata(data, 4)

#define CoverageF2_RangeCount(data)                     Data_UInt16(data, 2)
#define CoverageF2_RangeRecord(data, index)             Data_Subdata(data, 4 + ((index) * GlyphRange_Size()))
#define CoverageF2_GlyphRangeArray(data)                CoverageF2_RangeRecord(data, 0)

#define RangeRecord_StartGlyphID(data)                  GlyphRange_Start(data)
#define RangeRecord_EndGlyphID(data)                    GlyphRange_End(data)
#define RangeRecord_StartCoverageIndex(data)            GlyphRange_Value(data)

/**************************************************************************************************/

/**************************************CLASS DEFINITION TABLE**************************************/

#define ClassDef_Format(data)                           Data_UInt16(data, 0)

#define ClassDefF1_StartGlyphID(data)                   Data_UInt16(data, 2)
#define ClassDefF1_GlyphCount(data)                     Data_UInt16(data, 4)
#define ClassDefF1_ClassValueArray(data)                Data_Subdata(data, 6)

#define ClassDefF2_ClassRangeCount(data)                Data_UInt16(data, 2)
#define ClassDefF2_ClassRangeRecord(data, index)        Data_Subdata(data, 4 + ((index) * GlyphRange_Size()))
#define ClassDefF2_GlyphRangeArray(data)                ClassDefF2_ClassRangeRecord(data, 0)

#define ClassRangeRecord_Start(data)                    GlyphRange_Start(data)
#define ClassRangeRecord_End(data)                      GlyphRange_End(data)
#define ClassRangeRecord_Class(data)                    GlyphRange_Value(data)

/**************************************************************************************************/

/*******************************************DEVICE TABLE*******************************************/

#define Device_StartSize(data)                          Data_UInt16(data, 0)
#define Device_EndSize(data)                            Data_UInt16(data, 2)
#define Device_DeltaFormat(data)                        Data_UInt16(data, 4)
#define Device_DeltaValue(data, index)                  Data_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/**************************************VARIATION INDEX TABLE***************************************/

#define VarIndex_DeltaSetOuterIndex(data)               Data_UInt16(data, 0)
#define VarIndex_DeltaSetInnerIndex(data)               Data_UInt16(data, 2)
#define VarIndex_DeltaFormat(data)                      Data_UInt16(data, 4)

/**************************************************************************************************/

/*************************************FEATURE VARIATIONS TABLE*************************************/

#define FeatureVars_MajorVersion(data)                  Data_UInt16(data, 0)
#define FeatureVars_MinorVersion(data)                  Data_UInt16(data, 2)
#define FeatureVars_FeatureVarCount(data)               Data_UInt32(data, 4)
#define FeatureVars_FeatureVarRecord(data, index)       Data_Subdata(data, 8 + ((index) * 8))

#define FeatureVarRecord_ConditionSetOffset(data)       Data_UInt32(data, 0)
#define FeatureVarRecord_FeatureSubstOffset(data)       Data_UInt32(data, 4)

/**************************************************************************************************/

/***************************************CONDITION SET TABLE****************************************/

#define ConditionSet_ConditionCount(data)               Data_UInt16(data, 0)
#define ConditionSet_ConditionOffset(data, index)       Data_UInt32(data, 2 + ((index) * 4))
#define ConditionSet_ConditionTable(data, index) \
    Data_Subdata(data, ConditionSet_ConditionOffset(data, index))

/**************************************************************************************************/

/*****************************************CONDITION TABLE******************************************/

#define Condition_Format(data)                          Data_UInt16(data, 0)
#define Condition_AxisIndex(data)                       Data_UInt16(data, 2)
#define Condition_FilterRangeMinValue(data)             Data_Int16(data, 4)
#define Condition_FilterRangeMaxValue(data)             Data_Int16(data, 6)

/**************************************************************************************************/

/*********************************FEATURE TABLE SUBSTITUTION TABLE*********************************/

#define FeatureSubst_MajorVersion(data)                 Data_UInt16(data, 0)
#define FeatureSubst_MinorVersion(data)                 Data_UInt16(data, 2)
#define FeatureSubst_SubstCount(data)                   Data_UInt16(data, 4)
#define FeatureSubst_FeatureSubstRecord(data, index)    Data_Subdata(data, 6 + ((index) * 6))

#define FeatureSubstRecord_FeatureIndex(data)           Data_UInt16(data, 0)
#define FeatureSubstRecord_AltFeatureOffset(data)       Data_UInt32(data, 2)

/**************************************************************************************************/

/******************************************LOOKUP RECORD*******************************************/

#define LookupRecord_SequenceIndex(data)                Data_UInt16(data, 0)
#define LookupRecord_LookupListIndex(data)              Data_UInt16(data, 2)

/**************************************************************************************************/

/***************************************CONTEXTUAL SUBTABLE****************************************/

#define Context_Format(data)                            Data_UInt16(data, 0)

#define ContextF1_CoverageOffset(data)                  Data_UInt16(data, 2)
#define ContextF1_RuleSetCount(data)                    Data_UInt16(data, 4)
#define ContextF1_RuleSetOffset(data, index)            Data_UInt16(data, 6 + ((index) * 2))
#define ContextF1_CoverageTable(data) \
    Data_Subdata(data, ContextF1_CoverageOffset(data))
#define ContextF1_RuleSetTable(data, index) \
    Data_Subdata(data, ContextF1_RuleSetOffset(data, index))

#define ContextF2_CoverageOffset(data)                  Data_UInt16(data, 2)
#define ContextF2_ClassDefOffset(data)                  Data_UInt16(data, 4)
#define ContextF2_RuleSetCount(data)                    Data_UInt16(data, 6)
#define ContextF2_RuleSetOffset(data, index)            Data_UInt16(data, 8 + ((index) * 2))
#define ContextF2_CoverageTable(data) \
    Data_Subdata(data, ContextF2_CoverageOffset(data))
#define ContextF2_ClassDefTable(data) \
    Data_Subdata(data, ContextF2_ClassDefOffset(data))
#define ContextF2_RuleSetTable(data, index) \
    Data_Subdata(data, ContextF2_RuleSetOffset(data, index))

#define ContextF3_Rule(data)                            Data_Subdata(data, 2)

#define RuleSet_RuleCount(data)                         Data_UInt16(data, 0)
#define RuleSet_RuleOffset(data, index)                 Data_UInt16(data, 2 + ((index) * 2))

#define Rule_GlyphCount(data)                           Data_UInt16(data, 0)
#define Rule_LookupCount(data)                          Data_UInt16(data, 2)
#define Rule_ValueArray(data)                           Data_Subdata(data, 4)
#define Rule_LookupArray(data, glyphCount)              Data_Subdata(data, 4 + ((glyphCount) * 2))

#define LookupArray_Value(data, index)                  Data_Subdata(data, (index) * 4)

/**************************************************************************************************/

/***********************************CHAINING CONTEXTUAL SUBTABLE***********************************/

#define ChainContext_Format(data)                       Data_UInt16(data, 0)

#define ChainContextF1_CoverageOffset(data)             Data_UInt16(data, 2)
#define ChainContextF1_ChainRuleSetCount(data)          Data_UInt16(data, 4)
#define ChainContextF1_ChainRuleSetOffset(data, index)  Data_UInt16(data, 6 + ((index) * 2))
#define ChainContextF1_CoverageTable(data) \
    Data_Subdata(data, ChainContextF1_CoverageOffset(data))
#define ChainContextF1_ChainRuleSetTable(data, index) \
    Data_Subdata(data, ChainContextF1_ChainRuleSetOffset(data, index))

#define ChainContextF2_CoverageOffset(data)             Data_UInt16(data, 2)
#define ChainContextF2_BacktrackClassDefOffset(data)    Data_UInt16(data, 4)
#define ChainContextF2_InputClassDefOffset(data)        Data_UInt16(data, 6)
#define ChainContextF2_LookaheadClassDefOffset(data)    Data_UInt16(data, 8)
#define ChainContextF2_ChainRuleSetCount(data)          Data_UInt16(data, 10)
#define ChainContextF2_ChainRuleSetOffset(data, index)  Data_UInt16(data, 12 + ((index) * 2))
#define ChainContextF2_CoverageTable(data) \
    Data_Subdata(data, ChainContextF2_CoverageOffset(data))
#define ChainContextF2_BacktrackClassDefTable(data) \
    Data_Subdata(data, ChainContextF2_BacktrackClassDefOffset(data))
#define ChainContextF2_InputClassDefTable(data) \
    Data_Subdata(data, ChainContextF2_InputClassDefOffset(data))
#define ChainContextF2_LookaheadClassDefTable(data) \
    Data_Subdata(data, ChainContextF2_LookaheadClassDefOffset(data))
#define ChainContextF2_ChainRuleSetTable(data, index) \
    Data_Subdata(data, ChainContextF2_ChainRuleSetOffset(data, index))

#define ChainContextF3_ChainRuleTable(data)             Data_Subdata(data, 2)

#define ChainRuleSet_ChainRuleCount(data)               Data_UInt16(data, 0)
#define ChainRuleSet_ChainRuleOffset(data, index)       Data_UInt16(data, 2 + ((index) * 2))
#define ChainRuleSet_ChainRuleTable(data, index) \
    Data_Subdata(data, ChainRuleSet_ChainRuleOffset(data, index))

#define ChainRule_BacktrackRecord(data)                 Data_Subdata(data, 0)

#define BacktrackRecord_GlyphCount(data)                Data_UInt16(data, 0)
#define BacktrackRecord_ValueArray(data)                Data_Subdata(data, 2)
#define BacktrackRecord_InputRecord(data, glyphCount)   Data_Subdata(data, 2 + ((glyphCount) * 2))

#define InputRecord_GlyphCount(data)                    Data_UInt16(data, 0)
#define InputRecord_ValueArray(data)                    Data_Subdata(data, 2)
#define InputRecord_LookaheadRecord(data, glyphCount)   Data_Subdata(data, 2 + ((glyphCount) * 2))

#define LookaheadRecord_GlyphCount(data)                Data_UInt16(data, 0)
#define LookaheadRecord_ValueArray(data)                Data_Subdata(data, 2)
#define LookaheadRecord_ContextRecord(data, glyphCount) Data_Subdata(data, 2 + ((glyphCount) * 2))

#define ContextRecord_LookupCount(data)                 Data_UInt16(data, 0)
#define ContextRecord_LookupArray(data)                 Data_Subdata(data, 2)

/**************************************************************************************************/

/****************************************EXTENSION SUBTABLE****************************************/

#define Extension_Format(data)                          Data_UInt16(data, 0)

#define ExtensionF1_LookupType(data)                    Data_UInt16(data, 2)
#define ExtensionF1_ExtensionOffset(data)               Data_UInt32(data, 4)
#define ExtensionF1_ExtensionData(data) \
    Data_Subdata(data, ExtensionF1_ExtensionOffset(data))

/**************************************************************************************************/

SF_INTERNAL Data SearchScriptTable(Data scriptListTable, SFTag scriptTag);
SF_INTERNAL Data SearchLangSysTable(Data scriptTable, SFTag languageTag);
SF_INTERNAL Data SearchDefaultFeatureTable(Data langSysTable,
    Data featureListTable, SFTag featureTag, SFUInt16 *featureIndex);

SF_INTERNAL SFUInteger SearchCoverageIndex(Data coverageTable, SFGlyphID glyphID);
SF_INTERNAL SFUInt16 SearchGlyphClass(Data classDefTable, SFGlyphID glyphID);

SF_INTERNAL SFInt32 GetDevicePixels(Data deviceTable, SFUInt16 ppemSize);
SF_INTERNAL SFInt32 GetVariationPixels(Data varIndexTable, Data varStoreTable,
    const SFInt16 *coordArray, SFUInteger coordCount);

SF_INTERNAL SFInt32 GetRelevantDeltaPixels(Data devOrVarIdxTable, SFUInt16 ppemSize,
    Data varStoreTable, const SFInt16 *coordArray, SFUInteger coordCount);

SF_INTERNAL Data SearchFeatureSubstitutionTable(Data featureVarsTable,
    const SFInt16 *coordArray, SFUInteger coordCount);
SF_INTERNAL Data SearchAlternateFeatureTable(Data featureSubstTable, SFUInt16 featureIndex);

SF_INTERNAL Data SearchRelevantFeatureTable(Data langSysTable,
    Data featureListTable, Data featureSubstTable, SFTag featureTag);

#endif
