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

#ifndef _SF_ALBUM_INTERNAL_H
#define _SF_ALBUM_INTERNAL_H

#include <SFAlbum.h>
#include <SFConfig.h>
#include <SFTypes.h>

#include "SFList.h"

struct _SFGlyphDetail;
typedef struct _SFGlyphDetail SFGlyphDetail;
typedef SFGlyphDetail *SFGlyphDetailRef;

enum {
    SFGlyphTraitNone = 0,
    SFGlyphTraitBase = 1,
    SFGlyphTraitLigature = 2,
    SFGlyphTraitMark = 3,
    SFGlyphTraitComponent = 4,
    SFGlyphTraitMaster = 8,
    /**
     * Indicates that the glyph has been removed during substitution process;
     * possibly due to ligature substitution.
     */
    SFGlyphTraitRemoved = 16,
    /**
     * Indicates that the glyph is to be treated as right-to-left for cursive
     * attachments.
     */
    SFGlyphTraitRightToLeft = 32
};
typedef SFUInt32 SFGlyphTrait;

struct _SFGlyphDetail {
    SFUInteger association;    /**< Index of the code point to which the glyph maps. */
    SFGlyphTrait traits;       /**< Traits of the glyph. */
    /**
     * Offset to the next right-to-left cursively connected element.
     */
    SFUInt16 offset;
};

struct _SFAlbum {
    const SFCodepoint *codePointArray; /**< The array of codepoints which are to be shaped. */
    SFUInteger *mapArray;
    SFUInteger codePointCount;
    SFUInteger glyphCount;

    SF_LIST(SFGlyphID) _glyphs;
    SF_LIST(SFGlyphDetail) _details;
    SF_LIST(SFPoint) _positions;
    SF_LIST(SFInteger) _advances;

    SFUInteger _retainCount;
};

/**
 * Initializes the album for given code points.
 */
SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepoint *codePointArray, SFUInteger codePointCount);

SF_INTERNAL void SFAlbumAllocateGlyphs(SFAlbumRef album);
SF_INTERNAL void SFAlbumAllocatePositions(SFAlbumRef album);

SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFUInteger association);

/**
 * Reserves specified number of glyphs at the given index.
 * @note
 *      The reserved glyphs will be uninitialized.
 */
SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count);

/**
 * Allocates an array for charater to glyph map.
 * @note
 *      The allocated map will be uninitialized.
 */
SF_INTERNAL void SFAlbumAllocateMap(SFAlbumRef album);

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph);

SF_INTERNAL SFGlyphTrait SFAlbumGetTraits(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetTraits(SFAlbumRef album, SFUInteger index, SFGlyphTrait trait);

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association);

SF_INTERNAL SFPoint SFAlbumGetPosition(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetPosition(SFAlbumRef album, SFUInteger index, SFPoint position);

SF_INTERNAL SFInteger SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFInteger advance);

SF_INTERNAL SFUInt16 SFAlbumGetOffset(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset);

/**
 * Finalizes the album.
 */
SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album);

#endif
