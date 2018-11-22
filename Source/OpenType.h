/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_OPEN_TYPE_H
#define _SF_INTERNAL_OPEN_TYPE_H

#include <SFConfig.h>

#include "SFBase.h"
#include "Data.h"

SF_INTERNAL Data SearchScriptTable(Data scriptListTable, SFTag scriptTag);
SF_INTERNAL Data SearchLangSysTable(Data scriptTable, SFTag languageTag);
SF_INTERNAL Data SearchDefaultFeatureTable(Data langSysTable,
    Data featureListTable, SFTag featureTag, SFUInt16 *featureIndex);

SF_INTERNAL SFUInteger SearchCoverageIndex(Data coverageTable, SFGlyphID glyphID);
SF_INTERNAL SFUInt16 SearchGlyphClass(Data classDefTable, SFGlyphID glyphID);

SF_INTERNAL SFInt32 GetDevicePixels(Data deviceTable, SFUInt16 ppemSize);

SF_INTERNAL double CalculateScalarForRegion(Data regionListTable, SFUInt16 regionIndex,
    const SFInt16 *coordArray, SFUInteger coordCount);
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
