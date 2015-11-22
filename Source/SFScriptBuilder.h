/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#ifndef SF_SCRIPT_BUILDER_INTERNAL_H
#define SF_SCRIPT_BUILDER_INTERNAL_H

#include <SFConfig.h>
#include <SFFeature.h>
#include <SFLanguage.h>
#include <SFScript.h>
#include <SFTypes.h>

#include "SFScriptCache.h"

/**
 * ScriptBuilder is an inner class of ScriptCache, therefore it can access private members of
 * ScriptCache.
 */

struct _SFScriptBuilder;
typedef struct _SFScriptBuilder SFScriptBuilder;
typedef SFScriptBuilder *SFScriptBuilderRef;

/**
 * Provides a way to cache scripts, languages, features and lookups.
 */
struct _SFScriptBuilder {
    SFScriptCacheRef scriptCache;           /**< Reference to the script cache being built. */
    SFScriptDetailRef _currentScript;       /**< Reference to the script record being built. */
    SFLanguageDetailRef _currentLanguage;   /**< Reference to the language record being built. */
    SFHeaderKind _currentHeader;            /**< The kind of table whose features are being added. */
    SFFeatureDetailRef _currentFeature;     /**< Reference to the feature record being built. */
    SFLookupDetailRef _currentlookup;       /**< Reference to the lookup record being built. */
    SFUInteger _scriptCapacity;             /**< Capacity of scripts in script cache. */
    SFUInteger _languageCapacity;           /**< Capacity of languages in current script. */
    SFUInteger _featureCapacity;            /**< Capacity of features in current language. */
    SFUInteger _groupCapacity;              /**< Capacity of groups in current language. */
    SFUInteger _lookupCapacity;             /**< Capacity of lookups in current feature. */
};

/**
 * Initializes builder for a script cache.
 * @param scriptCache
 *      The script cache to be built. It must be an uninitialized object as it
 *		will be manipulated by the builder.
 */
SF_INTERNAL void SFScriptBuilderInitialize(SFScriptBuilderRef builder, SFScriptCacheRef scriptCache);

/**
 * Reserves capacity for desired number of scripts in cache.
 */
SF_INTERNAL void SFScriptBuilderReserveScripts(SFScriptBuilderRef builder, SFUInteger scriptCount);

/**
 * Adds a new script in cache.
 */
SF_INTERNAL void SFScriptBuilderAddScript(SFScriptBuilderRef builder, SFScript script);

/**
 * Reserves capacity for desired number of languages in recently added script.
 */
SF_INTERNAL void SFScriptBuilderReserveLanguages(SFScriptBuilderRef builder, SFUInteger languageCount);

/**
 * Adds a new language in recently added script.
 */
SF_INTERNAL void SFScriptBuilderAddLanguage(SFScriptBuilderRef builder, SFLanguage language);

/**
 * Reserves capacity for desired number of features in recently added language.
 */
SF_INTERNAL void SFScriptBuilderReserveFeatures(SFScriptBuilderRef builder, SFUInteger featureCount);

/**
 * Begins insertions of features having specified kind in recently added language.
 */
SF_INTERNAL void SFScriptBuilderBeginFeatures(SFScriptBuilderRef builder, SFHeaderKind kind);

/**
 * Adds a new feature in recently added language.
 */
SF_INTERNAL void SFScriptBuilderAddFeature(SFScriptBuilderRef builder, SFFeature feature, SFHeaderKind kind, SFUInteger orderIndex);

/**
 * Ends insertions of features in recently added language.
 */
SF_INTERNAL void SFScriptBuilderEndFeatures(SFScriptBuilderRef builder);

/**
 * Reserves capacity for desired number of feature groups in recently added language.
 */
SF_INTERNAL void SFScriptBuilderReserveGroups(SFScriptBuilderRef builder, SFUInteger groupCount);

/**
* Adds a feature group in recently added language.
*/
SF_INTERNAL void SFScriptBiulderAddGroup(SFScriptBuilderRef builder, SFUInteger featureIndex, SFUInteger featureCount);

/**
 * Reserves capacity for desired number of lookups in recently added feature.
 */
SF_INTERNAL void SFScriptBuilderReserveLookups(SFScriptBuilderRef builder, SFUInteger lookupCount);

/**
 * Adds a new lookup in recently added feature.
 */
SF_INTERNAL void SFScriptBuilderAddLookup(SFScriptBuilderRef builder, SFData lookupTable, SFUInteger orderIndex);

/**
 * Closes recently added feature.
 */
SF_INTERNAL void SFScriptBuilderCloseFeature(SFScriptBuilderRef builder);

/**
 * Closes recently added language.
 */
SF_INTERNAL void SFScriptBuilderCloseLanguage(SFScriptBuilderRef builder);

/**
 * Closes recently added script.
 */
SF_INTERNAL void SFScriptBuilderCloseScript(SFScriptBuilderRef builder);

SF_INTERNAL void SFScriptBuilderInvalidate(SFScriptBuilderRef builder);

#endif
