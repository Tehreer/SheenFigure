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

#ifndef _SF_INTERNAL_ALBUM_H
#define _SF_INTERNAL_ALBUM_H

#include <SFAlbum.h>
#include <SFConfig.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"
#include "SFList.h"

typedef enum {
    _SFAlbumStateEmpty,
    _SFAlbumStateFilling,
    _SFAlbumStateFilled,
    _SFAlbumStateArranging,
    _SFAlbumStateArranged
} _SFAlbumState;

enum {
    SFGlyphTraitNone        = 0 << 0,
    SFGlyphTraitPlaceholder = 1 << 0,   /**< HELPER: Insignificant, placeholder glyph. */
    SFGlyphTraitBase        = 1 << 1,   /**< STANDARD: Single character, spacing glyph. */
    SFGlyphTraitLigature    = 1 << 2,   /**< STANDARD: Multiple character, spacing glyph. */
    SFGlyphTraitMark        = 1 << 3,   /**< STANDARD: Non-spacing combining glyph. */
    SFGlyphTraitComponent   = 1 << 4,   /**< STANDARD: Part of single character, spacing glyph. */
    SFGlyphTraitAttached    = 1 << 5,   /**< HELPER: Attached with a previous glyph. */
    SFGlyphTraitCursive     = 1 << 6,   /**< HELPER: Cursively connected glyph. */
    SFGlyphTraitRightToLeft = 1 << 7,   /**< HELPER: Right-to-Left cursive glyph. */
    SFGlyphTraitResolved    = 1 << 8    /**< HELPER: Resolved cursive glyph. */
};
typedef SFUInt16 SFGlyphTraits;

typedef union {
    struct {
        SFUInt16 featureMask;
        SFUInt16 glyphTraits;
    } section;
    SFUInt32 full;
} SFGlyphMask;

typedef struct _SFGlyphDetail {
    SFUInteger association;     /**< Index of the code point to which the glyph maps. */
    SFGlyphMask mask;           /**< Mask of the glyph. */
    SFUInt16 cursiveOffset;     /**< Offset to the next cursively connected glyph. */
    SFUInt16 attachmentOffset;  /**< Offset to the previous glyph attached with this one. */
} SFGlyphDetail, *SFGlyphDetailRef;

typedef struct _SFAlbum {
    SFCodepointsRef codepoints;         /**< Code points to be shaped. */
    SFUInteger codeunitCount;           /**< Number of code units to process. */
    SFUInteger glyphCount;              /**< Total number of glyphs in the album. */

    SF_LIST(SFUInteger) _indexMap;      /**< Code unit index to glyph index mapping list. */
    SF_LIST(SFGlyphID) _glyphs;         /**< List of ids of all glyphs in the album. */
    SF_LIST(SFGlyphDetail) _details;    /**< List of details of all glyphs in the album. */
    SF_LIST(SFPoint) _offsets;          /**< List of offsets of all glyphs in the album. */
    SF_LIST(SFAdvance) _advances;       /**< List of advances of all glyphs in the album. */

    SFUInteger _version;                /**< Current version of the album. */
    _SFAlbumState _state;               /**< Current state of the album. */

    SFUInteger _retainCount;
} SFAlbum;

SF_PRIVATE SFUInt16 _SFAlbumGetAntiFeatureMask(SFUInt16 featureMask);

SF_INTERNAL void SFAlbumInitialize(SFAlbumRef album);

/**
 * Initializes the album for given code points.
 */
SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepointsRef codepoints, SFUInteger codeunitCount);

/**
 * Starts filling the album with provided glyphs.
 */
SF_INTERNAL void SFAlbumBeginFilling(SFAlbumRef album);

/**
 * Adds a new glyph into the album.
 */
SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFGlyphTraits traits, SFUInteger association);

/**
 * Reserves specified number of glyphs at the given index.
 * @note
 *      The reserved glyphs will be uninitialized.
 */
SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count);

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph);

SF_INTERNAL SFUInteger SFAlbumGetSingleAssociation(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetSingleAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association);

SF_PRIVATE SFGlyphMask _SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index);

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask);

SF_INTERNAL SFGlyphTraits SFAlbumGetTraits(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits);
SF_INTERNAL void SFAlbumInsertTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits);
SF_INTERNAL void SFAlbumRemoveTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits);

/**
 * Ends filling the album with glyphs.
 */
SF_INTERNAL void SFAlbumEndFilling(SFAlbumRef album);

/**
 * Begins arranging glyphs in the album at specified positions.
 */
SF_INTERNAL void SFAlbumBeginArranging(SFAlbumRef album);

SF_INTERNAL SFInt32 SFAlbumGetX(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetX(SFAlbumRef album, SFUInteger index, SFInt32 x);

SF_INTERNAL SFInt32 SFAlbumGetY(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetY(SFAlbumRef album, SFUInteger index, SFInt32 y);

SF_INTERNAL SFAdvance SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFAdvance advance);

SF_INTERNAL SFUInt16 SFAlbumGetCursiveOffset(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetCursiveOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset);

SF_INTERNAL SFUInt16 SFAlbumGetAttachmentOffset(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAttachmentOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset);

/**
 * Ends arranging glyphs in the album.
 */
SF_INTERNAL void SFAlbumEndArranging(SFAlbumRef album);

/**
 * Wraps up the album for client's usage.
 */
SF_INTERNAL void SFAlbumWrapUp(SFAlbumRef album);

/**
 * Finalizes the album.
 */
SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album);

#endif
