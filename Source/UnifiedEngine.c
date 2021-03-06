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

#include <SFConfig.h>

#include "ArabicEngine.h"
#include "SFArtist.h"
#include "ShapingKnowledge.h"
#include "StandardEngine.h"
#include "UnifiedEngine.h"

static ScriptKnowledgeRef UnifiedKnowledgeSeekScript(const void *object, SFTag scriptTag);

ShapingKnowledge UnifiedKnowledgeInstance = {
    &UnifiedKnowledgeSeekScript
};

static ScriptKnowledgeRef UnifiedKnowledgeSeekScript(const void *object, SFTag scriptTag)
{
    ScriptKnowledgeRef knowledge;

    knowledge = ShapingKnowledgeSeekScript(&ArabicKnowledgeInstance, scriptTag);

    if (!knowledge) {
        knowledge = ShapingKnowledgeSeekScript(&StandardKnowledgeInstance, scriptTag);
    }

    return knowledge;
}

SF_INTERNAL void UnifiedEngineInitialize(UnifiedEngineRef unifiedEngine, SFArtistRef artist)
{
    ScriptKnowledgeRef knowledge;

    knowledge = ShapingKnowledgeSeekScript(&ArabicKnowledgeInstance, artist->pattern->scriptTag);
    if (knowledge) {
        ArabicEngineInitialize(&unifiedEngine->_arabicEngine, artist);
        return;
    }

    StandardEngineInitialize(&unifiedEngine->_standardEngine, artist);
}
