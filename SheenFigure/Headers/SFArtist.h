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
#include "SFBase.h"
#include "SFPattern.h"

/**
 * The type used to represent an open type artist.
 */
typedef struct _SFArtist *SFArtistRef;

SFArtistRef SFArtistCreate(void);

/**
 * Sets the pattern which an artist will use while shaping.
 *
 * @param artist
 *      The artist for which to set the pattern.
 * @param pattern
 *      A pattern that should be used while shaping.
 */
void SFArtistSetPattern(SFArtistRef artist, SFPatternRef pattern);

/**
 * Sets the source string which an artist will shape.
 *
 * @param artist
 *      The artist for which to set the string.
 * @param stringEncoding
 *      The encoding of the string.
 * @param stringBuffer
 *      A buffer that contains the code units of the string.
 * @param stringLength
 *      The length of the string in terms of code units.
 */
void SFArtistSetString(SFArtistRef artist, SFStringEncoding stringEncoding, void *stringBuffer, SFUInteger stringLength);

/**
 * Sets the text direction which an artist will use while shaping.
 *
 * The value of textDirection must reflect the script direction of source string so that each glyph
 * is placed at appropriate location with respect to that script. It should not be confused with the
 * direction of a bidirectional run, as that may not reflect the script direction if overridden
 * explicitly.
 *
 * @param artist
 *      The artist for which to set the text direction.
 * @param textDirection
 *      A value of SFTextDirection.
 */
void SFArtistSetTextDirection(SFArtistRef artist, SFTextDirection textDirection);

/**
 * Sets the text mode which an artist will use while shaping.
 *
 * This method provides a convinient way of shaping a bidirectional run whose direction is opposite
 * to that of script. For example, if the direction of a run, 'car' is explicitly set as
 * right-to-left, backward text mode will automatically read it as 'rac' without reordering and
 * making a copy of the original string.
 *
 * @param artist
 *      The artist for which to set the text mode.
 * @param textMode
 *      A value of SFTextMode.
 */
void SFArtistSetTextMode(SFArtistRef artist, SFTextMode textMode);

/**
 * Shapes the source string with an appropriate shaping engine, filling the album with shaping
 * results. The album is cleared first, if not empty.
 *
 * The output glyphs in the album flow in logical text direction. For left-to-right direction, the
 * position of pen should be incremented with glyph's advance after rendering it. Similarly, for
 * right-to-left direction, the position of pen should be decremented with glyph's advance after
 * rendering it.
 *
 * @param artist
 *      The artist to use for shaping.
 * @param album
 *      The album that should be filled with shaping results.
 */
void SFArtistFillAlbum(SFArtistRef artist, SFAlbumRef album);

SFArtistRef SFArtistRetain(SFArtistRef artist);
void SFArtistRelease(SFArtistRef artists);

#endif
