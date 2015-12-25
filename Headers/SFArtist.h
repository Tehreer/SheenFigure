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

#ifndef SF_ARTIST_H
#define SF_ARTIST_H

#include "SFAlbum.h"
#include "SFPattern.h"
#include "SFTypes.h"

struct _SFArtist;
typedef struct _SFArtist SFArtist;
/**
 * The type used to represent a sheen figure artist.
 */
typedef SFArtist *SFArtistRef;

SFArtistRef SFArtistCreate(void);

void SFArtistSetTextAsCodepoints(SFArtistRef artist, SFCodepoint *codepoints, SFUInteger length);
void SFArtistSetPattern(SFArtistRef artist, SFPatternRef pattern);

/**
 * Shapes the provided text range with appropriate shaping engine and fills the album with shaping
 * results such as glyph ids, positions and advances.
 * @param artist
 *      The artist to be used for shaping the text.
 * @param album
 *      The album that should be filled with shaping results. If the album is not empty, it will be
 *      cleared first.
 * @param index
 *      The index of the text from where shaping should be started.
 * @param count
 *      The number of text elements that are intended to be shaped.
 */
void SFArtistFillAlbum(SFArtistRef artist, SFAlbumRef album, SFUInteger index, SFUInteger count);

SFArtistRef SFArtistRetain(SFArtistRef artist);
void SFArtistRelease(SFArtistRef artists);

#endif
