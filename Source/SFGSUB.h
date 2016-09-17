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

#ifndef _SF_INTERNAL_GSUB_H
#define _SF_INTERNAL_GSUB_H

#include "SFData.h"
#include "SFCommon.h"

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

#define SFSingleSubst_Format(data)                      SFData_UInt16(data, 0)

#define SFSingleSubstF1_CoverageOffset(data)            SFData_UInt16(data, 2)
#define SFSingleSubstF1_DeltaGlyphID(data)              SFData_Int16 (data, 4)

#define SFSingleSubstF2_CoverageOffset(data)            SFData_UInt16(data, 2)
#define SFSingleSubstF2_GlyphCount(data)                SFData_UInt16(data, 4)
#define SFSingleSubstF2_Substitute(data, index)         SFData_UInt16(data, 6 + ((index) * 2))

/**************************************************************************************************/

/**********************************MULTIPLE SUBSTITUTION SUBTABLE**********************************/

#define SFMultipleSubst_Format(data)                    SFData_UInt16(data, 0)
#define SFMultipleSubstF1_CoverageOffset(data)          SFData_UInt16(data, 2)
#define SFMultipleSubstF1_SequenceCount(data)           SFData_UInt16(data, 4)
#define SFMultipleSubstF1_SequenceOffset(data, index)   SFData_UInt16(data, 6 + ((index) * 2))

#define SFSequence_GlyphCount(data)                     SFData_UInt16(data, 0)
#define SFSequence_Substitute(data, index)              SFData_UInt16(data, 2 + ((index) * 2))

/**************************************************************************************************/

/*********************************ALTERNATE SUBSTITUTION SUBTABLE**********************************/

#define SFAlternateSubst_Format(data)                   SFData_UInt16(data, 0)
#define SFAlternateSubstF1_CoverageOffset(data)         SFData_UInt16(data, 2)
#define SFAlternateSubstF1_AlternateSetCount(data)      SFData_UInt16(data, 4)
#define SFAlternateSubstF1_AlternateSetOffset(data, index) \
                                                        SFData_UInt16(data, 6 + ((index) * 2))

#define SFAlternateSet_GlyphCount(data)                 SFData_UInt16(data, 0)
#define SFAlternateSet_Alternate(data, index)           SFData_UInt16(data, 2 * ((index) * 2))

/**************************************************************************************************/

/**********************************LIGATURE SUBSTITUTION SUBTABLE**********************************/

#define SFLigatureSubst_Format(data)                    SFData_UInt16(data, 0)
#define SFLigatureSubstF1_CoverageOffset(data)          SFData_UInt16(data, 2)
#define SFLigatureSubstF1_LigSetCount(data)             SFData_UInt16(data, 4)
#define SFLigatureSubstF1_LigatureSetOffset(data, index) \
                                                        SFData_UInt16(data, 6 + ((index) * 2))

#define SFLigatureSet_LigatureCount(data)               SFData_UInt16(data, 0)
#define SFLigatureSet_LigatureOffset(data, index)       SFData_UInt16(data, 2 + ((index) * 2))

#define SFLigature_LigGlyph(data)                       SFData_UInt16(data, 0)
#define SFLigature_CompCount(data)                      SFData_UInt16(data, 2)
#define SFLigature_Component(data, index)               SFData_UInt16(data, 4 + ((index) * 2))

/**************************************************************************************************/

#endif
