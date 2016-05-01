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

#ifndef _SF_ARTIST_H
#define _SF_ARTIST_H

#include "SFAlbum.h"
#include "SFPattern.h"
#include "SFTypes.h"

enum {
    SFTextDirectionDefault = 0,
    SFTextDirectionLeftToRight = 1,
    SFTextDirectionRightToLeft = 2
};
typedef SFUInt32 SFTextDirection;

enum {
    SFTextModeForward = 0,
    SFTextModeBackward = 1
};
typedef SFUInt32 SFTextMode;

struct _SFArtist;
typedef struct _SFArtist SFArtist;
/**
 * The type used to represent a sheen figure artist.
 */
typedef SFArtist *SFArtistRef;

SFArtistRef SFArtistCreate(void);

void SFArtistSetPattern(SFArtistRef artist, SFPatternRef pattern);

void SFArtistSetCodepoints(SFArtistRef artist, SFCodepoint *codepoints, SFUInteger length);

/**
 * Sets the direction of text for glyph positioning.
 *
 * The value of textDirection must reflect the script direction of input text so that individual
 * glyphs are placed at appropriate locations while rendering. It should not be confused with bidi
 * run direction as that may not reflect the script direction if overridden explicitly.
 *
 * @param artist
 *      The artist to modify.
 * @param textDirection
 *      A value of SFTextDirection.
 *
 */
void SFArtistSetTextDirection(SFArtistRef artist, SFTextDirection textDirection);

/**
 * Sets the mode for text processing.
 *
 * This method provides a convinient way of shaping a bidi run whose direction is opposite to that
 * of script. For example, if the direction of a run, 'car' is explicitly set as right-to-left,
 * backward mode will automatically treat it as 'rac' without reordering the original text.
 *
 * @param artist
 *      The artist to modify.
 * @param textMode
 *      A value of SFTextMode.
 */
void SFArtistSetTextMode(SFArtistRef artist, SFTextMode textMode);

/**
 * Shapes the text with appropriate shaping engine, filling the album with glyph infos.
 *
 * The output glyphs in the album flow in logical direction as described by text flow. For
 * left-to-right text flow, the x position of pen is incremented with glyph's advance after
 * rendering it. Similarly, for right-to-left text flow, the x position of pen is decremented with
 * glyph's advance after rendering it.
 *
 * @param artist
 *      The artist to be used for shaping.
 * @param album
 *      The album that should be filled with shaping results.
 */
void SFArtistFillAlbum(SFArtistRef artist, SFAlbumRef album);

SFArtistRef SFArtistRetain(SFArtistRef artist);
void SFArtistRelease(SFArtistRef artists);

#endif
