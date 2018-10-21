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

static SFScriptKnowledgeRef StandardKnowledgeSeekScript(const void *object, SFTag scriptTag);
static void StandardEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    StandardFeatureMaskNone = 0 << 0
};

static SFFeatureInfo StandardSubstFeatureArray[] = {
    /* Language based forms. */
    { 0, TAG('c', 'c', 'm', 'p'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    /* Typographical forms */
    { 1, TAG('l', 'i', 'g', 'a'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    { 1, TAG('c', 'l', 'i', 'g'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
};
#define StandardSubstFeatureCount (sizeof(StandardSubstFeatureArray) / sizeof(SFFeatureInfo))

static SFFeatureInfo StandardPosFeatureArray[] = {
    /* Positioning features. */
    { 2, TAG('d', 'i', 's', 't'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('k', 'e', 'r', 'n'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('m', 'a', 'r', 'k'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('m', 'k', 'm', 'k'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone }
};
#define StandardPosFeatureCount (sizeof(StandardPosFeatureArray) / sizeof(SFFeatureInfo))

static SFScriptKnowledge StandardScriptKnowledge = {
    SFTextDirectionLeftToRight,
    { StandardSubstFeatureArray, StandardSubstFeatureCount },
    { StandardPosFeatureArray, StandardPosFeatureCount }
};

SFShapingKnowledge SFStandardKnowledgeInstance = {
    &StandardKnowledgeSeekScript
};

static SFScriptKnowledgeRef StandardKnowledgeSeekScript(const void *object, SFTag scriptTag)
{
    switch (scriptTag) {
        case TAG('a', 'r', 'm', 'n'):
        case TAG('c', 'y', 'r', 'l'):
        case TAG('g', 'e', 'o', 'r'):
        case TAG('g', 'r', 'e', 'k'):
        case TAG('l', 'a', 't', 'n'):
        case TAG('o', 'g', 'a', 'm'):
        case TAG('r', 'u', 'n', 'r'):
            return &StandardScriptKnowledge;
    }

    return NULL;
}

static SFShapingEngine StandardEngineBase = {
    &StandardEngineProcessAlbum
};

SF_INTERNAL void SFStandardEngineInitialize(SFStandardEngineRef standardEngine, SFArtistRef artist)
{
    standardEngine->_base = StandardEngineBase;
    standardEngine->_artist = artist;
}

static void StandardEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    SFStandardEngineRef standardEngine = (SFStandardEngineRef)object;
    SFArtistRef artist = standardEngine->_artist;
    SFTextProcessor processor;

    SFTextProcessorInitialize(&processor, artist->pattern, album, artist->textDirection,
                              artist->ppemWidth, artist->ppemHeight, SFFalse);
    SFTextProcessorDiscoverGlyphs(&processor);
    SFTextProcessorSubstituteGlyphs(&processor);
    SFTextProcessorPositionGlyphs(&processor);
    SFTextProcessorWrapUp(&processor);
}
