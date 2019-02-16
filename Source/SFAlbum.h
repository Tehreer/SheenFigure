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

#ifndef _SF_INTERNAL_ALBUM_H
#define _SF_INTERNAL_ALBUM_H

#include <SFAlbum.h>
#include <SFConfig.h>

#include "SFBase.h"
#include "SFCodepoints.h"
#include "List.h"

typedef enum {
    AlbumStateEmpty,
    AlbumStateFilling,
    AlbumStateFilled,
    AlbumStateArranging,
    AlbumStateArranged
} AlbumState;

enum {
    GlyphTraitNone        = 0 << 0,
    GlyphTraitPlaceholder = 1 << 0,     /**< BASIC: Insignificant, placeholder glyph. */
    GlyphTraitBase        = 1 << 1,     /**< BASIC: Single character, spacing glyph. */
    GlyphTraitLigature    = 1 << 2,     /**< BASIC: Multiple character, spacing glyph. */
    GlyphTraitMark        = 1 << 3,     /**< BASIC: Non-spacing combining glyph. */
    GlyphTraitComponent   = 1 << 4,     /**< BASIC: Part of single character, spacing glyph. */
    GlyphTraitSequence    = 1 << 5,     /**< BASIC: Multiple substituted, sequence glyph. */

    GlyphTraitAttached    = 1 << 8,     /**< HELPER: Attached with a previous glyph. */
    GlyphTraitCursive     = 1 << 9,     /**< HELPER: Cursively connected glyph. */
    GlyphTraitRightToLeft = 1 << 10,    /**< HELPER: Right-to-Left cursive glyph. */
    GlyphTraitResolved    = 1 << 11,    /**< HELPER: Resolved cursive glyph. */

    GlyphTraitZeroWidth   = 1 << 12     /**< CONTROL: Zero width, space glyph.*/
};
typedef SFUInt16 GlyphTraits;

typedef union {
    struct {
        SFUInt16 feature;
        SFUInt16 traits;
    } section;
    SFUInt32 full;
} GlyphMask;

typedef struct _GlyphDetail {
    SFUInteger association;     /**< Index of the code point to which the glyph maps. */
    GlyphMask mask;             /**< Mask of the glyph. */
    SFUInt16 cursiveOffset;     /**< Offset to the next cursively connected glyph. */
    SFUInt16 attachmentOffset;  /**< Offset to the previous glyph attached with this one. */
} GlyphDetail, *GlyphDetailRef;

typedef struct _SFAlbum {
    SFCodepointsRef codepoints;         /**< Code points to be shaped. */
    SFUInteger codeunitCount;           /**< Number of code units to process. */
    SFUInteger glyphCount;              /**< Total number of glyphs in the album. */

    SFBoolean isBackward;               /**< True for backward order of codepoints. */
    SFTextDirection renderingDirection; /**< The rendering direction of glyphs in the album. */

    LIST(SFUInteger) _indexMap;         /**< Code unit index to glyph index mapping list. */
    LIST(SFGlyphID) _glyphs;            /**< List of ids of all glyphs in the album. */
    LIST(GlyphDetail) _details;         /**< List of details of all glyphs in the album. */
    LIST(SFPoint) _offsets;             /**< List of offsets of all glyphs in the album. */
    LIST(SFAdvance) _advances;          /**< List of advances of all glyphs in the album. */

    SFUInteger _version;                /**< Current version of the album. */
    AlbumState _state;                  /**< Current state of the album. */

    SFUInteger _retainCount;
} SFAlbum;

SF_PRIVATE SFUInt16 GetAntiFeatureMask(SFUInt16 featureMask);

SF_INTERNAL void SFAlbumInitialize(SFAlbumRef album);

/**
 * Initializes the album for given code points.
 */
SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepointsRef codepoints, SFTextDirection renderingDirection);

/**
 * Starts filling the album with provided glyphs.
 */
SF_INTERNAL void SFAlbumBeginFilling(SFAlbumRef album);

SF_INTERNAL SFUInteger *SFAlbumGetTemporaryIndexArray(SFAlbumRef album, SFUInteger count);

/**
 * Adds a new glyph into the album.
 */
SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, GlyphTraits traits, SFUInteger association);

/**
 * Reserves specified number of glyphs at the given index.
 * @note
 *      The reserved glyphs will be uninitialized.
 */
SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count);

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph);

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association);

SF_PRIVATE GlyphMask SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index);

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask);

SF_INTERNAL GlyphTraits SFAlbumGetAllTraits(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAllTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits);
SF_INTERNAL void SFAlbumReplaceBasicTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits);

/**
 * Ends filling the album with glyphs.
 */
SF_INTERNAL void SFAlbumEndFilling(SFAlbumRef album);

/**
 * Begins arranging glyphs in the album at specified positions.
 */
SF_INTERNAL void SFAlbumBeginArranging(SFAlbumRef album);

SF_INTERNAL void SFAlbumInsertHelperTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits);
SF_INTERNAL void SFAlbumRemoveHelperTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits);

SF_INTERNAL SFInt32 SFAlbumGetX(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetX(SFAlbumRef album, SFUInteger index, SFInt32 x);

SF_INTERNAL SFInt32 SFAlbumGetY(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetY(SFAlbumRef album, SFUInteger index, SFInt32 y);

SF_INTERNAL SFAdvance SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index);
SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFAdvance advance);

#define SFAlbumAddX(album, index, pos) \
    SFAlbumSetX(album, index, SFAlbumGetX(album, index) + pos)
#define SFAlbumAddY(album, index, pos) \
    SFAlbumSetY(album, index, SFAlbumGetY(album, index) + pos)
#define SFAlbumAddAdvance(album, index, advance) \
    SFAlbumSetAdvance(album, index, SFAlbumGetAdvance(album, index) + advance)

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
