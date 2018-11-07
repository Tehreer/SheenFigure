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

#define SFGlyphRange_Size()                             (6)
#define SFGlyphRange_Start(data)                        SFData_UInt16(data, 0)
#define SFGlyphRange_End(data)                          SFData_UInt16(data, 2)
#define SFGlyphRange_Value(data)                        SFData_UInt16(data, 4)

/**********************************************ARRAYS**********************************************/

#define SFUInt16Array_Value(data, index)                SFData_UInt16(data, (index) * 2)

#define SFGlyphArray_Value(data, index)                 SFUInt16Array_Value(data, index)

#define SFTagRecordArray_Value(data, index)             SFData_Subdata(data, (index) * SFTagRecord_Size())

/**************************************************************************************************/

#define SFHeader_Version(data)                          SFData_UInt32(data, 0)
#define SFHeader_ScriptListOffset(data)                 SFData_UInt16(data, 4)
#define SFHeader_FeatureListOffset(data)                SFData_UInt16(data, 6)
#define SFHeader_LookupListOffset(data)                 SFData_UInt16(data, 8)
#define SFHeader_ScriptListTable(data) \
    SFData_Subdata(data, SFHeader_ScriptListOffset(data))
#define SFHeader_FeatureListTable(data) \
    SFData_Subdata(data, SFHeader_FeatureListOffset(data))
#define SFHeader_LookupListTable(data) \
    SFData_Subdata(data, SFHeader_LookupListOffset(data))

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
#define SFLookupList_LookupTable(data, index) \
    SFData_Subdata(data, SFLookupList_LookupOffset(data, index))

/**************************************************************************************************/

/*******************************************LOOKUP TABLE*******************************************/

#define SFLookup_LookupType(data)                       SFData_UInt16(data, 0)
#define SFLookup_LookupFlag(data)                       SFData_UInt16(data, 2)
#define SFLookup_SubtableCount(data)                    SFData_UInt16(data, 4)
#define SFLookup_SubtableOffset(data, index)            SFData_UInt16(data, 6 + ((index) * 2))
#define SFLookup_MarkFilteringSet(data, subtableCount)  SFData_UInt16(data, 8 + ((subtableCount) * 2))
#define SFLookup_SubtableData(data, index) \
    SFData_Subdata(data, SFLookup_SubtableOffset(data, index))

/**************************************************************************************************/

/******************************************COVERAGE TABLE******************************************/

#define SFCoverage_Format(data)                         SFData_UInt16(data, 0)

#define SFCoverageF1_GlyphCount(data)                   SFData_UInt16(data, 2)
#define SFCoverageF1_GlyphArray(data)                   SFData_Subdata(data, 4)

#define SFCoverageF2_RangeCount(data)                   SFData_UInt16(data, 2)
#define SFCoverageF2_RangeRecord(data, index)           SFData_Subdata(data, 4 + ((index) * SFGlyphRange_Size()))
#define SFCoverageF2_GlyphRangeArray(data)              SFCoverageF2_RangeRecord(data, 0)

#define SFRangeRecord_StartGlyphID(data)                SFGlyphRange_Start(data)
#define SFRangeRecord_EndGlyphID(data)                  SFGlyphRange_End(data)
#define SFRangeRecord_StartCoverageIndex(data)          SFGlyphRange_Value(data)

/**************************************************************************************************/

/**************************************CLASS DEFINITION TABLE**************************************/

#define SFClassDef_Format(data)                         SFData_UInt16(data, 0)

#define SFClassDefF1_StartGlyphID(data)                 SFData_UInt16(data, 2)
#define SFClassDefF1_GlyphCount(data)                   SFData_UInt16(data, 4)
#define SFClassDefF1_ClassValueArray(data)              SFData_Subdata(data, 6)

#define SFClassDefF2_ClassRangeCount(data)              SFData_UInt16(data, 2)
#define SFClassDefF2_ClassRangeRecord(data, index)      SFData_Subdata(data, 4 + ((index) * SFGlyphRange_Size()))
#define SFClassDefF2_GlyphRangeArray(data)              SFClassDefF2_ClassRangeRecord(data, 0)

#define SFClassRangeRecord_Start(data)                  SFGlyphRange_Start(data)
#define SFClassRangeRecord_End(data)                    SFGlyphRange_End(data)
#define SFClassRangeRecord_Class(data)                  SFGlyphRange_Value(data)

/**************************************************************************************************/

/*******************************************DEVICE TABLE*******************************************/

#define SFDevice_StartSize(data)                        SFData_UInt16(data, 0)
#define SFDevice_EndSize(data)                          SFData_UInt16(data, 2)
#define SFDevice_DeltaFormat(data)                      SFData_UInt16(data, 4)
#define SFDevice_DeltaValue(data, index)                SFData_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/**************************************VARIATION INDEX TABLE***************************************/

#define SFVarIndex_DeltaSetOuterIndex(data)             SFData_UInt16(data, 0)
#define SFVarIndex_DeltaSetInnerIndex(data)             SFData_UInt16(data, 2)
#define SFVarIndex_DeltaFormat(data)                    SFData_UInt16(data, 4)

/**************************************************************************************************/

/*************************************FEATURE VARIATIONS TABLE*************************************/

#define SFFeatureVars_MajorVersion(data)                SFData_UInt16(data, 0)
#define SFFeatureVars_MinorVersion(data)                SFData_UInt16(data, 2)
#define SFFeatureVars_FeatureVarCount(data)             SFData_UInt32(data, 4)
#define SFFeatureVars_FeatureVarRecord(data, index)     SFData_Subdata(data, 8 + ((index) * 8))

#define SFFeatureVarRecord_ConditionSetOffset(data)     SFData_UInt32(data, 0)
#define SFFeatureVarRecord_FeatureSubstOffset(data)     SFData_UInt32(data, 4)

/**************************************************************************************************/

/***************************************CONDITION SET TABLE****************************************/

#define SFConditionSet_ConditionCount(data)             SFData_UInt16(data, 0)
#define SFConditionSet_ConditionOffset(data, index)     SFData_UInt32(data, 2 + ((index) * 4))
#define SFConditionSet_ConditionTable(data, index) \
    SFData_Subdata(data, SFConditionSet_ConditionOffset(data, index))

/**************************************************************************************************/

/*****************************************CONDITION TABLE******************************************/

#define SFCondition_Format(data)                        SFData_UInt16(data, 0)
#define SFCondition_AxisIndex(data)                     SFData_UInt16(data, 2)
#define SFCondition_FilterRangeMinValue(data)           SFData_Int16(data, 4)
#define SFCondition_FilterRangeMaxValue(data)           SFData_Int16(data, 6)

/**************************************************************************************************/

/*********************************FEATURE TABLE SUBSTITUTION TABLE*********************************/

#define SFFeatureSubst_MajorVersion(data)               SFData_UInt16(data, 0)
#define SFFeatureSubst_MinorVersion(data)               SFData_UInt16(data, 2)
#define SFFeatureSubst_SubstCount(data)                 SFData_UInt16(data, 4)
#define SFFeatureSubst_FeatureSubstRecord(data, index)  SFData_Subdata(data, 6 + ((index) * 6))

#define SFFeatureSubstRecord_FeatureIndex(data)         SFData_UInt16(data, 0)
#define SFFeatureSubstRecord_AltFeatureOffset(data)     SFData_UInt32(data, 2)

/**************************************************************************************************/

/******************************************LOOKUP RECORD*******************************************/

#define SFLookupRecord_SequenceIndex(data)              SFData_UInt16(data, 0)
#define SFLookupRecord_LookupListIndex(data)            SFData_UInt16(data, 2)

/**************************************************************************************************/

/***************************************CONTEXTUAL SUBTABLE****************************************/

#define SFContext_Format(data)                          SFData_UInt16(data, 0)

#define SFContextF1_CoverageOffset(data)                SFData_UInt16(data, 2)
#define SFContextF1_RuleSetCount(data)                  SFData_UInt16(data, 4)
#define SFContextF1_RuleSetOffset(data, index)          SFData_UInt16(data, 6 + ((index) * 2))
#define SFContextF1_CoverageTable(data) \
    SFData_Subdata(data, SFContextF1_CoverageOffset(data))
#define SFContextF1_RuleSetTable(data, index) \
    SFData_Subdata(data, SFContextF1_RuleSetOffset(data, index))

#define SFContextF2_CoverageOffset(data)                SFData_UInt16(data, 2)
#define SFContextF2_ClassDefOffset(data)                SFData_UInt16(data, 4)
#define SFContextF2_RuleSetCount(data)                  SFData_UInt16(data, 6)
#define SFContextF2_RuleSetOffset(data, index)          SFData_UInt16(data, 8 + ((index) * 2))
#define SFContextF2_CoverageTable(data) \
    SFData_Subdata(data, SFContextF2_CoverageOffset(data))
#define SFContextF2_ClassDefTable(data) \
    SFData_Subdata(data, SFContextF2_ClassDefOffset(data))
#define SFContextF2_RuleSetTable(data, index) \
    SFData_Subdata(data, SFContextF2_RuleSetOffset(data, index))

#define SFContextF3_Rule(data)                          SFData_Subdata(data, 2)

#define SFRuleSet_RuleCount(data)                       SFData_UInt16(data, 0)
#define SFRuleSet_RuleOffset(data, index)               SFData_UInt16(data, 2 + ((index) * 2))

#define SFRule_GlyphCount(data)                         SFData_UInt16(data, 0)
#define SFRule_LookupCount(data)                        SFData_UInt16(data, 2)
#define SFRule_ValueArray(data)                         SFData_Subdata(data, 4)
#define SFRule_LookupArray(data, glyphCount)            SFData_Subdata(data, 4 + ((glyphCount) * 2))

#define SFLookupArray_Value(data, index)                SFData_Subdata(data, (index) * 4)

/**************************************************************************************************/

/***********************************CHAINING CONTEXTUAL SUBTABLE***********************************/

#define SFChainContext_Format(data)                     SFData_UInt16(data, 0)

#define SFChainContextF1_CoverageOffset(data)           SFData_UInt16(data, 2)
#define SFChainContextF1_ChainRuleSetCount(data)        SFData_UInt16(data, 4)
#define SFChainContextF1_ChainRuleSetOffset(data, index) \
                                                        SFData_UInt16(data, 6 + ((index) * 2))
#define SFChainContextF1_CoverageTable(data) \
    SFData_Subdata(data, SFChainContextF1_CoverageOffset(data))
#define SFChainContextF1_ChainRuleSetTable(data, index) \
    SFData_Subdata(data, SFChainContextF1_ChainRuleSetOffset(data, index))

#define SFChainContextF2_CoverageOffset(data)           SFData_UInt16(data, 2)
#define SFChainContextF2_BacktrackClassDefOffset(data)  SFData_UInt16(data, 4)
#define SFChainContextF2_InputClassDefOffset(data)      SFData_UInt16(data, 6)
#define SFChainContextF2_LookaheadClassDefOffset(data)  SFData_UInt16(data, 8)
#define SFChainContextF2_ChainRuleSetCount(data)        SFData_UInt16(data, 10)
#define SFChainContextF2_ChainRuleSetOffset(data, index) \
                                                        SFData_UInt16(data, 12 + ((index) * 2))
#define SFChainContextF2_CoverageTable(data) \
    SFData_Subdata(data, SFChainContextF2_CoverageOffset(data))
#define SFChainContextF2_BacktrackClassDefTable(data) \
    SFData_Subdata(data, SFChainContextF2_BacktrackClassDefOffset(data))
#define SFChainContextF2_InputClassDefTable(data) \
    SFData_Subdata(data, SFChainContextF2_InputClassDefOffset(data))
#define SFChainContextF2_LookaheadClassDefTable(data) \
    SFData_Subdata(data, SFChainContextF2_LookaheadClassDefOffset(data))
#define SFChainContextF2_ChainRuleSetTable(data, index) \
    SFData_Subdata(data, SFChainContextF2_ChainRuleSetOffset(data, index))

#define SFChainContextF3_ChainRuleTable(data)           SFData_Subdata(data, 2)

#define SFChainRuleSet_ChainRuleCount(data)             SFData_UInt16(data, 0)
#define SFChainRuleSet_ChainRuleOffset(data, index)     SFData_UInt16(data, 2 + ((index) * 2))
#define SFChainRuleSet_ChainRuleTable(data, index) \
    SFData_Subdata(data, SFChainRuleSet_ChainRuleOffset(data, index))

#define SFChainRule_BacktrackRecord(data)               SFData_Subdata(data, 0)

#define SFBacktrackRecord_GlyphCount(data)              SFData_UInt16(data, 0)
#define SFBacktrackRecord_ValueArray(data)              SFData_Subdata(data, 2)
#define SFBacktrackRecord_InputRecord(data, glyphCount) SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFInputRecord_GlyphCount(data)                  SFData_UInt16(data, 0)
#define SFInputRecord_ValueArray(data)                  SFData_Subdata(data, 2)
#define SFInputRecord_LookaheadRecord(data, glyphCount) SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFLookaheadRecord_GlyphCount(data)              SFData_UInt16(data, 0)
#define SFLookaheadRecord_ValueArray(data)              SFData_Subdata(data, 2)
#define SFLookaheadRecord_ContextRecord(data, glyphCount) \
                                                        SFData_Subdata(data, 2 + ((glyphCount) * 2))

#define SFContextRecord_LookupCount(data)               SFData_UInt16(data, 0)
#define SFContextRecord_LookupArray(data)               SFData_Subdata(data, 2)

/**************************************************************************************************/

/****************************************EXTENSION SUBTABLE****************************************/

#define SFExtension_Format(data)                        SFData_UInt16(data, 0)

#define SFExtensionF1_LookupType(data)                  SFData_UInt16(data, 2)
#define SFExtensionF1_ExtensionOffset(data)             SFData_UInt32(data, 4)
#define SFExtensionF1_ExtensionData(data) \
    SFData_Subdata(data, SFExtensionF1_ExtensionOffset(data))

/**************************************************************************************************/

#endif
