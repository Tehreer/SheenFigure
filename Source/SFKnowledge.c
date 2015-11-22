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

#include <SFConfig.h>
#include <SFFeature.h>
#include <SFScript.h>
#include <SFTypes.h>

#include <stddef.h>

#include "SFAssert.h"
#include "SFArabicEngine.h"
#include "SFShapingEngine.h"
#include "SFStandardEngine.h"
#include "SFKnowledge.h"

static const SFShapingKnowledgeRef _SFKnowledgeArray[] = {
    &SFStandardKnowledgeInstance,
    &SFArabicKnowledgeInstance
};
static const SFUInteger _SFKnowledgeSize = sizeof(_SFKnowledgeArray) / sizeof(SFShapingKnowledgeRef);

SF_INTERNAL void SFKnowledgeInitialize(SFKnowledgeRef knowledge)
{
    knowledge->_seek = NULL;
}

SF_INTERNAL SFBoolean SFKnowledgeSeekScript(SFKnowledgeRef knowledge, SFScript script)
{
    SFUInteger index;

    for (index = 0; index < _SFKnowledgeSize; index++) {
        const SFShapingKnowledgeRef shapingKnowledge = _SFKnowledgeArray[index];
        const SFScriptKnowledgeRef scriptKnowledge = SFShapingKnowledgeSeekScript(shapingKnowledge, script);

        if (scriptKnowledge) {
            knowledge->_seek = scriptKnowledge;
            return SFTrue;
        }
    }

    return SFFalse;
}

SF_INTERNAL SFUInteger SFKnowledgeCountFeatures(SFKnowledgeRef knowledge)
{
    return knowledge->_seek->featureCount;
}

SF_INTERNAL SFUInteger SFKnowledgeSeekFeature(SFKnowledgeRef knowledge, SFFeature feature)
{
    return SFScriptKnowledgeSeekFeature(knowledge->_seek, feature);
}

SF_INTERNAL SFUInteger SFKnowledgeCountGroups(SFKnowledgeRef knowledge)
{
    return knowledge->_seek->groupCount;
}

SF_INTERNAL SFRange SFKnowledgeSeekGroup(SFKnowledgeRef knowledge, SFUInteger groupIndex)
{
    return knowledge->_seek->groupArray[groupIndex];
}
