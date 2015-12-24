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

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFTextProcessor.h"
#include "SFShapingEngine.h"

SF_INTERNAL SFScriptKnowledgeRef SFShapingKnowledgeSeekScript(SFShapingKnowledgeRef shapingKnowledge, SFScriptTag scriptTag)
{
    return (*shapingKnowledge->_seekScript)(shapingKnowledge, scriptTag);
}

SF_INTERNAL SFUInteger SFScriptKnowledgeSeekFeature(SFScriptKnowledgeRef scriptKnowledge, SFFeatureTag featureTag)
{
    if (!scriptKnowledge->_seekFeature) {
        SFUInteger index;

        for (index = 0; index < scriptKnowledge->featureTagCount; index++) {
            if (scriptKnowledge->featureTagArray[index] == featureTag) {
                return index;
            }
        }

        return SFInvalidIndex;
    }

    /* Invoke overridden implementation. */
    return (*scriptKnowledge->_seekFeature)(scriptKnowledge, featureTag);
}

SF_INTERNAL void SFShapingEngineInitialize(SFShapingEngineRef shapingEngine, SFFontRef font, SFScriptTag scriptTag, SFLanguageTag languageTag, SFPatternRef pattern)
{
	/* Font must NOT be null. */
	SFAssert(font != NULL);

    shapingEngine->_font = font;
    shapingEngine->_pattern = pattern;
    shapingEngine->_scriptTag = scriptTag;
    shapingEngine->_languageTag = languageTag;
}

SF_INTERNAL void SFShapingEngineProcessCollection(SFShapingEngineRef shapingEngine, SFAlbumRef album)
{
    SFTextProcessor processor;

    /* Collection must NOT be null. */
    SFAssert(album != NULL);

    SFTextProcessorInitialize(&processor, shapingEngine->_font, shapingEngine->_pattern, album);
    SFTextProcessorDiscoverGlyphs(&processor);
    if (shapingEngine->_pattern != NULL) {
        SFTextProcessorManipulateGlyphs(&processor);
    }
}
