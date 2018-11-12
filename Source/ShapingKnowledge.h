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
    REQUIRED = 0,
    ON_BY_DEFAULT = 1,
    OFF_BY_DEFAULT = 2
};
typedef SFUInt8 FeatureStatus;

enum {
    INDIVIDUAL = 0,
    SIMULTANEOUS = 1
};
typedef SFUInt8 FeatureExecution;

typedef struct _FeatureInfo {
    SFUInt8 group;
    SFTag tag;
    FeatureStatus status;
    FeatureExecution execution;
    SFUInt16 mask;
} FeatureInfo, *FeatureInfoRef;

typedef struct _ScriptKnowledge {
    /**
     * Default direction of the script.
     */
    SFTextDirection defaultDirection;
    /**
     * Information of all features supported by the script in implementation order.
     */
    struct {
        FeatureInfo *items;
        SFUInteger count;
    } substFeatures;
    struct {
        FeatureInfo *items;
        SFUInteger count;
    } posFeatures;
} ScriptKnowledge, *ScriptKnowledgeRef;

/**
 * A common interface for the knowledge of a shaping engine.
 */
typedef struct _ShapingKnowledge {
    ScriptKnowledgeRef (*_seekScript)(const void *, SFTag);
} ShapingKnowledge, *ShapingKnowledgeRef;

/**
 * Returns script knowledge related to given script, or NULL.
 */
SF_INTERNAL ScriptKnowledgeRef ShapingKnowledgeSeekScript(ShapingKnowledgeRef shapingKnowledge, SFTag scriptTag);

#endif
