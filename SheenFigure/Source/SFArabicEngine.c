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
#include <SFFeatureTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include <stddef.h>

#include "SFAssert.h"
#include "SFJoiningType.h"
#include "SFJoiningTypeLookup.h"
#include "SFShapingEngine.h"
#include "SFTextProcessor.h"
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFScriptTag scriptTag);
static void SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    _SFGlyphTraitIsolated = SFGlyphTraitMakeSpecial(0),
    _SFGlyphTraitInitial  = SFGlyphTraitMakeSpecial(1),
    _SFGlyphTraitMedial   = SFGlyphTraitMakeSpecial(2),
    _SFGlyphTraitFinal    = SFGlyphTraitMakeSpecial(3)
};

static SFFeatureInfo _SFArabicFeatureInfoArray[] = {
    /* Language based forms */
    { SFFeatureTagCCMP, SFGlyphTraitNone },
    { SFFeatureTagISOL, _SFGlyphTraitIsolated },
    { SFFeatureTagFINA, _SFGlyphTraitFinal },
    { SFFeatureTagMEDI, _SFGlyphTraitMedial },
    { SFFeatureTagINIT, _SFGlyphTraitInitial },
    { SFFeatureTagRLIG, SFGlyphTraitNone },
    { SFFeatureTagCALT, SFGlyphTraitNone },
    /* Typographical forms */
    { SFFeatureTagLIGA, SFGlyphTraitNone },
    { SFFeatureTagDLIG, SFGlyphTraitNone },
    { SFFeatureTagCSWH, SFGlyphTraitNone },
    { SFFeatureTagMSET, SFGlyphTraitNone },
    /* Positioning features */
    { SFFeatureTagCURS, SFGlyphTraitNone },
    { SFFeatureTagKERN, SFGlyphTraitNone },
    { SFFeatureTagMARK, SFGlyphTraitNone },
    { SFFeatureTagMKMK, SFGlyphTraitNone }
};
static const SFUInteger _SFArabicFeatureInfoCount = sizeof(_SFArabicFeatureInfoArray) / sizeof(SFFeatureInfo);

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    { _SFArabicFeatureInfoArray, _SFArabicFeatureInfoCount },
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

SF_INTERNAL void SFArabicEngineInitialize(SFArabicEngineRef arabicEngine, SFPatternRef pattern)
{
    arabicEngine->_base = _SFArabicEngineBase;
    arabicEngine->_pattern = pattern;
}

static void _SFPutArabicFeatureTraits(SFAlbumRef album)
{
    const SFCodepoint *codepoints = album->codePointArray;
    SFUInteger length = album->codePointCount;
    SFJoiningType priorJoiningType = SFJoiningTypeU;
    SFJoiningType joiningType = SFJoiningTypeDetermine(codepoints[0]);
    SFUInteger index = 0;

    while (joiningType != SFJoiningTypeNil) {
        SFGlyphTraits traits = SFGlyphTraitNone;
        SFJoiningType nextJoiningType = SFJoiningTypeNil;
        SFUInteger nextIndex = index;

        /* Find the joining type of next character. */
        while (++nextIndex < length) {
            nextJoiningType = SFJoiningTypeDetermine(codepoints[nextIndex]);

            /* Normalize the joining type of next character. */
            switch (nextJoiningType) {
            case SFJoiningTypeT:
                break;

            case SFJoiningTypeC:
                nextJoiningType = SFJoiningTypeD;
                goto Process;

            default:
                goto Process;
            }
        }

    Process:
        switch (joiningType) {
        case SFJoiningTypeR:
            switch (priorJoiningType) {
            case SFJoiningTypeD:
                traits |= _SFGlyphTraitFinal;
                break;

            default:
                traits |= _SFGlyphTraitIsolated;
                break;
            }
            break;

        case SFJoiningTypeD:
            switch (priorJoiningType) {
            case SFJoiningTypeD:
                switch (nextJoiningType) {
                case SFJoiningTypeR:
                case SFJoiningTypeD:
                    traits |= _SFGlyphTraitMedial;
                    break;

                default:
                    traits |= _SFGlyphTraitFinal;
                    break;
                }
                break;

            default:
                switch (nextJoiningType) {
                case SFJoiningTypeR:
                case SFJoiningTypeD:
                    traits |= _SFGlyphTraitInitial;
                    break;
                    
                default:
                    traits |= _SFGlyphTraitIsolated;
                    break;
                }
                break;
            }
            break;

        /* Can only occur for first character. Should be treated same as dual joining. */
        case SFJoiningTypeC:
            joiningType = SFJoiningTypeD;
            goto Process;

        /* Can only occur for first character. Should be treated as if there was no character. */
        case SFJoiningTypeT:
            joiningType = SFJoiningTypeU;
            break;
        }

        /* Save the traits of current character. */
        traits |= SFAlbumGetTraits(album, index);
        SFAlbumSetTraits(album, index, traits);

        /* Move to the next character. */
        priorJoiningType = joiningType;
        index = nextIndex;
        joiningType = nextJoiningType;
    }
}

static void SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    SFArabicEngineRef arabicEngine = (SFArabicEngineRef)object;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, arabicEngine->_pattern, album);
    SFTextProcessorDiscoverGlyphs(&processor);
    _SFPutArabicFeatureTraits(album);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
}
