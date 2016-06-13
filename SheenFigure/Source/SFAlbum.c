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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"
#include "SFAlbum.h"

static const SFGlyphMask _SFGlyphMaskEmpty = { { SFUInt16Max, 0 } };
static const SFGlyphMask _SFGlyphMaskPlaceholder = { { SFUInt16Max, SFGlyphTraitPlaceholder } };

static void _SFAlbumSetGlyphMask(SFAlbumRef album, SFUInteger index, SFGlyphMask glyphMask);
static void _SFAlbumRemoveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count);
static void _SFAlbumRemovePlaceholders(SFAlbumRef album);
static void _SFAlbumBuildCodeunitToGlyphMap(SFAlbumRef album);

SF_PRIVATE SFUInt16 _SFAlbumGetAntiFeatureMask(SFUInt16 featureMask)
{
    /* The assumtion must NOT break that the feature mask will never be equal to default mask. */
    SFAssert(featureMask != _SFGlyphMaskEmpty.section.featureMask);

    return !featureMask ? ~_SFGlyphMaskEmpty.section.featureMask : ~featureMask;
}

SFAlbumRef SFAlbumCreate(void)
{
    SFAlbumRef album = malloc(sizeof(SFAlbum));
    SFAlbumInitialize(album);

    return album;
}

SFUInteger SFAlbumGetCodeunitCount(SFAlbumRef album)
{
    return album->stringRange.count;
}

SFUInteger SFAlbumGetGlyphCount(SFAlbumRef album)
{
    return album->glyphCount;
}

const SFGlyphID *SFAlbumGetGlyphIDsPtr(SFAlbumRef album)
{
    return album->_glyphs.items;
}

const SFPoint *SFAlbumGetGlyphOffsetsPtr(SFAlbumRef album)
{
    return album->_offsets.items;
}

const SFAdvance *SFAlbumGetGlyphAdvancesPtr(SFAlbumRef album)
{
    return album->_advances.items;
}

const SFUInteger *SFAlbumGetCodeunitToGlyphMapPtr(SFAlbumRef album)
{
    return album->_mapArray;
}

SFAlbumRef SFAlbumRetain(SFAlbumRef album)
{
    if (album) {
        album->_retainCount++;
    }

    return album;
}

void SFAlbumRelease(SFAlbumRef album)
{
    if (album && --album->_retainCount == 0) {
        SFAlbumFinalize(album);
    }
}

SF_INTERNAL void SFAlbumInitialize(SFAlbumRef album)
{
    album->codepoints = NULL;
    album->stringRange = SFRangeEmpty;
    album->glyphCount = 0;
    album->_mapArray = NULL;

    SFListInitialize(&album->_associates, sizeof(SFUInteger));
    SFListInitialize(&album->_glyphs, sizeof(SFGlyphID));
    SFListInitialize(&album->_details, sizeof(SFGlyphDetail));
    SFListInitialize(&album->_offsets, sizeof(SFPoint));
    SFListInitialize(&album->_advances, sizeof(SFAdvance));

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
    album->_retainCount = 1;
}

SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepointsRef codepoints, SFRange stringRange)
{
    free(album->_mapArray);

    album->codepoints = codepoints;
    album->stringRange = stringRange;
    album->glyphCount = 0;
    album->_mapArray = NULL;

    SFListClear(&album->_associates);
    SFListClear(&album->_glyphs);
    SFListClear(&album->_details);
    SFListClear(&album->_offsets);
    SFListClear(&album->_advances);

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
}

SF_INTERNAL void SFAlbumBeginFilling(SFAlbumRef album)
{
    SFUInteger associatesCapacity = album->stringRange.count >> 1;
	SFUInteger glyphCapacity = album->stringRange.count << 1;

    SFListReserveRange(&album->_associates, 0, associatesCapacity);
    SFListReserveRange(&album->_glyphs, 0, glyphCapacity);
    SFListReserveRange(&album->_details, 0, glyphCapacity);

	album->_state = _SFAlbumStateFilling;
}

SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFUInteger association, SFUInteger length)
{
    SFUInteger index;

    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_version++;
    index = album->glyphCount++;

    /* Initialize the glyph along with its details. */
    SFAlbumSetGlyph(album, index, glyph);
    _SFAlbumSetGlyphMask(album, index, _SFGlyphMaskEmpty);
    SFAlbumSetSingleAssociation(album, index, association);

    while (--length) {
        index = album->glyphCount++;

        /* Initialize placeholder glyph along with its details. */
        SFAlbumSetGlyph(album, index, 0);
        _SFAlbumSetGlyphMask(album, index, _SFGlyphMaskPlaceholder);
        SFAlbumSetSingleAssociation(album, index, association);
    }
}

SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_version++;
    album->glyphCount += count;

    SFListReserveRange(&album->_glyphs, index, count);
    SFListReserveRange(&album->_details, index, count);
}

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index)
{
    return SFListGetVal(&album->_glyphs, index);
}

SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListSetVal(&album->_glyphs, index, glyph);
}

SF_INTERNAL SFUInteger SFAlbumGetSingleAssociation(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->association;
}

SF_INTERNAL void SFAlbumSetSingleAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The glyph must not be composite. */
    SFAssert(!(SFAlbumGetTraits(album, index) & SFGlyphTraitComposite));

    SFListGetRef(&album->_details, index)->association = association;
}

SF_INTERNAL SFUInteger *SFAlbumGetCompositeAssociations(SFAlbumRef album, SFUInteger index, SFUInteger *outCount)
{
    SFUInteger association;
    SFUInteger *array;

    /* The glyph must be composite. */
    SFAssert(SFAlbumGetTraits(album, index) & SFGlyphTraitComposite);

    association = SFListGetRef(&album->_details, index)->association;
    array = SFListGetRef(&album->_associates, association);
    *outCount = array[0];

    return &array[1];
}

SF_INTERNAL SFUInteger *SFAlbumMakeCompositeAssociations(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    SFUInteger association;
    SFUInteger reference;
    SFUInteger *array;

    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The glyph must be composite. */
    SFAssert(SFAlbumGetTraits(album, index) & SFGlyphTraitComposite);

    association = album->_associates.count;
    SFListAdd(&album->_associates, count);
    SFListGetRef(&album->_details, index)->association = association;

    reference = album->_associates.count;
    SFListReserveRange(&album->_associates, reference, count);
    array = SFListGetRef(&album->_associates, reference);

    return array;
}

SF_PRIVATE SFGlyphMask _SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->mask;
}

static void _SFAlbumSetGlyphMask(SFAlbumRef album, SFUInteger index, SFGlyphMask glyphMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListGetRef(&album->_details, index)->mask = glyphMask;
}

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->mask.section.featureMask;
}

SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListGetRef(&album->_details, index)->mask.section.featureMask = featureMask;
}

SF_INTERNAL SFGlyphTraits SFAlbumGetTraits(SFAlbumRef album, SFUInteger index)
{
    return (SFGlyphTraits)SFListGetRef(&album->_details, index)->mask.section.glyphTraits;
}

SF_INTERNAL void SFAlbumSetTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits = (SFUInt16)traits;
}

SF_INTERNAL void SFAlbumInsertTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits |= (SFUInt16)traits;
}

SF_INTERNAL void SFAlbumRemoveTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits &= (SFUInt16)~traits;
}

SF_INTERNAL void SFAlbumEndFilling(SFAlbumRef album)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_state = _SFAlbumStateFilled;
}

SF_INTERNAL void SFAlbumBeginArranging(SFAlbumRef album)
{
    /* The album must be filled before arranging it. */
    SFAssert(album->_state == _SFAlbumStateFilled);

    SFListReserveRange(&album->_offsets, 0, album->glyphCount);
    SFListReserveRange(&album->_advances, 0, album->glyphCount);

    album->_state = _SFAlbumStateArranging;
}

SF_INTERNAL SFInteger SFAlbumGetX(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_offsets, index)->x;
}

SF_INTERNAL void SFAlbumSetX(SFAlbumRef album, SFUInteger index, SFInteger x)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_offsets, index)->x = x;
}

SF_INTERNAL SFInteger SFAlbumGetY(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_offsets, index)->y;
}

SF_INTERNAL void SFAlbumSetY(SFAlbumRef album, SFUInteger index, SFInteger y)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_offsets, index)->y = y;
}

SF_INTERNAL SFAdvance SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index)
{
    return SFListGetVal(&album->_advances, index);
}

SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFAdvance advance)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    SFListSetVal(&album->_advances, index, advance);
}

SF_INTERNAL SFUInt16 SFAlbumGetCursiveOffset(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->cursiveOffset;
}

SF_INTERNAL void SFAlbumSetCursiveOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_details, index)->cursiveOffset = offset;
}

SF_INTERNAL SFUInt16 SFAlbumGetAttachmentOffset(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->attachmentOffset;
}

SF_INTERNAL void SFAlbumSetAttachmentOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    SFListGetRef(&album->_details, index)->attachmentOffset = offset;
}

SF_INTERNAL void SFAlbumEndArranging(SFAlbumRef album)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    album->_state = _SFAlbumStateArranged;
}

static void _SFAlbumRemoveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    SFListRemoveRange(&album->_glyphs, index, count);
    SFListRemoveRange(&album->_details, index, count);
    SFListRemoveRange(&album->_offsets, index, count);
    SFListRemoveRange(&album->_advances, index, count);
}

static void _SFAlbumRemovePlaceholders(SFAlbumRef album)
{
    SFUInteger placeholderCount = 0;
    SFUInteger index = album->glyphCount;

    while (index--) {
        SFGlyphTraits traits = SFAlbumGetTraits(album, index);
        if (traits & SFGlyphTraitPlaceholder) {
            placeholderCount++;
        } else {
            if (placeholderCount) {
                _SFAlbumRemoveGlyphs(album, index + 1, placeholderCount);
                album->glyphCount -= placeholderCount;
                placeholderCount = 0;
            }
        }
    }

    if (placeholderCount) {
        _SFAlbumRemoveGlyphs(album, 0, placeholderCount);
        album->glyphCount -= placeholderCount;
    }
}

static void _SFAlbumBuildCodeunitToGlyphMap(SFAlbumRef album)
{
    SFUInteger stringStart = album->stringRange.start;
    SFUInteger index = album->glyphCount;
    SFUInteger *map;

    map = malloc(sizeof(SFUInteger) * album->stringRange.count);

    /* Traverse in reverse order so that first glyph takes priority in case of multiple substitution. */
    while (index--) {
        SFGlyphTraits traits = SFAlbumGetTraits(album, index);
        SFUInteger association;

        if (traits & SFGlyphTraitComposite) {
            SFUInteger count;
            SFUInteger *array;
            SFUInteger j;

            array = SFAlbumGetCompositeAssociations(album, index, &count);

            for (j = 0; j < count; j++) {
                association = array[j] - stringStart;
                map[association] = index;
            }
        } else {
            association = SFAlbumGetSingleAssociation(album, index) - stringStart;
            map[association] = index;
        }
    }

    album->_mapArray = map;
}

SF_INTERNAL void SFAlbumWrapUp(SFAlbumRef album)
{
    /* The album must be in completed state before wrapping up. */
    SFAssert(album->_state == _SFAlbumStateFilled || album->_state == _SFAlbumStateArranged);

    _SFAlbumRemovePlaceholders(album);
    _SFAlbumBuildCodeunitToGlyphMap(album);

    album->codepoints = NULL;
}

SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album) {
    SFListFinalize(&album->_associates);
    SFListFinalize(&album->_glyphs);
    SFListFinalize(&album->_details);
    SFListFinalize(&album->_offsets);
    SFListFinalize(&album->_advances);
}
