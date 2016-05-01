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

#ifndef _SF_SHAPING_KNOWLEDGE_INTERNAL_H
#define _SF_SHAPING_KNOWLEDGE_INTERNAL_H

#include <SFConfig.h>
#include <SFTypes.h>

#include "SFArtist.h"

typedef struct _SFFeatureInfo {
    SFTag featureTag;
    SFUInt16 featureMask;
} SFFeatureInfo, *SFFeatureInfoRef;

typedef struct _SFScriptKnowledge {
    /**
     * Default direction of the script.
     */
    SFTextDirection defaultDirection;
    /**
     * Information of all features supported by the script in implementation order.
     */
    struct {
        SFFeatureInfo *items;
        SFUInteger count;
    } featureInfos;
	/**
     * Ranges of those features which must be applied simultaneously.
     */
    struct {
        SFRange *items;
        SFUInteger count;
    } featureUnits;
} SFScriptKnowledge, *SFScriptKnowledgeRef;

/**
 * A common interface for the knowledge of a shaping engine.
 */
typedef struct _SFShapingKnowledge {
    SFScriptKnowledgeRef (*_seekScript)(const void *, SFTag);
} SFShapingKnowledge, *SFShapingKnowledgeRef;

/**
 * Returns script knowledge related to given script, or NULL.
 */
SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFTag scriptTag);

#endif
