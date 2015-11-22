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

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFTextProcessor.h"
#include "SFShapingEngine.h"

SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScript script)
{
    (*shapingKnowledge->_seekScript)(shapingKnowledge, script);
}

SF_INTERNAL SFUInteger SFScriptKnowledgeSeekFeature(SFScriptKnowledgeRef scriptKnowledge, SFFeature feature)
{
    if (!scriptKnowledge->_seekFeature) {
        SFUInteger index;

        for (index = 0; index < scriptKnowledge->featureCount; index++) {
            if (scriptKnowledge->featureArray[index] == feature) {
                return index;
            }
        }

        return SFInvalidIndex;
    }

    /* Invoke overridden implementation. */
    return (*scriptKnowledge->_seekFeature)(scriptKnowledge, feature);
}

SF_INTERNAL void SFShapingEngineInitialize(SFShapingEngineRef shapingEngine, SFFontRef font, SFScript script, SFLanguage language)
{
	/* Font must NOT be null. */
	SFAssert(font != NULL);

    shapingEngine->_font = font;
    shapingEngine->_langDetail = SFScriptCacheFindLanguage(&font->scripts, script, language);
    shapingEngine->_script = script;
    shapingEngine->_language = language;
}

SF_INTERNAL void SFShapingEngineProcessCollection(SFShapingEngineRef shapingEngine, SFCollectionRef collection)
{
    SFTextProcessor processor;

    /* Collection must NOT be null. */
    SFAssert(collection != NULL);

    SFTextProcessorInitialize(&processor, shapingEngine->_font, shapingEngine->_langDetail, collection);
    SFTextProcessorDiscoverGlyphs(&processor);
    if (shapingEngine->_langDetail != NULL) {
        SFTextProcessorManipulateGlyphs(&processor);
    }
}
