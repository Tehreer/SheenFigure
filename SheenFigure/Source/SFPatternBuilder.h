/*
 * Copyright (C) 2016 Muhammad Tayyab Akram
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

#ifndef _SF_PATTERN_BUILDER_INTERNAL_H
#define _SF_PATTERN_BUILDER_INTERNAL_H

#include <SFConfig.h>
#include <SFFeatureTag.h>
#include <SFLanguageTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include "SFGlyphTraits.h"
#include "SFList.h"
#include "SFPattern.h"

/**
 * SFPatternBuilder is an inner class of SFPattern, therefore it can access private members of
 * SFPattern.
 */

/**
 * Provides a way to cache scripts, languages, features and lookups.
 */
typedef struct _SFPatternBuilder {
    SFPatternRef _pattern;
    SFFontRef _font;                /**< The font, whose pattern is being built. */
    SFUInteger _gsubUnitCount;      /**< Total number of GSUB feature units. */
    SFUInteger _gposUnitCount;      /**< Total number of GPOS feature units. */
    SFUInteger _featureIndex;       /**< Starting feature index of the feature unit being built. */
    SFScriptTag _scriptTag;         /**< Tag of the script, whose pattern is being built. */
    SFLanguageTag _languageTag;     /**< Tag of the language, whose pattern is being built. */
    SFGlyphTraits _requiredTraits;  /**< Required traits of the feature unit being built. */
    SFFeatureKind _featureKind;     /**< Kind of the features being added. */
    SFBoolean _canBuild;

    SF_LIST(SFFeatureTag) _featureTags;
    SF_LIST(SFFeatureUnit) _featureUnits;
    SF_LIST(SFUInt16) _lookupIndexes;
} SFPatternBuilder, *SFPatternBuilderRef;

/**
 * Initializes builder for a script cache.
 */
SF_INTERNAL void SFPatternBuilderInitialize(SFPatternBuilderRef builder, SFPatternRef pattern);
SF_INTERNAL void SFPatternBuilderFinalize(SFPatternBuilderRef builder);

SF_INTERNAL void SFPatternBuilderSetFont(SFPatternBuilderRef builder, SFFontRef font);
SF_INTERNAL void SFPatternBuilderSetScript(SFPatternBuilderRef builder, SFScriptTag scriptTag);
SF_INTERNAL void SFPatternBuilderSetLanguage(SFPatternBuilderRef builder, SFLanguageTag languageTag);

/**
 * Begins building features of specified kind.
 */
SF_INTERNAL void SFPatternBuilderBeginFeatures(SFPatternBuilderRef builder, SFFeatureKind featureKind);

/**
 * Adds a new feature having recently specified kind.
 */
SF_INTERNAL void SFPatternBuilderAddFeature(SFPatternBuilderRef builder, SFFeatureTag featureTag, SFGlyphTraits requiredTraits);

/**
 * Adds a new lookup in recently added feature.
 */
SF_INTERNAL void SFPatternBuilderAddLookup(SFPatternBuilderRef builder, SFUInt16 lookupIndex);

/**
 * Makes a unit of recently added features.
 */
SF_INTERNAL void SFPatternBuilderMakeFeatureUnit(SFPatternBuilderRef builder);

/**
 * Ends building features of specified kind.
 */
SF_INTERNAL void SFPatternBuilderEndFeatures(SFPatternBuilderRef builder);

SF_INTERNAL void SFPatternBuilderBuild(SFPatternBuilderRef builder);

#endif
