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
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SFAssert.h"
#include "SFAlbum.h"

static const SFGlyphMask _SFGlyphMaskEmpty = { { SFUInt16Max, 0 } };

static void SFAlbumSetGlyphMask(SFAlbumRef album, SFUInteger index, SFGlyphMask glyphMask);

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

SFUInteger SFAlbumGetGlyphCount(SFAlbumRef album)
{
    return album->glyphCount;
}

SFGlyphID *SFAlbumGetGlyphIDs(SFAlbumRef album)
{
    return album->_glyphs.items;
}

SFPoint *SFAlbumGetGlyphPositions(SFAlbumRef album)
{
    return album->_positions.items;
}

SFInteger *SFAlbumGetGlyphAdvances(SFAlbumRef album)
{
    return album->_advances.items;
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
    album->codePointArray = NULL;
    album->mapArray = NULL;
    album->codePointCount = 0;
    album->glyphCount = 0;

    SFListInitialize(&album->_glyphs, sizeof(SFGlyphID));
    SFListInitialize(&album->_details, sizeof(SFGlyphDetail));
    SFListInitialize(&album->_positions, sizeof(SFPoint));
    SFListInitialize(&album->_advances, sizeof(SFInteger));

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
    album->_retainCount = 1;
}

SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepoint *codePointArray, SFUInteger codePointCount)
{
	/* There must be some code points. */
	SFAssert(codePointArray != NULL && codePointCount > 0);

    album->codePointArray = codePointArray;
	album->mapArray = NULL;
    album->codePointCount = codePointCount;
    album->glyphCount = 0;

    SFListClear(&album->_glyphs);
    SFListClear(&album->_details);
    SFListClear(&album->_positions);
    SFListClear(&album->_advances);

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
}

SF_INTERNAL void SFAlbumStartFilling(SFAlbumRef album)
{
	SFUInteger capacity = album->codePointCount * 2;

    SFListReserveRange(&album->_glyphs, 0, capacity);
    SFListReserveRange(&album->_details, 0, capacity);

	album->_state = _SFAlbumStateFilling;
}

SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFUInteger association) {
    SFUInteger index;

    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_version++;
    index = album->glyphCount++;

    /* Initialize glyph along with its details. */
    SFAlbumSetGlyph(album, index, glyph);
    SFAlbumSetGlyphMask(album, index, _SFGlyphMaskEmpty);
    SFAlbumSetAssociation(album, index, association);
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
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetVal(&album->_glyphs, index);
}

SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListSetVal(&album->_glyphs, index, glyph);
}

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetRef(&album->_details, index)->association;
}

SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->association = association;
}

SF_PRIVATE SFGlyphMask _SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->mask;
}

static void SFAlbumSetGlyphMask(SFAlbumRef album, SFUInteger index, SFGlyphMask glyphMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->mask = glyphMask;
}

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetRef(&album->_details, index)->mask.section.featureMask;
}

SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->mask.section.featureMask = featureMask;
}

SF_INTERNAL SFGlyphTraits SFAlbumGetTraits(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return (SFGlyphTraits)SFListGetRef(&album->_details, index)->mask.section.glyphTraits;
}

SF_INTERNAL void SFAlbumSetTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits = (SFUInt16)traits;
}

SF_INTERNAL void SFAlbumInsertTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits |= (SFUInt16)traits;
}

SF_INTERNAL void SFAlbumRemoveTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be either in filling state or arranging state. */
    SFAssert(album->_state == _SFAlbumStateFilling || album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->mask.section.glyphTraits &= (SFUInt16)~traits;
}

SF_INTERNAL void SFAlbumStopFilling(SFAlbumRef album)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_state = _SFAlbumStateFilled;
}

SF_INTERNAL void SFAlbumStartArranging(SFAlbumRef album)
{
    /* The album must be filled before arranging it. */
    SFAssert(album->_state == _SFAlbumStateFilled);

    SFListReserveRange(&album->_positions, 0, album->glyphCount);
    SFListReserveRange(&album->_advances, 0, album->glyphCount);

    album->_state = _SFAlbumStateArranging;
}

SF_INTERNAL SFPoint SFAlbumGetPosition(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetVal(&album->_positions, index);
}

SF_INTERNAL void SFAlbumSetPosition(SFAlbumRef album, SFUInteger index, SFPoint position)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListSetVal(&album->_positions, index, position);
}

SF_INTERNAL SFInteger SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetVal(&album->_advances, index);
}

SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFInteger advance)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListSetVal(&album->_advances, index, advance);
}

SF_INTERNAL SFUInt16 SFAlbumGetOffset(SFAlbumRef album, SFUInteger index)
{
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    return SFListGetRef(&album->_details, index)->offset;
}

SF_INTERNAL void SFAlbumSetOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);
    /* The index must be valid. */
    SFAssert(index < album->glyphCount);

    SFListGetRef(&album->_details, index)->offset = offset;
}

SF_INTERNAL void SFAlbumStopArranging(SFAlbumRef album)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);

    album->_state = _SFAlbumStateArranged;
}

SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album) {
    SFListFinalize(&album->_glyphs);
    SFListFinalize(&album->_details);
    SFListFinalize(&album->_positions);
    SFListFinalize(&album->_advances);
}
