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

#ifndef _SF_INTERNAL_UNIFIED_ENGINE_H
#define _SF_INTERNAL_UNIFIED_ENGINE_H

#include <SFConfig.h>

#include "ArabicEngine.h"
#include "SFArtist.h"
#include "ShapingEngine.h"
#include "ShapingKnowledge.h"
#include "SimpleEngine.h"
#include "StandardEngine.h"

typedef union _UnifiedEngine {
    ShapingEngine _base;
    ArabicEngine _arabicEngine;
    StandardEngine _standardEngine;
    SimpleEngine _simpleEngine;
} UnifiedEngine, *UnifiedEngineRef;

extern ShapingKnowledge UnifiedKnowledgeInstance;

SF_INTERNAL void UnifiedEngineInitialize(UnifiedEngineRef unifiedEngine, SFArtistRef artist);

#endif
