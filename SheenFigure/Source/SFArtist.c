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

#include <SBCodepointSequence.h>
#include <stddef.h>
#include <stdlib.h>

#include "SFBase.h"
#include "SFUnifiedEngine.h"
#include "SFArtist.h"

SFArtistRef SFArtistCreate(void)
{
    SFArtistRef artist = malloc(sizeof(SFArtist));
    artist->codepointSequence = NULL;
    artist->pattern = NULL;
    artist->textDirection = SFTextDirectionLeftToRight;
    artist->textMode = SFTextModeForward;
    artist->_retainCount = 1;

    return artist;
}

SFTextDirection SFArtistGetDefaultDirectionForScript(SFArtistRef artist, SFTag scriptTag)
{
    SFScriptKnowledgeRef knowledge = SFShapingKnowledgeSeekScript(&SFUnifiedKnowledgeInstance, scriptTag);
    if (knowledge != NULL) {
        return knowledge->defaultDirection;
    }

    return SFTextModeForward;
}

void SFArtistSetString(SFArtistRef artist, SFStringEncoding stringEncoding, const void *stringBuffer, SFUInteger stringLength)
{
    SBCodepointSequenceRelease(artist->codepointSequence);

    switch (stringEncoding) {
        case SFStringEncodingUTF8:
            artist->codepointSequence = SBCodepointSequenceCreateWithUTF8String(stringBuffer, stringLength);
            break;

        case SFStringEncodingUTF16:
            artist->codepointSequence = SBCodepointSequenceCreateWithUTF16String(stringBuffer, stringLength);
            break;

        case SFStringEncodingUTF32:
            artist->codepointSequence = SBCodepointSequenceCreateWithUTF32String(stringBuffer, stringLength);
            break;

        default:
            artist->codepointSequence = NULL;
            break;
    }
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
    if (artist->pattern && artist->codepointSequence) {
        SFCodepoints codepoints;
        SFUnifiedEngine unifiedEngine;
        SFShapingEngineRef shapingEngine;

        SFCodepointsInitialize(&codepoints,
                               artist->codepointSequence,
                               artist->textMode == SFTextModeBackward);

        SFUnifiedEngineInitialize(&unifiedEngine, artist);
        shapingEngine = (SFShapingEngineRef)&unifiedEngine;

        SFAlbumReset(album, &codepoints, SBCodepointSequenceGetStringLength(artist->codepointSequence));
        SFShapingEngineProcessAlbum(shapingEngine, album);
    } else {
        SFAlbumReset(album, NULL, 0);
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
        SBCodepointSequenceRelease(artist->codepointSequence);
        free(artist);
    }
}
