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

#include <SBCodepointSequence.h>
#include <stddef.h>
#include <stdlib.h>

#include "SFBase.h"
#include "UnifiedEngine.h"
#include "SFArtist.h"

static void LoadCodepointSequence(SBCodepointSequence *codepointSequence,
    SFStringEncoding stringEncoding, void *stringBuffer, SFUInteger stringLength)
{
    codepointSequence->stringEncoding = stringEncoding;
    codepointSequence->stringBuffer = stringBuffer;
    codepointSequence->stringLength = stringLength;
}

static SFBoolean IsValidCodepointSequence(SBCodepointSequence *codepointSequence)
{
    return (codepointSequence->stringBuffer && codepointSequence->stringLength);
}

SFArtistRef SFArtistCreate(void)
{
    SFArtistRef artist = malloc(sizeof(SFArtist));
    LoadCodepointSequence(&artist->codepointSequence, 0, NULL, 0);
    artist->pattern = NULL;
    artist->textDirection = SFTextDirectionLeftToRight;
    artist->textMode = SFTextModeForward;
    artist->ppemWidth = 0;
    artist->ppemHeight = 0;
    artist->_retainCount = 1;

    return artist;
}

void SFArtistSetPPEMSizes(SFArtistRef artist, SFUInt16 ppemWidth, SFUInt16 ppemHeight)
{
    artist->ppemWidth = ppemWidth;
    artist->ppemHeight = ppemHeight;
}

void SFArtistSetString(SFArtistRef artist, SFStringEncoding stringEncoding, void *stringBuffer, SFUInteger stringLength)
{
    LoadCodepointSequence(&artist->codepointSequence, stringEncoding, stringBuffer, stringLength);
}

void SFArtistSetPattern(SFArtistRef artist, SFPatternRef pattern)
{
    artist->pattern = SFPatternRetain(pattern);
}

void SFArtistSetTextDirection(SFArtistRef artist, SFTextDirection textDirection)
{
    switch (textDirection) {
        case SFTextDirectionLeftToRight:
        case SFTextDirectionRightToLeft:
            break;

        default:
            /* Fallback to default value. */
            textDirection = SFTextDirectionLeftToRight;
            break;
    }

    artist->textDirection = textDirection;
}

void SFArtistSetTextMode(SFArtistRef artist, SFTextMode textMode)
{
    switch (textMode) {
        case SFTextModeForward:
        case SFTextModeBackward:
            break;

        default:
            /* Fallback to default value. */
            textMode = SFTextModeForward;
            break;
    }

    artist->textMode = textMode;
}

void SFArtistFillAlbum(SFArtistRef artist, SFAlbumRef album)
{
    SFCodepoints codepoints;

    if (artist->pattern && IsValidCodepointSequence(&artist->codepointSequence)) {
        UnifiedEngine unifiedEngine;
        ShapingEngineRef shapingEngine;

        SFCodepointsInitialize(&codepoints,
                               &artist->codepointSequence,
                               artist->textMode == SFTextModeBackward);

        UnifiedEngineInitialize(&unifiedEngine, artist);
        shapingEngine = (ShapingEngineRef)&unifiedEngine;

        SFAlbumReset(album, &codepoints);
        ShapingEngineProcessAlbum(shapingEngine, album);
    } else {
        LoadCodepointSequence(&artist->codepointSequence, 0, NULL, 0);
        SFCodepointsInitialize(&codepoints, &artist->codepointSequence, 0);
        SFAlbumReset(album, &codepoints);
    }
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
