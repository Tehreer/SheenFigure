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

#include "SFAssert.h"
#include "SFGlyphTraits.h"
#include "SFList.h"

#ifndef NDEBUG
#define SF_SAFE_ALBUM
#endif

typedef enum {
    _SFAlbumStateEmpty,
    _SFAlbumStateFilling,
    _SFAlbumStateFilled,
    _SFAlbumStateArranging,
    _SFAlbumStateArranged
} _SFAlbumState;

typedef struct _SFGlyphDetail {
    SFUInteger association;     /**< Index of the code point to which the glyph maps. */
    SFGlyphTraits traits;       /**< Traits of the glyph. */
    /**
     * Offset to the next right-to-left cursively connected element.
     */
    SFUInt16 offset;
} SFGlyphDetail, *SFGlyphDetailRef;

struct _SFAlbum {
    const SFCodepoint *codePointArray; /**< The array of codepoints which are to be shaped. */
    SFUInteger *mapArray;
    SFUInteger codePointCount;
    SFUInteger glyphCount;              /**< Total number of glyphs in the album. */

    SF_LIST(SFGlyphID) _glyphs;         /**< List of ids of all glyphs in the album. */
    SF_LIST(SFGlyphDetail) _details;    /**< List of details of all glyphs in the album. */
    SF_LIST(SFPoint) _positions;        /**< List of positions of all glyphs in the album. */
    SF_LIST(SFInteger) _advances;       /**< List of advances of all glyphs in the album. */

    SFUInteger _version;                /**< Current version of the album. */
    _SFAlbumState _state;               /**< Current state of the album. */

    SFUInteger _retainCount;
};

SF_INTERNAL void SFAlbumInitialize(SFAlbumRef album);

/**
 * Initializes the album for given code points.
 */
SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepoint *codePointArray, SFUInteger codePointCount);

/**
 * Allocates an array for charater to glyph map.
 * @note
 *      The allocated map will be uninitialized.
 */
SF_INTERNAL void SFAlbumAllocateMap(SFAlbumRef album);

/**
 * Starts filling the album with provided glyphs.
 */
SF_INTERNAL void SFAlbumStartFilling(SFAlbumRef album);

/**
 * Adds a new glyph into the album.
 */
SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFUInteger association);

/**
 * Reserves specified number of glyphs at the given index.
 * @note
 *      The reserved glyphs will be uninitialized.
 */
SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count);

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph);

SF_INTERNAL SFGlyphTraits SFAlbumGetTraits(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits);

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association);

/**
 * Sops filling the album with glyphs.
 */
SF_INTERNAL void SFAlbumStopFilling(SFAlbumRef album);

/**
 * Starts arranging glyphs in the album at specified positions.
 */
SF_INTERNAL void SFAlbumStartArranging(SFAlbumRef album);

SF_INTERNAL SFPoint SFAlbumGetPosition(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetPosition(SFAlbumRef album, SFUInteger index, SFPoint position);

SF_INTERNAL SFInteger SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFInteger advance);

SF_INTERNAL SFUInt16 SFAlbumGetOffset(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset);

/**
 * Stops arranging glyphs in the album.
 */
SF_INTERNAL void SFAlbumStopArranging(SFAlbumRef album);

/**
 * Finalizes the album.
 */
SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album);

#endif
