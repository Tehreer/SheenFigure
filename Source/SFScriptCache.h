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

#ifndef SF_SCRIPT_CACHE_INTERNAL_H
#define SF_SCRIPT_CACHE_INTERNAL_H

#include <SFConfig.h>
#include <SFFeature.h>
#include <SFLanguage.h>
#include <SFScript.h>
#include <SFTypes.h>

#include "SFData.h"

struct _SFScriptCache;
struct _SFScriptDetail;
struct _SFLanguageDetail;
struct _SFFeatureDetail;
struct _SFGroupDetail;
struct _SFLookupDetail;

typedef struct _SFScriptCache SFScriptCache;
typedef struct _SFScriptDetail SFScriptDetail;
typedef struct _SFLanguageDetail SFLanguageDetail;
typedef struct _SFFeatureDetail SFFeatureDetail;
typedef struct _SFGroupDetail SFGroupDetail;
typedef struct _SFLookupDetail SFLookupDetail;

typedef SFScriptCache *SFScriptCacheRef;
typedef SFScriptDetail *SFScriptDetailRef;
typedef SFLanguageDetail *SFLanguageDetailRef;
typedef SFFeatureDetail *SFFeatureDetailRef;
typedef SFGroupDetail *SFGroupDetailRef;
typedef SFLookupDetail *SFLookupDetailRef;

/* The script cache keeps details about scripts, languages, features and lookups. */

/**
 * A type to represent the kind of an OpenType feature.
 */
typedef enum {
    SFHeaderKindGSUB = 0x00, /**< A value indicating that the feature belongs to 'GSUB' table. */
    SFHeaderKindGPOS = 0x01  /**< A value indicating that the feature belongs to 'GPOS' table. */
} SFHeaderKind;

/**
 * Keeps details of an OpenType lookup.
 */
struct _SFLookupDetail {
    SFData table; /**< Lookup table. */
};

/**
 * Keeps details of an OpenType feature.
 */
struct _SFFeatureDetail {
    SFLookupDetail *lookupArray; /**< All lookups of the feature in ascending order. */
    SFUInteger lookupCount;      /**< Total number of lookups in the feature. */
    SFFeature feature;           /**< Tag of the feature whose detail is kept. */
    SFHeaderKind headerKind;     /**< Kind of feature's header table. */
};

/**
 * Keeps details of a group having multiple features which must be applied simultaneously.
 */
struct _SFGroupDetail {
    SFLookupDetail *lookupArray; /**< Unique lookups of all features of the group in ascending order. */
    SFUInteger lookupCount;      /**< Total number of lookups in the group. */
    SFUInteger featureIndex;     /**< Starting index of feature array in language detail. */
    SFUInteger featureCount;     /**< Total number of features belonging to the group. */
};

/**
 * Keeps details of a language from both 'GSUB' and 'GPOS' tables.
 */
struct _SFLanguageDetail {
	struct {
		SFFeatureDetailRef gsub;   /**< Required gsub feature of the language. May be NULL. */
		SFFeatureDetailRef gpos;   /**< Required gpos feature of the language. May be NULL. */
	} requiredFeature;
    SFFeatureDetail *featureArray; /**< All features of the language in implementation order. Substitution features will be followed by positioning features. */
    SFGroupDetail *groupArray;     /**< Groups with features in 'featureArray'. */
	struct {
		SFUInteger gsub;           /**< Total number of gsub features in the language.*/
		SFUInteger gpos;	       /**< Total number of gpos features in the language.*/
	} featureCount;
    SFUInteger groupCount;         /**< Total number of feature groups in the language. */
    SFLanguage language;           /**< Tag of the language whose detail is kept. */
};

/**
 * Keeps details of a script from both 'GSUB' and 'GPOS' tables.
 */
struct _SFScriptDetail {
    SFLanguageDetailRef defaultLanguage; /**< Default language of the script. May be NULL. */
    SFLanguageDetail *languageArray;     /**< Loaded languages of the script. */
    SFUInteger languageCount;            /**< Number of loaded languages in the script. */
    SFScript script;                     /**< Tag of the script whose detail is kept. */
};

/**
 * Keeps details of OpenType scripts from both 'GSUB' and 'GPOS' tables.
 */
struct _SFScriptCache {
    SFScriptDetail *scriptArray; /**< Loaded scripts of the font. */
    SFUInteger scriptCount;      /**< Number of loaded scripts in the cache. */
};

/**
 * Returns desired language, if available.
 */
SF_INTERNAL SFLanguageDetailRef SFScriptCacheFindLanguage(SFScriptCacheRef scriptCache, SFScript script, SFLanguage language);

SF_INTERNAL void SFScriptCacheInvalidate(SFScriptCacheRef scriptCache);

#endif
