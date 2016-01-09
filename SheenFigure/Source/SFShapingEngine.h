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

#ifndef _SF_SHAPING_ENGINE_INTERNAL_H
#define _SF_SHAPING_ENGINE_INTERNAL_H

#include <SFConfig.h>
#include <SFFeatureTag.h>
#include <SFTypes.h>

#include "SFAlbum.h"
#include "SFFont.h"
#include "SFGlyphTraits.h"

typedef struct _SFFeatureInfo {
    SFFeatureTag featureTag;
    SFGlyphTraits requiredTraits;
} SFFeatureInfo, *SFFeatureInfoRef;

typedef struct _SFScriptKnowledge {
    struct {
        SFFeatureInfo *items;
        SFUInteger count;
    } featureInfos;
	/* Groups of features in implementation order; Substitution as well as positioning. */
    struct {
        SFRange *items;
        SFUInteger count;
    } featureUnits;
} SFScriptKnowledge, *SFScriptKnowledgeRef;

/**
 * A common interface for the knowledge of a shaping engine.
 */
typedef struct _SFShapingKnowledge {
    SFScriptKnowledgeRef (*_seekScript)(const void *, SFScriptTag);
} SFShapingKnowledge, *SFShapingKnowledgeRef;

/**
 * A common interface of all shaping engines.
 */
typedef struct _SFShapingEngine {
    void (*_processAlbum)(const void *, SFAlbumRef);
} SFShapingEngine, *SFShapingEngineRef;

/**
 * Returns script knowledge related to given script, or NULL.
 */
SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScriptTag scriptTag);

SF_INTERNAL void SFShapingEngineProcessAlbum(SFShapingEngineRef shapingEngine, SFAlbumRef album);

#endif
