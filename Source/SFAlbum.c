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

static const GlyphMask EmptyGlyphMask = { { SFUInt16Max, 0 } };

SF_PRIVATE SFUInt16 GetAntiFeatureMask(SFUInt16 featureMask)
{
    /* The assumtion must NOT break that the feature mask will never be equal to default mask. */
    SFAssert(featureMask != EmptyGlyphMask.section.feature);

    return !featureMask ? ~EmptyGlyphMask.section.feature : ~featureMask;
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

    ListInitialize(&album->_indexMap, sizeof(SFUInteger));
    ListInitialize(&album->_glyphs, sizeof(SFGlyphID));
    ListInitialize(&album->_details, sizeof(GlyphDetail));
    ListInitialize(&album->_offsets, sizeof(SFPoint));
    ListInitialize(&album->_advances, sizeof(SFAdvance));

    album->_version = 0;
    album->_state = AlbumStateEmpty;
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

    ListClear(&album->_indexMap);
    ListReserveRange(&album->_indexMap, 0, codeunitCount);

    ListClear(&album->_glyphs);
    ListClear(&album->_details);
    ListClear(&album->_offsets);
    ListClear(&album->_advances);

    album->_version = 0;
    album->_state = AlbumStateEmpty;
}

SF_INTERNAL void SFAlbumBeginFilling(SFAlbumRef album)
{
	SFUInteger glyphCapacity = album->codeunitCount;

    ListReserveRange(&album->_glyphs, 0, glyphCapacity);
    ListReserveRange(&album->_details, 0, glyphCapacity);

	album->_state = AlbumStateFilling;
}

SF_INTERNAL void SFAlbumAddGlyph(SFAlbumRef album, SFGlyphID glyph, GlyphTraits traits, SFUInteger association)
{
    SFUInteger index;
    GlyphDetailRef detail;

    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    album->_version++;
    index = album->glyphCount++;
    detail = ListGetRef(&album->_details, index);

    /* Initialize the glyph along with its details. */
    ListSetVal(&album->_glyphs, index, glyph);
    detail->association = association;
    detail->mask.section.feature = SFUInt16Max;
    detail->mask.section.traits = traits;
}

SF_INTERNAL SFUInteger *SFAlbumGetTemporaryIndexArray(SFAlbumRef album, SFUInteger count)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    if (album->_indexMap.capacity < count) {
        ListSetCapacity(&album->_indexMap, count);
    }

    return album->_indexMap.items;
}

SF_INTERNAL void SFAlbumReserveGlyphs(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    album->_version++;
    album->glyphCount += count;

    ListReserveRange(&album->_glyphs, index, count);
    ListReserveRange(&album->_details, index, count);
}

SF_INTERNAL SFGlyphID SFAlbumGetGlyph(SFAlbumRef album, SFUInteger index)
{
    return ListGetVal(&album->_glyphs, index);
}

SF_INTERNAL void SFAlbumSetGlyph(SFAlbumRef album, SFUInteger index, SFGlyphID glyph)
{
    ListSetVal(&album->_glyphs, index, glyph);
}

SF_INTERNAL SFUInteger SFAlbumGetAssociation(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_details, index)->association;
}

SF_INTERNAL void SFAlbumSetAssociation(SFAlbumRef album, SFUInteger index, SFUInteger association)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    ListGetRef(&album->_details, index)->association = association;
}

SF_PRIVATE GlyphMask SFAlbumGetGlyphMask(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_details, index)->mask;
}

SF_INTERNAL SFUInt16 SFAlbumGetFeatureMask(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_details, index)->mask.section.feature;
}

SF_INTERNAL void SFAlbumSetFeatureMask(SFAlbumRef album, SFUInteger index, SFUInt16 featureMask)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    ListGetRef(&album->_details, index)->mask.section.feature = featureMask;
}

SF_INTERNAL GlyphTraits SFAlbumGetAllTraits(SFAlbumRef album, SFUInteger index)
{
    return (GlyphTraits)ListGetRef(&album->_details, index)->mask.section.traits;
}

SF_INTERNAL void SFAlbumSetAllTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    ListGetRef(&album->_details, index)->mask.section.traits = traits;
}

SF_INTERNAL void SFAlbumReplaceBasicTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits)
{
    GlyphTraits *all;

    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    all = &ListGetRef(&album->_details, index)->mask.section.traits;
    *all = (*all & 0xFF00) | (traits & 0x00FF);
}

SF_INTERNAL void SFAlbumEndFilling(SFAlbumRef album)
{
    /* The album must be in filling state. */
    SFAssert(album->_state == AlbumStateFilling);

    album->_state = AlbumStateFilled;
}

SF_INTERNAL void SFAlbumBeginArranging(SFAlbumRef album)
{
    /* The album must be filled before arranging it. */
    SFAssert(album->_state == AlbumStateFilled);

    ListReserveRange(&album->_offsets, 0, album->glyphCount);
    ListReserveRange(&album->_advances, 0, album->glyphCount);

    album->_state = AlbumStateArranging;
}

SF_INTERNAL void SFAlbumInsertHelperTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == AlbumStateArranging);
    /* Traits must be helping ones only. */
    SFAssert((traits & 0x0F00) == traits);

    ListGetRef(&album->_details, index)->mask.section.traits |= traits;
}

SF_INTERNAL void SFAlbumRemoveHelperTraits(SFAlbumRef album, SFUInteger index, GlyphTraits traits)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == AlbumStateArranging);
    /* Traits must be helping ones only. */
    SFAssert((traits & 0x0F00) == traits);

    ListGetRef(&album->_details, index)->mask.section.traits &= (SFUInt16)~traits;
}

SF_INTERNAL SFInt32 SFAlbumGetX(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_offsets, index)->x;
}

SF_INTERNAL void SFAlbumSetX(SFAlbumRef album, SFUInteger index, SFInt32 x)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == AlbumStateArranging || album->_state == AlbumStateArranged);

    ListGetRef(&album->_offsets, index)->x = x;
}

SF_INTERNAL SFInt32 SFAlbumGetY(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_offsets, index)->y;
}

SF_INTERNAL void SFAlbumSetY(SFAlbumRef album, SFUInteger index, SFInt32 y)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == AlbumStateArranging || album->_state == AlbumStateArranged);

    ListGetRef(&album->_offsets, index)->y = y;
}

SF_INTERNAL SFAdvance SFAlbumGetAdvance(SFAlbumRef album, SFUInteger index)
{
    return ListGetVal(&album->_advances, index);
}

SF_INTERNAL void SFAlbumSetAdvance(SFAlbumRef album, SFUInteger index, SFAdvance advance)
{
    /* The album must be in arranging or arranged state. */
    SFAssert(album->_state == AlbumStateArranging || album->_state == AlbumStateArranged);

    ListSetVal(&album->_advances, index, advance);
}

SF_INTERNAL SFUInt16 SFAlbumGetCursiveOffset(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_details, index)->cursiveOffset;
}

SF_INTERNAL void SFAlbumSetCursiveOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == AlbumStateArranging);

    ListGetRef(&album->_details, index)->cursiveOffset = offset;
}

SF_INTERNAL SFUInt16 SFAlbumGetAttachmentOffset(SFAlbumRef album, SFUInteger index)
{
    return ListGetRef(&album->_details, index)->attachmentOffset;
}

SF_INTERNAL void SFAlbumSetAttachmentOffset(SFAlbumRef album, SFUInteger index, SFUInt16 offset)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == AlbumStateArranging);

    ListGetRef(&album->_details, index)->attachmentOffset = offset;
}

SF_INTERNAL void SFAlbumEndArranging(SFAlbumRef album)
{
    /* The album must be in arranging state. */
    SFAssert(album->_state == AlbumStateArranging);

    album->_state = AlbumStateArranged;
}

static void RemoveGlyphRange(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    ListRemoveRange(&album->_glyphs, index, count);
    ListRemoveRange(&album->_details, index, count);
    ListRemoveRange(&album->_offsets, index, count);
    ListRemoveRange(&album->_advances, index, count);
}

static void RemovePlaceholderGlyphs(SFAlbumRef album)
{
    SFUInteger placeholderCount = 0;
    SFUInteger index = album->glyphCount;

    while (index--) {
        GlyphTraits traits = SFAlbumGetAllTraits(album, index);
        if (traits & GlyphTraitPlaceholder) {
            placeholderCount++;
        } else {
            if (placeholderCount) {
                RemoveGlyphRange(album, index + 1, placeholderCount);
                album->glyphCount -= placeholderCount;
                placeholderCount = 0;
            }
        }
    }

    if (placeholderCount) {
        RemoveGlyphRange(album, 0, placeholderCount);
        album->glyphCount -= placeholderCount;
    }
}

static void BuildCodeUnitToGlyphMap(SFAlbumRef album)
{
    SFUInteger codeunitCount = album->codeunitCount;
    SFUInteger association = 0;
    SFUInteger index;

    /* Initialize the map array. */
    for (index = 0; index < codeunitCount; index++) {
        ListSetVal(&album->_indexMap, index, SFInvalidIndex);
    }

    /* Traverse in reverse order so that first glyph takes priority in case of multiple substitution. */
    for (index = album->glyphCount; index--;) {
        association = SFAlbumGetAssociation(album, index);
        ListSetVal(&album->_indexMap, association, index);
    }

    if (!album->codepoints->backward) {
        /* Assign the same glyph index to subsequent codeunits. */
        for (index = 0; index < codeunitCount; index++) {
            if (ListGetVal(&album->_indexMap, index) == SFInvalidIndex) {
                ListSetVal(&album->_indexMap, index, association);
            }

            association = ListGetVal(&album->_indexMap, index);
        }
    } else {
        /* Assign the same glyph index to preceding codeunits. */
        for (index = codeunitCount; index--;) {
            if (ListGetVal(&album->_indexMap, index) == SFInvalidIndex) {
                ListSetVal(&album->_indexMap, index, association);
            }

            association = ListGetVal(&album->_indexMap, index);
        }
    }
}

SF_INTERNAL void SFAlbumWrapUp(SFAlbumRef album)
{
    /* The album must be in completed state before wrapping up. */
    SFAssert(album->_state == AlbumStateFilled || album->_state == AlbumStateArranged);

    RemovePlaceholderGlyphs(album);
    BuildCodeUnitToGlyphMap(album);

    album->codepoints = NULL;
}

SF_INTERNAL void SFAlbumFinalize(SFAlbumRef album) {
    ListFinalize(&album->_indexMap);
    ListFinalize(&album->_glyphs);
    ListFinalize(&album->_details);
    ListFinalize(&album->_offsets);
    ListFinalize(&album->_advances);
}
