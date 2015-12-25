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
#include <stddef.h>

#include "SFAssert.h"
#include "SFShapingEngine.h"

SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScriptTag scriptTag)
{
    /* The function must be implemented by the concrete instance. */
    SFAssert(shapingKnowledge->_seekScript != NULL);

    return (*shapingKnowledge->_seekScript)(shapingKnowledge, scriptTag);
}

SF_INTERNAL void SFShapingEngineProcessAlbum(SFShapingEngineRef shapingEngine, SFPatternRef pattern, SFAlbumRef album)
{
    /* The function must be implemented by the concrete instance. */
    SFAssert(shapingEngine->_processAlbum != NULL);

    (*shapingEngine->_processAlbum)(shapingEngine, pattern, album);
}
