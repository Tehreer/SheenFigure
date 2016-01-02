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
static void SFArabicEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album);

enum {
    _SFGlyphTraitIsolated = SFGlyphTraitMakeSpecial(0),
    _SFGlyphTraitInitial  = SFGlyphTraitMakeSpecial(1),
    _SFGlyphTraitMedial   = SFGlyphTraitMakeSpecial(2),
    _SFGlyphTraitFinal    = SFGlyphTraitMakeSpecial(3)
};

static const SFFeatureKnowledge _SFArabicFeatureArray[] = {
    /* Language based forms */
    { SFFeatureTagCCMP, 0 },
    { SFFeatureTagISOL, _SFGlyphTraitIsolated },
    { SFFeatureTagFINA, _SFGlyphTraitFinal    },
    { SFFeatureTagMEDI, _SFGlyphTraitMedial   },
    { SFFeatureTagINIT, _SFGlyphTraitInitial  },
    { SFFeatureTagRLIG, 0 },
    { SFFeatureTagCALT, 0 },
    /* Typographical forms */
    { SFFeatureTagLIGA, 0 },
    { SFFeatureTagDLIG, 0 },
    { SFFeatureTagCSWH, 0 },
    { SFFeatureTagMSET, 0 },
    /* Positioning features */
    { SFFeatureTagCURS, 0 },
    { SFFeatureTagKERN, 0 },
    { SFFeatureTagMARK, 0 },
    { SFFeatureTagMKMK, 0 }
};
static const SFUInteger _SFArabicFeatureCount = sizeof(_SFArabicFeatureArray) / sizeof(SFFeatureKnowledge);

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    { _SFArabicFeatureArray, _SFArabicFeatureCount },
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

static SFShapingEngine _SFArabicEngineBase = {
    &SFArabicEngineProcessAlbum
};

SF_INTERNAL void SFArabicEngineInitialize(SFArabicEngineRef arabicEngine)
{
    arabicEngine->_base = _SFArabicEngineBase;
}

static void SFArabicEngineProcessAlbum(const void *object, SFPatternRef pattern, SFAlbumRef album)
{
    SFTextProcessor processor;
    SFTextProcessorInitialize(&processor, pattern, album);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
}
