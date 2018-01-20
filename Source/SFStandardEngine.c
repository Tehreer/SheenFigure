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
#include "SFShapingEngine.h"
#include "SFShapingKnowledge.h"
#include "SFTextProcessor.h"
#include "SFStandardEngine.h"

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *object, SFTag scriptTag);
static void _SFStandardEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    _SFStandardFeatureMaskNone = 0 << 0
};

static SFFeatureInfo _SFStandardSubstFeatureArray[] = {
    /* Language based forms. */
    { 1, SFTagMake('c', 'c', 'm', 'p'), SFFeatureNatureOn, _SFStandardFeatureMaskNone },
    /* Typographical forms */
    { 1, SFTagMake('l', 'i', 'g', 'a'), SFFeatureNatureOn, _SFStandardFeatureMaskNone },
    { 1, SFTagMake('c', 'l', 'i', 'g'), SFFeatureNatureOn, _SFStandardFeatureMaskNone },
};
#define _SFStandardSubstFeatureCount (sizeof(_SFStandardSubstFeatureArray) / sizeof(SFFeatureInfo))

static SFFeatureInfo _SFStandardPosFeatureArray[] = {
    /* Positioning features. */
    { 1, SFTagMake('d', 'i', 's', 't'), SFFeatureNatureRequired, _SFStandardFeatureMaskNone },
    { 1, SFTagMake('k', 'e', 'r', 'n'), SFFeatureNatureOn,       _SFStandardFeatureMaskNone },
    { 1, SFTagMake('m', 'a', 'r', 'k'), SFFeatureNatureRequired, _SFStandardFeatureMaskNone },
    { 1, SFTagMake('m', 'k', 'm', 'k'), SFFeatureNatureRequired, _SFStandardFeatureMaskNone }
};
#define _SFStandardPosFeatureCount (sizeof(_SFStandardPosFeatureArray) / sizeof(SFFeatureInfo))

static SFScriptKnowledge _SFStandardScriptKnowledge = {
    SFTextDirectionLeftToRight,
    { _SFStandardSubstFeatureArray, _SFStandardSubstFeatureCount },
    { _SFStandardPosFeatureArray, _SFStandardPosFeatureCount }
};

SFShapingKnowledge SFStandardKnowledgeInstance = {
    &_SFStandardKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *object, SFTag scriptTag)
{
    switch (scriptTag) {
        case SFTagMake('a', 'r', 'm', 'n'):
        case SFTagMake('c', 'y', 'r', 'l'):
        case SFTagMake('g', 'e', 'o', 'r'):
        case SFTagMake('g', 'r', 'e', 'k'):
        case SFTagMake('l', 'a', 't', 'n'):
        case SFTagMake('o', 'g', 'a', 'm'):
        case SFTagMake('r', 'u', 'n', 'r'):
            return &_SFStandardScriptKnowledge;
    }

    return NULL;
}

static SFShapingEngine _SFStandardEngineBase = {
    &_SFStandardEngineProcessAlbum
};

SF_INTERNAL void SFStandardEngineInitialize(SFStandardEngineRef standardEngine, SFArtistRef artist)
{
    standardEngine->_base = _SFStandardEngineBase;
    standardEngine->_artist = artist;
}

static void _SFStandardEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    SFStandardEngineRef standardEngine = (SFStandardEngineRef)object;
    SFArtistRef artist = standardEngine->_artist;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, artist->pattern, album, artist->textDirection, artist->textMode, SFFalse);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);
}
