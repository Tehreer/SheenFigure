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
#include <SFTypes.h>

#include <stdlib.h>

#include "SFUnifiedEngine.h"
#include "SFArtist.h"

SFArtistRef SFArtistCreate(void)
{
    SFArtistRef artist = malloc(sizeof(SFArtist));
    artist->_retainCount = 1;

    return artist;
}

void SFArtistSetTextAsCodepoints(SFArtistRef artist, SFCodepoint *codepoints, SFUInteger length)
{
    artist->_codepointArray = codepoints;
    artist->_codepointCount = length;
}

void SFArtistSetPattern(SFArtistRef artist, SFPatternRef pattern)
{
    artist->_pattern = pattern;
}

void SFArtistFillAlbum(SFArtistRef artist, SFAlbumRef album)
{
    SFUnifiedEngine unifiedEngine;
    SFShapingEngineRef shapingEngine;

    SFAlbumReset(album, artist->_codepointArray, artist->_codepointCount);
    SFUnifiedEngineInitialize(&unifiedEngine, artist->_pattern);

    shapingEngine = (SFShapingEngineRef)&unifiedEngine;
    SFShapingEngineProcessAlbum(shapingEngine, album);
}

SFArtistRef SFArtistRetain(SFArtistRef artist)
{
    if (artist) {
        artist->_retainCount++;
    }

    return artist;
}

void SFArtistRelease(SFArtistRef artist)
{
    if (artist && --artist->_retainCount == 0) {
        free(artist);
    }
}
