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
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFScriptTag scriptTag);

static const SFFeatureTag _SFArabicFeatureTagArray[] = {
    /* Language based forms */
    SFFeatureTagCCMP,
    SFFeatureTagISOL,
    SFFeatureTagFINA,
    SFFeatureTagMEDI,
    SFFeatureTagINIT,
    SFFeatureTagRLIG,
    SFFeatureTagCALT,
    /* Typographical forms */
    SFFeatureTagLIGA,
    SFFeatureTagDLIG,
    SFFeatureTagCSWH,
    SFFeatureTagMSET,
    /* Positioning features */
    SFFeatureTagCURS,
    SFFeatureTagKERN,
    SFFeatureTagMARK,
    SFFeatureTagMKMK
};
static const SFUInteger _SFArabicFeatureTagCount = sizeof(_SFArabicFeatureTagArray) / sizeof(SFFeatureTag);

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    { _SFArabicFeatureTagArray, _SFArabicFeatureTagCount },
    { NULL, 0 }
};

SFShapingKnowledge SFArabicKnowledgeInstance = {
    &_SFArabicKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFScriptTag scriptTag)
{
    switch (scriptTag) {
    case SFScriptTagARAB:
        return &_SFArabicScriptKnowledge;
    }

    return NULL;
}


static void SFArabicEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album);

static SFShapingEngine _SFArabicEngineBase = {
    &SFArabicEngineProcessAlbum
};

SF_INTERNAL void SFArabicEngineInitialize(SFArabicEngineRef arabicEngine)
{
    arabicEngine->_base = &_SFArabicEngineBase;
}

static void SFArabicEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album)
{
    SFTextProcessor processor;

    /* Album must NOT be null. */
    SFAssert(album != NULL);

    SFTextProcessorInitialize(&processor, pattern, album);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorManipulateGlyphs(&processor);
}
