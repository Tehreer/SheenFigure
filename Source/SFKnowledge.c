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
#include <SFFeatureTag.h>
#include <SFScriptTag.h>
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

SF_INTERNAL SFBoolean SFKnowledgeSeekScript(SFKnowledgeRef knowledge, SFScriptTag scriptTag)
{
    SFUInteger index;

    for (index = 0; index < _SFKnowledgeSize; index++) {
        const SFShapingKnowledgeRef shapingKnowledge = _SFKnowledgeArray[index];
        const SFScriptKnowledgeRef scriptKnowledge = SFShapingKnowledgeSeekScript(shapingKnowledge, scriptTag);

        if (scriptKnowledge) {
            knowledge->_seek = scriptKnowledge;
            return SFTrue;
        }
    }

    return SFFalse;
}

SF_INTERNAL SFUInteger SFKnowledgeCountFeatures(SFKnowledgeRef knowledge)
{
    return knowledge->_seek->featureTagCount;
}

SF_INTERNAL SFUInteger SFKnowledgeSeekFeature(SFKnowledgeRef knowledge, SFFeatureTag featureTag)
{
    return SFScriptKnowledgeSeekFeature(knowledge->_seek, featureTag);
}

SF_INTERNAL SFFeatureTag SFKnowledgeGetFeatureAt(SFKnowledgeRef knowledge, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < knowledge->_seek->featureTagCount);

    return knowledge->_seek->featureTagArray[index];
}

SF_INTERNAL SFUInteger SFKnowledgeCountGroups(SFKnowledgeRef knowledge)
{
    return knowledge->_seek->groupCount;
}

SF_INTERNAL SFRange SFKnowledgeGetGroupAt(SFKnowledgeRef knowledge, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < knowledge->_seek->groupCount);

    return knowledge->_seek->groupArray[index];
}
