/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

SF_PRIVATE SFUInt16 _SFAlbumGetAntiFeatureMask(SFUInt16 featureMask)
{
    /* The assumtion must NOT break that the feature mask will never be equal to default mask. */
    SFAssert(featureMask != _SFGlyphMaskEmpty.section.feature);

    return !featureMask ? ~_SFGlyphMaskEmpty.section.feature : ~featureMask;
}

SFAlbumRef SFAlbumCreate(void)
{
    SFAlbumRef album = malloc(sizeof(SFAlbum));
    SFAlbumInitialize(album);

    return album;
}

SFUInteger SFAlbumGetCodeunitCount(SFAlbumRef album)
{
    return album->codeunitCount;
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
    return album->_indexMap.items;
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
    album->codeunitCount = 0;
    album->glyphCount = 0;

    SFListInitialize(&album->_indexMap, sizeof(SFUInteger));
    SFListInitialize(&album->_glyphs, sizeof(SFGlyphID));
    SFListInitialize(&album->_details, sizeof(SFGlyphDetail));
    SFListInitialize(&album->_offsets, sizeof(SFPoint));
    SFListInitialize(&album->_advances, sizeof(SFAdvance));

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
    album->_retainCount = 1;
}

SF_INTERNAL void SFAlbumReset(SFAlbumRef album, SFCodepointsRef codepoints)
{
    SFUInteger codeunitCount;

    /* Codepoints object must be non-null. */
    SFAssert(codepoints != NULL);

    codeunitCount = SFCodepointsGetCodeUnitCount(codepoints);

    album->codepoints = codepoints;
    album->codeunitCount = codeunitCount;
    album->glyphCount = 0;

    SFListClear(&album->_indexMap);
    SFListReserveRange(&album->_indexMap, 0, codeunitCount);

    SFListClear(&album->_glyphs);
    SFListClear(&album->_details);
    SFListClear(&album->_offsets);
    SFListClear(&album->_advances);

    album->_version = 0;
    album->_state = _SFAlbumStateEmpty;
}

SF_INTERNAL void SFAlbumBeginFilling(SFAlbumRef album)
{
	SFUInteger glyphCapacity = album->codeunitCount;

    SFListReserveRange(&album->_glyphs, 0, glyphCapacity);
    SFListReserveRange(&album->_details, 0, glyphCapacity);

	album->_state = _SFAlbumStateFilling;
}

SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, SFGlyphTraits traits, SFUInteger association)
{
    SFUInteger index;
    SFGlyphDetailRef detail;

    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    album->_version++;
    index = album->glyphCount++;
    detail = SFListGetRef(&album->_details, index);

    /* Initialize the glyph along with its details. */
    SFListSetVal(&album->_glyphs, index, glyph);
    detail->association = association;
    detail->mask.section.feature = SFUInt16Max;
    detail->mask.section.traits = traits;
}

SF_INTERNAL SFUInteger *SFAlbumGetTemporaryIndexArray(SFAlbumRef album, SFUInteger count)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    if (album->_indexMap.capacity < count) {
        SFListSetCapacity(&album->_indexMap, count);
    }

    return album->_indexMap.items;
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
    SFListSetVal(&album->_glyphs, index, glyph);
}

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->association;
}

SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListGetRef(&album->_details, index)->association = association;
}

SF_PRIVATE SFGlyphMask _SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->mask;
}

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_details, index)->mask.section.feature;
}

SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListGetRef(&album->_details, index)->mask.section.feature = featureMask;
}

SF_INTERNAL SFGlyphTraits SFAlbumGetAllTraits(SFAlbumRef album, SFUInteger index)
{
    return (SFGlyphTraits)SFListGetRef(&album->_details, index)->mask.section.traits;
}

SF_INTERNAL void SFAlbumSetAllTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    SFListGetRef(&album->_details, index)->mask.section.traits = traits;
}

SF_INTERNAL void SFAlbumReplaceBasicTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    SFGlyphTraits *all;

    /* The album must be in filling state. */
    SFAssert(album->_state == _SFAlbumStateFilling);

    all = &SFListGetRef(&album->_details, index)->mask.section.traits;
    *all = (*all & 0xFF00) | (traits & 0x00FF);
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

SF_INTERNAL void SFAlbumInsertHelperTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);
    /* Traits must be helping ones only. */
    SFAssert((traits & 0x0F00) == traits);

    SFListGetRef(&album->_details, index)->mask.section.traits |= traits;
}

SF_INTERNAL void SFAlbumRemoveHelperTraits(SFAlbumRef album, SFUInteger index, SFGlyphTraits traits)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == _SFAlbumStateArranging);
    /* Traits must be helping ones only. */
    SFAssert((traits & 0x0F00) == traits);

    SFListGetRef(&album->_details, index)->mask.section.traits &= (SFUInt16)~traits;
}

SF_INTERNAL SFInt32 SFAlbumGetX(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_offsets, index)->x;
}

SF_INTERNAL void SFAlbumSetX(SFAlbumRef album, SFUInteger index, SFInt32 x)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == _SFAlbumStateArranging || album->_state == _SFAlbumStateArranged);

    SFListGetRef(&album->_offsets, index)->x = x;
}

SF_INTERNAL SFInt32 SFAlbumGetY(SFAlbumRef album, SFUInteger index)
{
    return SFListGetRef(&album->_offsets, index)->y;
}

SF_INTERNAL void SFAlbumSetY(SFAlbumRef album, SFUInteger index, SFInt32 y)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == _SFAlbumStateArranging || album->_state == _SFAlbumStateArranged);

    SFListGetRef(&album->_offsets, index)->y = y;
}

SF_INTERNAL SFAdvance SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index)
{
    return SFListGetVal(&album->_advances, index);
}

SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFAdvance advance)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == _SFAlbumStateArranging || album->_state == _SFAlbumStateArranged);

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
        SFGlyphTraits traits = SFAlbumGetAllTraits(album, index);
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
    SFUInteger codeunitCount = album->codeunitCount;
    SFUInteger association = 0;
    SFUInteger index;

    /* Initialize the map array. */
    for (index = 0; index < codeunitCount; index++) {
        SFListSetVal(&album->_indexMap, index, SFInvalidIndex);
    }

    /* Traverse in reverse order so that first glyph takes priority in case of multiple substitution. */
    for (index = album->glyphCount; index--;) {
        association = SFAlbumGetAssociation(album, index);
        SFListSetVal(&album->_indexMap, association, index);
    }

    if (!album->codepoints->backward) {
        /* Assign the same glyph index to subsequent codeunits. */
        for (index = 0; index < codeunitCount; index++) {
            if (SFListGetVal(&album->_indexMap, index) == SFInvalidIndex) {
                SFListSetVal(&album->_indexMap, index, association);
            }

            association = SFListGetVal(&album->_indexMap, index);
        }
    } else {
        /* Assign the same glyph index to preceding codeunits. */
        for (index = codeunitCount; index--;) {
            if (SFListGetVal(&album->_indexMap, index) == SFInvalidIndex) {
                SFListSetVal(&album->_indexMap, index, association);
            }

            association = SFListGetVal(&album->_indexMap, index);
        }
    }
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
    SFListFinalize(&album->_indexMap);
    SFListFinalize(&album->_glyphs);
    SFListFinalize(&album->_details);
    SFListFinalize(&album->_offsets);
    SFListFinalize(&album->_advances);
}
