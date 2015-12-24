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
#include "SFShapingEngine.h"
#include "SFTextProcessor.h"
#include "SFStandardEngine.h"

static const SFFeatureTag _SFStandardFeatureTagArray[] = {
    /* Language based forms. */
    SFFeatureTagCCMP,
    /* Typographical forms */
    SFFeatureTagLIGA,
    SFFeatureTagCLIG,
    /* Positioning features. */
    SFFeatureTagDIST,
    SFFeatureTagKERN,
    SFFeatureTagMARK,
    SFFeatureTagMKMK
};
static const SFUInteger _SFStandardFeatureTagCount = sizeof(_SFStandardFeatureTagArray) / sizeof(SFFeatureTag);

static SFScriptKnowledge _SFStandardScriptKnowledge = {
    { _SFStandardFeatureTagArray, _SFStandardFeatureTagCount },
    { NULL, 0 }
};

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *object, SFScriptTag scriptTag);

SFShapingKnowledge SFStandardKnowledgeInstance = {
    &_SFStandardKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *object, SFScriptTag scriptTag)
{
    switch (scriptTag) {
    case SFScriptTagARMN:
    case SFScriptTagCYRL:
    case SFScriptTagGEOR:
    case SFScriptTagGREK:
    case SFScriptTagLATN:
    case SFScriptTagOGAM:
    case SFScriptTagRUNR:
        return &_SFStandardScriptKnowledge;
    }

    return NULL;
}


static void _SFStandardEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album);

static SFShapingEngine _SFStandardEngineBase = {
    &_SFStandardEngineProcessAlbum
};

SF_INTERNAL void SFStandardEngineInitialize(SFStandardEngineRef standardEngine)
{
    standardEngine->_base = &_SFStandardEngineBase;
}

static void _SFStandardEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album)
{
    SFTextProcessor processor;

    /* Album must NOT be null. */
    SFAssert(album != NULL);

    SFTextProcessorInitialize(&processor, pattern, album);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorManipulateGlyphs(&processor);
}
