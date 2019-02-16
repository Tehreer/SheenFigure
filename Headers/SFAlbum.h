/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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
 * Returns the direction in which the glyphs must be rendered.
 *
 * @param album
 *      The album whose rendering direction is returned.
 * @return
 *      The direction in which the glyphs must be rendered.
 */
SFTextDirection SFAlbumGetRenderingDirection(SFAlbumRef album);

/**
 * Returns the text flow direction in which the caret navigates.
 *
 * @param album
 *      The album whose caret direction is returned.
 * @return
 *      The text flow direction in which the caret navigates.
 */
SFTextDirection SFAlbumGetCaretDirection(SFAlbumRef album);

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
 * - If nonconsecutive code units translate to a single glyph or ligature, then each participating
 *   code unit, and all in-between code units, map to this glyph or ligature.
 *
 * @param album
 *      The album from which to access the glyph map.
 * @return
 *      A valid pointer to an array of glyph map.
 */
const SFUInteger *SFAlbumGetCodeunitToGlyphMapPtr(SFAlbumRef album);

/**
 * Loads all caret edges into a user-provided buffer.
 *
 * @param album
 *      The album from which to load the caret edges.
 * @param caretStops
 *      A buffer of SFBoolean values that identifies the caret stops of code units represented by
 *      the passed-in album. The NULL value indicates that there is caret stop on every code unit.
 * @param advanceScale
 *      A scale value that would be applied on glyph advances and subsequently on caret edges.
 * @param caretEdges
 *      A SFFloat buffer into which caret edges are loaded. The length of buffer must be one greater
 *      than the total number of code units represented by the passed-in album.
 * @return
 *      The total sum of the advances of all glyphs.
 */
SFFloat SFAlbumLoadCaretEdges(SFAlbumRef album, SFBoolean *caretStops, SFFloat advanceScale, SFFloat *caretEdges);

SFAlbumRef SFAlbumRetain(SFAlbumRef album);
void SFAlbumRelease(SFAlbumRef album);

#endif
