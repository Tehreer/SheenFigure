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
#include <SFFeatureTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include "SFAlbum.h"
#include "SFFont.h"
#include "SFPattern.h"

/* NOTE:
 *      SFShapingEngine is an abstract class.
 */

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
    SFUInteger (*_seekFeature)(const void *, SFFeatureTag);
	/**
	 * All features of the script, substitution as well as positioning, in
	 * implementation order.
	 */
    const SFFeatureTag *featureTagArray;
	/**
	 * All group ranges covering those features which must be applied
	 * simultaneously.
	 */
    const SFRange *groupArray;
    SFUInteger featureTagCount;                /**< Total number of features supported by the script. */
    SFUInteger groupCount;                  /**< Total number of groups. */
};

struct _SFShapingKnowledge {
    /* Abstract function. */
    SFScriptKnowledgeRef (*_seekScript)(const void *, SFScriptTag);
};

struct _SFShapingEngine {
    SFFontRef _font;
    SFPatternRef _pattern;
    SFScriptTag _scriptTag;
    SFLanguageTag _languageTag;
    SFDirection _direction;
};

/**
 * Returns script knowledge related to given script, or NULL.
 */
SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScriptTag scriptTag);

/**
 * Returns feature index in script knowledge, or SFInvalidIndex.
 */
SF_INTERNAL SFUInteger SFScriptKnowledgeSeekFeature(SFScriptKnowledgeRef scriptKnowledge, SFFeatureTag featureTag);

SF_INTERNAL void SFShapingEngineInitialize(SFShapingEngineRef shapingEngine, SFFontRef font, SFScriptTag scriptTag, SFLanguageTag languageTag, SFPatternRef pattern);
SF_INTERNAL void SFShapingEngineProcessCollection(SFShapingEngineRef shapingEngine, SFAlbumRef album);

#endif
