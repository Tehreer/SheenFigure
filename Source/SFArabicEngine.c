/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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

#include "SFAlbum.h"
#include "SFArtist.h"
#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"
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

static SFFeatureInfo _SFArabicSubstFeatureArray[] = {
    /* Language based forms */
    { 1, SFTagMake('c', 'c', 'm', 'p'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    { 1, SFTagMake('i', 's', 'o', 'l'), SFFeatureNatureRequired, _SFArabicFeatureMaskIsolated },
    { 1, SFTagMake('f', 'i', 'n', 'a'), SFFeatureNatureRequired, _SFArabicFeatureMaskFinal },
    { 1, SFTagMake('m', 'e', 'd', 'i'), SFFeatureNatureRequired, _SFArabicFeatureMaskMedial },
    { 1, SFTagMake('i', 'n', 'i', 't'), SFFeatureNatureRequired, _SFArabicFeatureMaskInitial },
    { 1, SFTagMake('r', 'l', 'i', 'g'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    { 1, SFTagMake('c', 'a', 'l', 't'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    /* Typographical forms */
    { 1, SFTagMake('l', 'i', 'g', 'a'), SFFeatureNatureOn,       _SFArabicFeatureMaskNone },
    { 1, SFTagMake('d', 'l', 'i', 'g'), SFFeatureNatureOff,      _SFArabicFeatureMaskNone },
    { 1, SFTagMake('c', 's', 'w', 'h'), SFFeatureNatureOff,      _SFArabicFeatureMaskNone },
    { 1, SFTagMake('m', 's', 'e', 't'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
};
#define _SFArabicSubstFeatureCount (sizeof(_SFArabicSubstFeatureArray) / sizeof(SFFeatureInfo))

static SFFeatureInfo _SFArabicPosFeatureArray[] = {
    /* Positioning features */
    { 1, SFTagMake('c', 'u', 'r', 's'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    { 1, SFTagMake('k', 'e', 'r', 'n'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    { 1, SFTagMake('m', 'a', 'r', 'k'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
    { 1, SFTagMake('m', 'k', 'm', 'k'), SFFeatureNatureRequired, _SFArabicFeatureMaskNone },
};
#define _SFArabicPosFeatureCount (sizeof(_SFArabicPosFeatureArray) / sizeof(SFFeatureInfo))

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    SFTextDirectionRightToLeft,
    { _SFArabicSubstFeatureArray, _SFArabicSubstFeatureCount },
    { _SFArabicPosFeatureArray, _SFArabicPosFeatureCount }
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
    SFCodepointsRef codepoints = album->codepoints;
    SFUInteger currentIndex = 0;
    SFUInteger nextIndex = 0;
    SFJoiningType priorJoiningType;
    SFJoiningType joiningType;

    SFCodepointsReset(codepoints);

    priorJoiningType = SFJoiningTypeU;
    joiningType = _SFDetermineJoiningType(SFCodepointsNext(codepoints));

    while (joiningType != SFJoiningTypeNil) {
        SFUInt16 featureMask = _SFArabicFeatureMaskNone;
        SFJoiningType nextJoiningType = SFJoiningTypeNil;
        SBCodepoint nextCodepoint;

        /* Find the joining type of next character. */
        while ((nextCodepoint = SFCodepointsNext(codepoints)) != SFCodepointInvalid) {
            nextIndex += 1;
            nextJoiningType = _SFDetermineJoiningType(nextCodepoint);

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
        SFAlbumSetFeatureMask(album, currentIndex, featureMask);

        /* Move to the next character. */
        priorJoiningType = joiningType;
        currentIndex = nextIndex;
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
