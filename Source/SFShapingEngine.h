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

#ifndef SF_SHAPING_ENGINE_INTERNAL_H
#define SF_SHAPING_ENGINE_INTERNAL_H

#include <SFConfig.h>
#include <SFDirection.h>
#include <SFFeature.h>
#include <SFScript.h>
#include <SFTypes.h>

#include "SFCollection.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFFont.h"
#include "SFScriptCache.h"
#include "SFTableCache.h"

/* ShapingEngine is an abstract class. */

struct _SFScriptKnowledge;
struct _SFShapingKnowledge;
struct _SFShapingEngine;

typedef struct _SFScriptKnowledge SFScriptKnowledge;
typedef struct _SFShapingKnowledge SFShapingKnowledge;
typedef struct _SFShapingEngine SFShapingEngine;

typedef const SFScriptKnowledge *SFScriptKnowledgeRef;
typedef const SFShapingKnowledge *SFShapingKnowledgeRef;
typedef SFShapingEngine *SFShapingEngineRef;

struct _SFScriptKnowledge {
    /* Virtual function. */
    SFUInteger (*_seekFeature)(const void *, SFFeature);
	/**
	 * All features of the script, substitution as well as positioning, in
	 * implementation order.
	 */
    const SFFeature *featureArray;
	/**
	 * All group ranges covering those features which must be applied
	 * simultaneously.
	 */
    const SFRange *groupArray;
    SFUInteger featureCount;                /**< Total number of features supported by the script. */
    SFUInteger groupCount;                  /**< Total number of groups. */
};

struct _SFShapingKnowledge {
    /* Abstract function. */
    SFScriptKnowledgeRef (*_seekScript)(const void *, SFScript);
};

typedef struct _SFShapingTraits {
    SFScript script;
    SFLanguage language;
    SFDirection direction;
} SFShapingTraits;

struct _SFShapingEngine {
    SFFontRef _font;
    SFTableCache _tables;
    SFData _glyphClassDef;
    SFLanguageDetailRef _langDetail;
    SFCollectionRef _collection;
    SFShapingTraits _traits;
    /* State values. */
    SFFeature _feature;
    SFLookupFlag _lookupFlag;
    SFHeaderKind _headerKind;
};

/**
 * Returns script knowledge related to given script, or NULL.
 */
SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScript script);

/**
 * Returns feature index in script knowledge, or SFInvalidIndex.
 */
SF_INTERNAL SFUInteger SFScriptKnowledgeSeekFeature(SFScriptKnowledgeRef scriptKnowledge, SFFeature feature);

SF_INTERNAL void SFShapingEngineInitialize(SFShapingEngineRef shapingEngine, SFShapingTraits traits, SFFont font, SFCollectionRef collection);

SF_INTERNAL void SFShapingEngineDiscoverGlyphs(SFShapingEngineRef shapingEngine);

#endif
