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

#ifndef _SF_INTERNAL_GSUB_H
#define _SF_INTERNAL_GSUB_H

#include "SFData.h"

enum {
    LookupTypeSingle = 1,
    LookupTypeMultiple = 2,
    LookupTypeAlternate = 3,
    LookupTypeLigature = 4,
    LookupTypeContext = 5,
    LookupTypeChainingContext = 6,
    LookupTypeExtension = 7,
    LookupTypeReverseChainingContext = 8
};

/*******************************************GSUB HEADER********************************************/

#define GSUB_Version(data)                              Header_Version(data)
#define GSUB_ScriptListOffset(data)                     Header_ScriptListOffset(data)
#define GSUB_FeatureListOffset(data)                    Header_FeatureListOffset(data)
#define GSUB_LookupListOffset(data)                     Header_LookupListOffset(data)

/**************************************************************************************************/

/***********************************SINGLE SUBSTITUTION SUBTABLE***********************************/

#define SingleSubst_Format(data)                        Data_UInt16(data, 0)

#define SingleSubstF1_CoverageOffset(data)              Data_UInt16(data, 2)
#define SingleSubstF1_DeltaGlyphID(data)                Data_Int16 (data, 4)
#define SingleSubstF1_CoverageTable(data) \
    Data_Subdata(data, SingleSubstF1_CoverageOffset(data))

#define SingleSubstF2_CoverageOffset(data)              Data_UInt16(data, 2)
#define SingleSubstF2_GlyphCount(data)                  Data_UInt16(data, 4)
#define SingleSubstF2_Substitute(data, index)           Data_UInt16(data, 6 + ((index) * 2))
#define SingleSubstF2_CoverageTable(data) \
    Data_Subdata(data, SingleSubstF2_CoverageOffset(data))

/**************************************************************************************************/

/**********************************MULTIPLE SUBSTITUTION SUBTABLE**********************************/

#define MultipleSubst_Format(data)                      Data_UInt16(data, 0)
#define MultipleSubstF1_CoverageOffset(data)            Data_UInt16(data, 2)
#define MultipleSubstF1_SequenceCount(data)             Data_UInt16(data, 4)
#define MultipleSubstF1_SequenceOffset(data, index)     Data_UInt16(data, 6 + ((index) * 2))
#define MultipleSubstF1_CoverageTable(data) \
    Data_Subdata(data, MultipleSubstF1_CoverageOffset(data))
#define MultipleSubstF1_SequenceTable(data, index) \
    Data_Subdata(data, MultipleSubstF1_SequenceOffset(data, index))

#define Sequence_GlyphCount(data)                       Data_UInt16(data, 0)
#define Sequence_Substitute(data, index)                Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/*********************************ALTERNATE SUBSTITUTION SUBTABLE**********************************/

#define AlternateSubst_Format(data)                     Data_UInt16(data, 0)
#define AlternateSubstF1_CoverageOffset(data)           Data_UInt16(data, 2)
#define AlternateSubstF1_AlternateSetCount(data)        Data_UInt16(data, 4)
#define AlternateSubstF1_AlternateSetOffset(data, index) \
                                                        Data_UInt16(data, 6 + ((index) * 2))
#define AlternateSubstF1_CoverageTable(data) \
    Data_Subdata(data, AlternateSubstF1_CoverageOffset(data))
#define AlternateSubstF1_AlternateSetTable(data, index) \
    Data_Subdata(data, AlternateSubstF1_AlternateSetOffset(data, index))

#define AlternateSet_GlyphCount(data)                   Data_UInt16(data, 0)
#define AlternateSet_Alternate(data, index)             Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/**********************************LIGATURE SUBSTITUTION SUBTABLE**********************************/

#define LigatureSubst_Format(data)                      Data_UInt16(data, 0)
#define LigatureSubstF1_CoverageOffset(data)            Data_UInt16(data, 2)
#define LigatureSubstF1_LigSetCount(data)               Data_UInt16(data, 4)
#define LigatureSubstF1_LigatureSetOffset(data, index)  Data_UInt16(data, 6 + ((index) * 2))
#define LigatureSubstF1_CoverageTable(data) \
    Data_Subdata(data, LigatureSubstF1_CoverageOffset(data))
#define LigatureSubstF1_LigatureSetTable(data, index) \
    Data_Subdata(data, LigatureSubstF1_LigatureSetOffset(data, index))

#define LigatureSet_LigatureCount(data)                 Data_UInt16(data, 0)
#define LigatureSet_LigatureOffset(data, index)         Data_UInt16(data, 2 + ((index) * 2))
#define LigatureSet_LigatureTable(data, index) \
    Data_Subdata(data, LigatureSet_LigatureOffset(data, index))

#define Ligature_LigGlyph(data)                         Data_UInt16(data, 0)
#define Ligature_CompCount(data)                        Data_UInt16(data, 2)
#define Ligature_Component(data, index)                 Data_UInt16(data, 4 + ((index) * 2))

/**************************************************************************************************/

/*********************REVERSE CHAINING CONTEXTUAL SINGLE SUBSTITUTION SUBTABLE*********************/

#define ReverseChainSubst_Format(data)                  Data_UInt16(data, 0)

#define ReverseChainSubstF1_CoverageOffset(data)        Data_UInt16(data, 2)
#define ReverseChainSubstF1_RevBacktrackRecord(data)    Data_Subdata(data, 4)
#define ReverseChainSubstF1_CoverageTable(data) \
    Data_Subdata(data, ReverseChainSubstF1_CoverageOffset(data))

#define RevBacktrackRecord_GlyphCount(data)             Data_UInt16(data, 0)
#define RevBacktrackRecord_CoverageOffsets(data)        Data_Subdata(data, 2)
#define RevBacktrackRecord_RevLookaheadRecord(data, glyphCount) \
                                                        Data_Subdata(data, 2 + ((glyphCount) * 2))

#define RevLookaheadRecord_GlyphCount(data)             Data_UInt16(data, 0)
#define RevLookaheadRecord_CoverageOffsets(data)        Data_Subdata(data, 2)
#define RevLookaheadRecord_RevSubstRecord(data, glyphCount) \
                                                        Data_Subdata(data, 2 + ((glyphCount) * 2))

#define RevSubstRecord_GlyphCount(data)                 Data_UInt16(data, 0)
#define RevSubstRecord_Substitute(data, index)          Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

#endif
