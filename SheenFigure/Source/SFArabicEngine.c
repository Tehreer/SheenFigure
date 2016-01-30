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
#include "SFShapingKnowledge.h"
#include "SFTextProcessor.h"
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFScriptTag scriptTag);
static void _SFPutArabicFeatureMask(SFAlbumRef album);
static void _SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album, SFDirection direction);

enum {
    _SFArabicFeatureMaskNone     = 0 << 0,
    _SFArabicFeatureMaskIsolated = 1 << 0,
    _SFArabicFeatureMaskInitial  = 1 << 1,
    _SFArabicFeatureMaskMedial   = 1 << 2,
    _SFArabicFeatureMaskFinal    = 1 << 3
};

static SFFeatureInfo _SFArabicFeatureInfoArray[] = {
    /* Language based forms */
    { SFFeatureTagCCMP, _SFArabicFeatureMaskNone },
    { SFFeatureTagISOL, _SFArabicFeatureMaskIsolated },
    { SFFeatureTagFINA, _SFArabicFeatureMaskFinal },
    { SFFeatureTagMEDI, _SFArabicFeatureMaskMedial },
    { SFFeatureTagINIT, _SFArabicFeatureMaskInitial },
    { SFFeatureTagRLIG, _SFArabicFeatureMaskNone },
    { SFFeatureTagCALT, _SFArabicFeatureMaskNone },
    /* Typographical forms */
    { SFFeatureTagLIGA, _SFArabicFeatureMaskNone },
    { SFFeatureTagDLIG, _SFArabicFeatureMaskNone },
    { SFFeatureTagCSWH, _SFArabicFeatureMaskNone },
    { SFFeatureTagMSET, _SFArabicFeatureMaskNone },
    /* Positioning features */
    { SFFeatureTagCURS, _SFArabicFeatureMaskNone },
    { SFFeatureTagKERN, _SFArabicFeatureMaskNone },
    { SFFeatureTagMARK, _SFArabicFeatureMaskNone },
    { SFFeatureTagMKMK, _SFArabicFeatureMaskNone }
};
static const SFUInteger _SFArabicFeatureInfoCount = sizeof(_SFArabicFeatureInfoArray) / sizeof(SFFeatureInfo);

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    SFDirectionRTL,
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
    &_SFArabicEngineProcessAlbum
};

SF_INTERNAL void SFArabicEngineInitialize(SFArabicEngineRef arabicEngine, SFPatternRef pattern)
{
    arabicEngine->_base = _SFArabicEngineBase;
    arabicEngine->_pattern = pattern;
}

static void _SFPutArabicFeatureMask(SFAlbumRef album)
{
    const SFCodepoint *codepoints = album->codePointArray;
    SFUInteger length = album->codePointCount;
    SFUInteger association = SFAlbumGetAssociation(album, 0);
    SFJoiningType priorJoiningType = SFJoiningTypeU;
    SFJoiningType joiningType = SFJoiningTypeDetermine(codepoints[association]);
    SFUInteger index = 0;

    while (joiningType != SFJoiningTypeNil) {
        SFUInt16 featureMask = _SFArabicFeatureMaskNone;
        SFJoiningType nextJoiningType = SFJoiningTypeNil;
        SFUInteger nextIndex = index;

        /* Find the joining type of next character. */
        while (++nextIndex < length) {
            association = SFAlbumGetAssociation(album, nextIndex);
            nextJoiningType = SFJoiningTypeDetermine(codepoints[association]);

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
                featureMask |= _SFArabicFeatureMaskFinal;
                break;

            default:
                featureMask |= _SFArabicFeatureMaskIsolated;
                break;
            }
            break;

        case SFJoiningTypeD:
            switch (priorJoiningType) {
            case SFJoiningTypeD:
                switch (nextJoiningType) {
                case SFJoiningTypeR:
                case SFJoiningTypeD:
                    featureMask |= _SFArabicFeatureMaskMedial;
                    break;

                default:
                    featureMask |= _SFArabicFeatureMaskFinal;
                    break;
                }
                break;

            default:
                switch (nextJoiningType) {
                case SFJoiningTypeR:
                case SFJoiningTypeD:
                    featureMask |= _SFArabicFeatureMaskInitial;
                    break;
                    
                default:
                    featureMask |= _SFArabicFeatureMaskIsolated;
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

        /* Save the mask of current character. */
        SFAlbumSetFeatureMask(album, index, featureMask);

        /* Move to the next character. */
        priorJoiningType = joiningType;
        index = nextIndex;
        joiningType = nextJoiningType;
    }
}

static void _SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album, SFDirection direction)
{
    SFArabicEngineRef arabicEngine = (SFArabicEngineRef)object;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, arabicEngine->_pattern, album, direction);
    SFTextProcessorDiscoverGlyphs(&processor);
    _SFPutArabicFeatureMask(album);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
}
