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
    SFLookupTypeSingle = 1,
    SFLookupTypeMultiple = 2,
    SFLookupTypeAlternate = 3,
    SFLookupTypeLigature = 4,
    SFLookupTypeContext = 5,
    SFLookupTypeChainingContext = 6,
    SFLookupTypeExtension = 7,
    SFLookupTypeReverseChainingContext = 8
};

/*******************************************GSUB HEADER********************************************/

#define SFGSUB_Version(data)                            SFHeader_Version(data)
#define SFGSUB_ScriptListOffset(data)                   SFHeader_ScriptListOffset(data)
#define SFGSUB_FeatureListOffset(data)                  SFHeader_FeatureListOffset(data)
#define SFGSUB_LookupListOffset(data)                   SFHeader_LookupListOffset(data)

/**************************************************************************************************/

/***********************************SINGLE SUBSTITUTION SUBTABLE***********************************/

#define SFSingleSubst_Format(data)                      Data_UInt16(data, 0)

#define SFSingleSubstF1_CoverageOffset(data)            Data_UInt16(data, 2)
#define SFSingleSubstF1_DeltaGlyphID(data)              Data_Int16 (data, 4)
#define SFSingleSubstF1_CoverageTable(data) \
    Data_Subdata(data, SFSingleSubstF1_CoverageOffset(data))

#define SFSingleSubstF2_CoverageOffset(data)            Data_UInt16(data, 2)
#define SFSingleSubstF2_GlyphCount(data)                Data_UInt16(data, 4)
#define SFSingleSubstF2_Substitute(data, index)         Data_UInt16(data, 6 + ((index) * 2))
#define SFSingleSubstF2_CoverageTable(data) \
    Data_Subdata(data, SFSingleSubstF2_CoverageOffset(data))

/**************************************************************************************************/

/**********************************MULTIPLE SUBSTITUTION SUBTABLE**********************************/

#define SFMultipleSubst_Format(data)                    Data_UInt16(data, 0)
#define SFMultipleSubstF1_CoverageOffset(data)          Data_UInt16(data, 2)
#define SFMultipleSubstF1_SequenceCount(data)           Data_UInt16(data, 4)
#define SFMultipleSubstF1_SequenceOffset(data, index)   Data_UInt16(data, 6 + ((index) * 2))
#define SFMultipleSubstF1_CoverageTable(data) \
    Data_Subdata(data, SFMultipleSubstF1_CoverageOffset(data))
#define SFMultipleSubstF1_SequenceTable(data, index) \
    Data_Subdata(data, SFMultipleSubstF1_SequenceOffset(data, index))

#define SFSequence_GlyphCount(data)                     Data_UInt16(data, 0)
#define SFSequence_Substitute(data, index)              Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/*********************************ALTERNATE SUBSTITUTION SUBTABLE**********************************/

#define SFAlternateSubst_Format(data)                   Data_UInt16(data, 0)
#define SFAlternateSubstF1_CoverageOffset(data)         Data_UInt16(data, 2)
#define SFAlternateSubstF1_AlternateSetCount(data)      Data_UInt16(data, 4)
#define SFAlternateSubstF1_AlternateSetOffset(data, index) \
                                                        Data_UInt16(data, 6 + ((index) * 2))
#define SFAlternateSubstF1_CoverageTable(data) \
    Data_Subdata(data, SFAlternateSubstF1_CoverageOffset(data))
#define SFAlternateSubstF1_AlternateSetTable(data, index) \
    Data_Subdata(data, SFAlternateSubstF1_AlternateSetOffset(data, index))

#define SFAlternateSet_GlyphCount(data)                 Data_UInt16(data, 0)
#define SFAlternateSet_Alternate(data, index)           Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/**********************************LIGATURE SUBSTITUTION SUBTABLE**********************************/

#define SFLigatureSubst_Format(data)                    Data_UInt16(data, 0)
#define SFLigatureSubstF1_CoverageOffset(data)          Data_UInt16(data, 2)
#define SFLigatureSubstF1_LigSetCount(data)             Data_UInt16(data, 4)
#define SFLigatureSubstF1_LigatureSetOffset(data, index) \
                                                        Data_UInt16(data, 6 + ((index) * 2))
#define SFLigatureSubstF1_CoverageTable(data) \
    Data_Subdata(data, SFLigatureSubstF1_CoverageOffset(data))
#define SFLigatureSubstF1_LigatureSetTable(data, index) \
    Data_Subdata(data, SFLigatureSubstF1_LigatureSetOffset(data, index))

#define SFLigatureSet_LigatureCount(data)               Data_UInt16(data, 0)
#define SFLigatureSet_LigatureOffset(data, index)       Data_UInt16(data, 2 + ((index) * 2))
#define SFLigatureSet_LigatureTable(data, index) \
    Data_Subdata(data, SFLigatureSet_LigatureOffset(data, index))

#define SFLigature_LigGlyph(data)                       Data_UInt16(data, 0)
#define SFLigature_CompCount(data)                      Data_UInt16(data, 2)
#define SFLigature_Component(data, index)               Data_UInt16(data, 4 + ((index) * 2))

/**************************************************************************************************/

/*********************REVERSE CHAINING CONTEXTUAL SINGLE SUBSTITUTION SUBTABLE*********************/

#define SFReverseChainSubst_Format(data)                Data_UInt16(data, 0)

#define SFReverseChainSubstF1_CoverageOffset(data)      Data_UInt16(data, 2)
#define SFReverseChainSubstF1_RevBacktrackRecord(data)  Data_Subdata(data, 4)
#define SFReverseChainSubstF1_CoverageTable(data) \
    Data_Subdata(data, SFReverseChainSubstF1_CoverageOffset(data))

#define SFRevBacktrackRecord_GlyphCount(data)           Data_UInt16(data, 0)
#define SFRevBacktrackRecord_CoverageOffsets(data)      Data_Subdata(data, 2)
#define SFRevBacktrackRecord_RevLookaheadRecord(data, glyphCount) \
                                                        Data_Subdata(data, 2 + ((glyphCount) * 2))

#define SFRevLookaheadRecord_GlyphCount(data)           Data_UInt16(data, 0)
#define SFRevLookaheadRecord_CoverageOffsets(data)      Data_Subdata(data, 2)
#define SFRevLookaheadRecord_RevSubstRecord(data, glyphCount) \
                                                        Data_Subdata(data, 2 + ((glyphCount) * 2))

#define SFRevSubstRecord_GlyphCount(data)               Data_UInt16(data, 0)
#define SFRevSubstRecord_Substitute(data, index)        Data_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

#endif
