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
#include <SFTypes.h>

#include <stddef.h>

#include "SFAlbum.h"
#include "SFArtist.h"
#include "SFAssert.h"
#include "SFGeneralCategory.h"
#include "SFGeneralCategoryLookup.h"
#include "SFJoiningType.h"
#include "SFJoiningTypeLookup.h"
#include "SFShapingEngine.h"
#include "SFShapingKnowledge.h"
#include "SFTextProcessor.h"
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFTag scriptTag);
static void _SFPutArabicFeatureMask(SFAlbumRef album);
static void _SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    _SFArabicFeatureMaskNone     = 0 << 0,
    _SFArabicFeatureMaskIsolated = 1 << 0,
    _SFArabicFeatureMaskInitial  = 1 << 1,
    _SFArabicFeatureMaskMedial   = 1 << 2,
    _SFArabicFeatureMaskFinal    = 1 << 3
};

static SFFeatureInfo _SFArabicFeatureInfoArray[] = {
    /* Language based forms */
    { SFTagMake('c', 'c', 'm', 'p'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    { SFTagMake('i', 's', 'o', 'l'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskIsolated },
    { SFTagMake('f', 'i', 'n', 'a'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskFinal },
    { SFTagMake('m', 'e', 'd', 'i'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskMedial },
    { SFTagMake('i', 'n', 'i', 't'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskInitial },
    { SFTagMake('r', 'l', 'i', 'g'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    { SFTagMake('c', 'a', 'l', 't'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    /* Typographical forms */
    { SFTagMake('l', 'i', 'g', 'a'), SFFeatureBehaviourOn,       _SFArabicFeatureMaskNone },
    { SFTagMake('d', 'l', 'i', 'g'), SFFeatureBehaviourOff,      _SFArabicFeatureMaskNone },
    { SFTagMake('c', 's', 'w', 'h'), SFFeatureBehaviourOff,      _SFArabicFeatureMaskNone },
    { SFTagMake('m', 's', 'e', 't'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    /* Positioning features */
    { SFTagMake('c', 'u', 'r', 's'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    { SFTagMake('m', 'a', 'r', 'k'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    { SFTagMake('m', 'k', 'm', 'k'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone },
    { SFTagMake('k', 'e', 'r', 'n'), SFFeatureBehaviourRequired, _SFArabicFeatureMaskNone }
};
#define _SFArabicFeatureInfoCount (sizeof(_SFArabicFeatureInfoArray) / sizeof(SFFeatureInfo))

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    SFTextDirectionRightToLeft,
    { _SFArabicFeatureInfoArray, _SFArabicFeatureInfoCount },
    { NULL, 0 }
};

SFShapingKnowledge SFArabicKnowledgeInstance = {
    &_SFArabicKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *object, SFTag scriptTag)
{
    switch (scriptTag) {
    case SFTagMake('a', 'r', 'a', 'b'):
        return &_SFArabicScriptKnowledge;
    }

    return NULL;
}

static SFShapingEngine _SFArabicEngineBase = {
    &_SFArabicEngineProcessAlbum
};

SF_INTERNAL void SFArabicEngineInitialize(SFArabicEngineRef arabicEngine, SFArtistRef artist)
{
    arabicEngine->_base = _SFArabicEngineBase;
    arabicEngine->_artist = artist;
}

static SFJoiningType _SFDetermineJoiningType(SFCodepoint codepoint)
{
    SFJoiningType joiningType = SFJoiningTypeDetermine(codepoint);

    if (joiningType == SFJoiningTypeF) {
        SFGeneralCategory generalCategory = SFGeneralCategoryDetermine(codepoint);

        switch (generalCategory) {
        case SFGeneralCategoryMN:
        case SFGeneralCategoryME:
        case SFGeneralCategoryCF:
            joiningType = SFJoiningTypeT;
            break;
            
        default:
            joiningType = SFJoiningTypeU;
            break;
        }
    }

    return joiningType;
}

static void _SFPutArabicFeatureMask(SFAlbumRef album)
{
    const SFCodepoint *codepoints = album->codepointArray;
    SFUInteger length = album->codepointCount;
    SFUInteger association = SFAlbumGetSingleAssociation(album, 0);
    SFJoiningType priorJoiningType = SFJoiningTypeU;
    SFJoiningType joiningType = SFJoiningTypeDetermine(codepoints[association]);
    SFUInteger index = 0;

    while (joiningType != SFJoiningTypeNil) {
        SFUInt16 featureMask = _SFArabicFeatureMaskNone;
        SFJoiningType nextJoiningType = SFJoiningTypeNil;
        SFUInteger nextIndex = index;

        /* Find the joining type of next character. */
        while (++nextIndex < length) {
            association = SFAlbumGetSingleAssociation(album, nextIndex);
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

static void _SFArabicEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    SFArabicEngineRef arabicEngine = (SFArabicEngineRef)object;
    SFArtistRef artist = arabicEngine->_artist;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, artist->pattern, album, artist->textDirection, artist->textMode);
    SFTextProcessorDiscoverGlyphs(&processor);
    _SFPutArabicFeatureMask(album);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);
}
