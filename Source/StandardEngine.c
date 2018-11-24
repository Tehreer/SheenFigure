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
#include "ShapingEngine.h"
#include "ShapingKnowledge.h"
#include "TextProcessor.h"
#include "StandardEngine.h"

static ScriptKnowledgeRef StandardKnowledgeSeekScript(const void *object, SFTag scriptTag);
static void StandardEngineProcessAlbum(const void *object, SFAlbumRef album);

enum {
    StandardFeatureMaskNone = 0 << 0
};

static FeatureInfo StandardSubstFeatureArray[] = {
    /* Language based forms. */
    { 0, TAG('c', 'c', 'm', 'p'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    /* Typographical forms */
    { 1, TAG('l', 'i', 'g', 'a'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    { 1, TAG('c', 'l', 'i', 'g'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
};
#define StandardSubstFeatureCount (sizeof(StandardSubstFeatureArray) / sizeof(FeatureInfo))

static FeatureInfo StandardPosFeatureArray[] = {
    /* Positioning features. */
    { 2, TAG('d', 'i', 's', 't'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('k', 'e', 'r', 'n'), ON_BY_DEFAULT, INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('m', 'a', 'r', 'k'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone },
    { 2, TAG('m', 'k', 'm', 'k'), REQUIRED,      INDIVIDUAL, StandardFeatureMaskNone }
};
#define StandardPosFeatureCount (sizeof(StandardPosFeatureArray) / sizeof(FeatureInfo))

static ScriptKnowledge StandardScriptKnowledge = {
    SFTextDirectionLeftToRight,
    { StandardSubstFeatureArray, StandardSubstFeatureCount },
    { StandardPosFeatureArray, StandardPosFeatureCount }
};

ShapingKnowledge StandardKnowledgeInstance = {
    &StandardKnowledgeSeekScript
};

static ScriptKnowledgeRef StandardKnowledgeSeekScript(const void *object, SFTag scriptTag)
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

    /* Use same features for unknown scripts as they are not script specific. */
    return &StandardScriptKnowledge;
}

static ShapingEngine StandardEngineBase = {
    &StandardEngineProcessAlbum
};

SF_INTERNAL void StandardEngineInitialize(StandardEngineRef standardEngine, SFArtistRef artist)
{
    standardEngine->_base = StandardEngineBase;
    standardEngine->_artist = artist;
}

static void StandardEngineProcessAlbum(const void *object, SFAlbumRef album)
{
    StandardEngineRef standardEngine = (StandardEngineRef)object;
    SFArtistRef artist = standardEngine->_artist;
    TextProcessor processor;

    TextProcessorInitialize(&processor, artist->pattern, album, artist->textDirection,
                              artist->ppemWidth, artist->ppemHeight, SFFalse);
    TextProcessorDiscoverGlyphs(&processor);
    TextProcessorSubstituteGlyphs(&processor);
    TextProcessorPositionGlyphs(&processor);
    TextProcessorWrapUp(&processor);
}
