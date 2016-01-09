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

#include <SFConfig.h>
#include <SFScriptTag.h>

#include "SFArabicEngine.h"
#include "SFStandardEngine.h"
#include "SFUnifiedEngine.h"

static SFScriptKnowledgeRef _SFUnifiedKnowledgeSeekScript(const void *object, SFScriptTag scriptTag);

SFShapingKnowledge SFUnifiedKnowledgeInstance = {
    &_SFUnifiedKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFUnifiedKnowledgeSeekScript(const void *object, SFScriptTag scriptTag)
{
    SFScriptKnowledgeRef knowledge;

    knowledge = SFShapingKnowledgeSeekScript(&SFArabicKnowledgeInstance, scriptTag);

    if (!knowledge) {
        knowledge = SFShapingKnowledgeSeekScript(&SFStandardKnowledgeInstance, scriptTag);
    }

    return knowledge;
}

SF_INTERNAL void SFUnifiedEngineInitialize(SFUnifiedEngineRef unifiedEngine, SFPatternRef pattern)
{
    SFScriptKnowledgeRef knowledge;

    knowledge = SFShapingKnowledgeSeekScript(&SFArabicKnowledgeInstance, pattern->scriptTag);
    if (knowledge) {
        SFArabicEngineInitialize(&unifiedEngine->_arabicEngine, pattern);
        return;
    }

    knowledge = SFShapingKnowledgeSeekScript(&SFStandardKnowledgeInstance, pattern->scriptTag);
    if (knowledge) {
        SFStandardEngineInitialize(&unifiedEngine->_standardEngine, pattern);
        return;
    }
}
