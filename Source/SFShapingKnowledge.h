/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_SHAPING_KNOWLEDGE_H
#define _SF_INTERNAL_SHAPING_KNOWLEDGE_H

#include <SFConfig.h>

#include "SFArtist.h"
#include "SFBase.h"

enum {
    SFFeatureNatureOff = 0,
    SFFeatureNatureOn = 1,
    SFFeatureNatureRequired = 2
};
typedef SFUInt16 SFFeatureNature;

typedef struct _SFFeatureInfo {
    SFUInt8 group;
    SBBoolean simultaneous;
    SFTag tag;
    SFFeatureNature nature;
    SFUInt16 mask;
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
    } substFeatures;
    struct {
        SFFeatureInfo *items;
        SFUInteger count;
    } posFeatures;
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
