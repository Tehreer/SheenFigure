/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#include <SBCodepoint.h>
#include <SFConfig.h>
#include <stddef.h>

#include "SFAlbum.h"
#include "SFArtist.h"
#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"
#include "SFJoiningType.h"
#include "SFJoiningTypeLookup.h"
#include "SFShapingEngine.h"
#include "SFShapingKnowledge.h"
#include "SFTextProcessor.h"
#include "SFArabicEngine.h"

static ScriptKnowledgeRef ArabicKnowledgeSeekScript(const void *object, SFTag scriptTag);
static void ArabicEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    ArabicFeatureMaskNone     = 0 << 0,
    ArabicFeatureMaskIsolated = 1 << 0,
    ArabicFeatureMaskInitial  = 1 << 1,
    ArabicFeatureMaskMedial   = 1 << 2,
    ArabicFeatureMaskFinal    = 1 << 3
};

/**
 * The OpenType spec recommends to include the items in the `rclt` feature in the `calt` feature for
 * backward compatibility. So they should be applied simultaneously to avoid the application of
 * duplicate lookups.
 */
static FeatureInfo ArabicSubstFeatureArray[] = {
    /* Language based forms */
    { 0, TAG('c', 'c', 'm', 'p'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskNone },
    { 0, TAG('i', 's', 'o', 'l'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskIsolated },
    { 0, TAG('f', 'i', 'n', 'a'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskFinal },
    { 0, TAG('m', 'e', 'd', 'i'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskMedial },
    { 0, TAG('i', 'n', 'i', 't'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskInitial },
    { 0, TAG('r', 'l', 'i', 'g'), REQUIRED, INDIVIDUAL,   ArabicFeatureMaskNone },
    { 0, TAG('r', 'c', 'l', 't'), REQUIRED, SIMULTANEOUS, ArabicFeatureMaskNone },
    { 0, TAG('c', 'a', 'l', 't'), REQUIRED, SIMULTANEOUS, ArabicFeatureMaskNone },
    /* Typographical forms */
    { 1, TAG('l', 'i', 'g', 'a'), ON_BY_DEFAULT,  INDIVIDUAL, ArabicFeatureMaskNone },
    { 1, TAG('d', 'l', 'i', 'g'), OFF_BY_DEFAULT, INDIVIDUAL, ArabicFeatureMaskNone },
    { 1, TAG('c', 's', 'w', 'h'), OFF_BY_DEFAULT, INDIVIDUAL, ArabicFeatureMaskNone },
    { 1, TAG('m', 's', 'e', 't'), REQUIRED,       INDIVIDUAL, ArabicFeatureMaskNone },
};
#define ArabicSubstFeatureCount (sizeof(ArabicSubstFeatureArray) / sizeof(FeatureInfo))

static FeatureInfo ArabicPosFeatureArray[] = {
    /* Positioning features */
    { 2, TAG('c', 'u', 'r', 's'), REQUIRED, INDIVIDUAL, ArabicFeatureMaskNone },
    { 2, TAG('k', 'e', 'r', 'n'), REQUIRED, INDIVIDUAL, ArabicFeatureMaskNone },
    { 2, TAG('m', 'a', 'r', 'k'), REQUIRED, INDIVIDUAL, ArabicFeatureMaskNone },
    { 2, TAG('m', 'k', 'm', 'k'), REQUIRED, INDIVIDUAL, ArabicFeatureMaskNone },
};
#define ArabicPosFeatureCount (sizeof(ArabicPosFeatureArray) / sizeof(FeatureInfo))

static ScriptKnowledge ArabicScriptKnowledge = {
    SFTextDirectionRightToLeft,
    { ArabicSubstFeatureArray, ArabicSubstFeatureCount },
    { ArabicPosFeatureArray, ArabicPosFeatureCount }
};

ShapingKnowledge ArabicKnowledgeInstance = {
    &ArabicKnowledgeSeekScript
};

static ScriptKnowledgeRef ArabicKnowledgeSeekScript(const void *object, SFTag scriptTag)
{
    switch (scriptTag) {
        case TAG('a', 'r', 'a', 'b'):
            return &ArabicScriptKnowledge;
    }

    return NULL;
}

static ShapingEngine ArabicEngineBase = {
    &ArabicEngineProcessAlbum
};

SF_INTERNAL void ArabicEngineInitialize(ArabicEngineRef arabicEngine, SFArtistRef artist)
{
    arabicEngine->_base = ArabicEngineBase;
    arabicEngine->_artist = artist;
}

static SFJoiningType DetermineJoiningType(SFCodepoint codepoint)
{
    SFJoiningType joiningType = SFJoiningTypeDetermine(codepoint);

    if (joiningType == SFJoiningTypeF) {
        SBGeneralCategory generalCategory = SBCodepointGetGeneralCategory(codepoint);

        switch (generalCategory) {
            case SBGeneralCategoryMN:
            case SBGeneralCategoryME:
            case SBGeneralCategoryCF:
                joiningType = SFJoiningTypeT;
                break;
                
            default:
                joiningType = SFJoiningTypeU;
                break;
        }
    }

    return joiningType;
}

static void PutArabicFeatureMask(SFAlbumRef album)
{
    SFCodepointsRef codepoints = album->codepoints;
    SFUInteger currentIndex = 0;
    SFUInteger nextIndex = 0;
    SFJoiningType priorJoiningType;
    SFJoiningType joiningType;

    SFCodepointsReset(codepoints);

    priorJoiningType = SFJoiningTypeU;
    joiningType = DetermineJoiningType(SFCodepointsNext(codepoints));

    while (joiningType != SFJoiningTypeNil) {
        SFUInt16 featureMask = ArabicFeatureMaskNone;
        SFJoiningType nextJoiningType = SFJoiningTypeNil;
        SBCodepoint nextCodepoint;

        /* Find the joining type of next character. */
        while ((nextCodepoint = SFCodepointsNext(codepoints)) != SFCodepointInvalid) {
            nextIndex += 1;
            nextJoiningType = DetermineJoiningType(nextCodepoint);

            /* Process only non-transparent characters. */
            if (nextJoiningType != SFJoiningTypeT) {
                /* Normalize the joining type of next character. */
                if (nextJoiningType == SFJoiningTypeC) {
                    nextJoiningType = SFJoiningTypeD;
                }
                break;
            }
        }

    Process:
        switch (joiningType) {
            case SFJoiningTypeL:
                if (nextJoiningType == SFJoiningTypeD || nextJoiningType == SFJoiningTypeR) {
                    featureMask |= ArabicFeatureMaskInitial;
                } else {
                    featureMask |= ArabicFeatureMaskIsolated;
                }
                break;

            case SFJoiningTypeR:
                if (priorJoiningType == SFJoiningTypeD || priorJoiningType == SFJoiningTypeL) {
                    featureMask |= ArabicFeatureMaskFinal;
                } else {
                    featureMask |= ArabicFeatureMaskIsolated;
                }
                break;

            case SFJoiningTypeD:
                if (priorJoiningType == SFJoiningTypeD || priorJoiningType == SFJoiningTypeL) {
                    if (nextJoiningType == SFJoiningTypeD || nextJoiningType == SFJoiningTypeR) {
                        featureMask |= ArabicFeatureMaskMedial;
                    } else {
                        featureMask |= ArabicFeatureMaskFinal;
                    }
                } else {
                    if (nextJoiningType == SFJoiningTypeD || nextJoiningType == SFJoiningTypeR) {
                        featureMask |= ArabicFeatureMaskInitial;
                    } else {
                        featureMask |= ArabicFeatureMaskIsolated;
                    }
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

static void ArabicEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    ArabicEngineRef arabicEngine = (ArabicEngineRef)object;
    SFArtistRef artist = arabicEngine->_artist;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, artist->pattern, album, artist->textDirection,
                              artist->ppemWidth, artist->ppemHeight, SFTrue);
    SFTextProcessorDiscoverGlyphs(&processor);
    PutArabicFeatureMask(album);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);
}
