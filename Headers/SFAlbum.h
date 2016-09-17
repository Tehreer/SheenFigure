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

#ifndef _SF_PUBLIC_ALBUM_H
#define _SF_PUBLIC_ALBUM_H

#include "SFBase.h"

/**
 * The type used to represent an open type album.
 */
typedef struct _SFAlbum *SFAlbumRef;

/**
 * Creates an instance of an open type album.
 *
 * @return
 *      A reference to an album object. 
 */
SFAlbumRef SFAlbumCreate(void);

/**
 * Returns the number of code units processed by the shaping engine.
 *
 * @param album
 *      The album for which to return the number of code units.
 * @return
 *      The number of code units analyzed by the shaping engine.
 */
SFUInteger SFAlbumGetCodeunitCount(SFAlbumRef album);

/**
 * Returns the number of glyphs produced by the shaping engine.
 *
 * @param album
 *      The album for which to return the number of glyphs.
 * @return
 *      The number of glyphs produced by the shaping engine.
 */
SFUInteger SFAlbumGetGlyphCount(SFAlbumRef album);

/**
 * Returns a direct pointer for the glyph IDs produced by the shaping engine.
 *
 * @param album
 *      The album from which to access the glyph IDs.
 * @return
 *      A valid pointer to an array of glyph IDs.
 */
const SFGlyphID *SFAlbumGetGlyphIDsPtr(SFAlbumRef album);

/**
 * Returns a direct pointer for the glyph offsets produced by the shaping engine. Each offset
 * describes the position of a glyph with respect to zero origin.
 *
 * @param album
 *      The album from which to access the glyph offsets.
 * @return
 *      A valid pointer to an array of SFPoint structures.
 */
const SFPoint *SFAlbumGetGlyphOffsetsPtr(SFAlbumRef album);

/**
 * Returns a direct pointer for the glyph advances produced by the shaping engine.
 *
 * @param album
 *      The album from which to access the glyph advances.
 * @return
 *      A valid pointer to an array of glyph advances.
 */
const SFAdvance *SFAlbumGetGlyphAdvancesPtr(SFAlbumRef album);

/**
 * Returns a direct pointer to an array of indexes, mapping each code unit in the source string to
 * corresponding glyph.
 *
 * The map is produced according to the following rules.
 * - If a single code unit translates into multiple glyphs, then it maps to the first glyph in the
 *   sequence.
 * - If multiple code units combine to make a single code point which in turn translates into
 *   multiple glyphs, then each code unit maps to the first glyph in the sequence.
 * - If multiple code units translate to a single glyph or ligature, then each participating code
 *   unit, whether consecutive or not, maps to this glyph or ligature.
 *
 * @param album
 *      The album from which to access the glyph map.
 * @return
 *      A valid pointer to an array of glyph map.
 */
const SFUInteger *SFAlbumGetCodeunitToGlyphMapPtr(SFAlbumRef album);

SFAlbumRef SFAlbumRetain(SFAlbumRef album);
void SFAlbumRelease(SFAlbumRef album);

#endif
