/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#include <stdlib.h>
#include <string.h>

#include "SFAssert.h"
#include "SFCollection.h"

#define _SF_CAPACITY_FACTOR 2

SF_INTERNAL void SFCollectionInitialize(SFCollectionRef collection, SFCodePoint *codePointArray, SFUInteger codePointCount)
{
	/* There must be some code points. */
	SFAssert(codePointArray != NULL && codePointCount > 0);

    collection->codePointArray = codePointArray;
    collection->_glyphArray = NULL;
    collection->_detailArray = NULL;
	collection->_positionArray = NULL;
	collection->_advanceArray = NULL;
	collection->mapArray = NULL;
    collection->codePointCount = codePointCount;
    collection->elementCount = 0;
    collection->_elementCapacity = 0;
}

SF_INTERNAL void SFCollectionAllocateGlyphs(SFCollectionRef collection)
{
	SFUInteger capacity = collection->codePointCount * 1.5;

	collection->_glyphArray = malloc(sizeof(SFGlyph) * capacity);
	collection->_detailArray = malloc(sizeof(SFGlyphDetail) * capacity);
	collection->elementCount = collection->codePointCount;
	collection->_elementCapacity = capacity;
}

static void _SFIncreaseCapacity(SFCollectionRef collection, SFUInteger glyphCount)
{
    SFUInteger newCapacity = (collection->_elementCapacity + glyphCount) * _SF_CAPACITY_FACTOR;
    collection->_glyphArray = realloc(collection->_glyphArray, sizeof(SFGlyph) * newCapacity);
    collection->_detailArray = realloc(collection->_detailArray, sizeof(SFGlyphDetail) * newCapacity);
    collection->_elementCapacity = newCapacity;
}

static void _SFMakeRoomForGlyphs(SFCollectionRef collection, SFUInteger sourceIndex, SFUInteger glyphCount) {
    memmove(collection->_glyphArray + sourceIndex + glyphCount,
            collection->_glyphArray + sourceIndex + 0,
            sizeof(SFGlyph));
    memmove(collection->_detailArray + sourceIndex + glyphCount,
            collection->_detailArray + sourceIndex + 0,
            sizeof(SFGlyphDetail));
}

SF_INTERNAL void SFCollectionAddGlyph(SFCollectionRef collection, SFGlyph glyph, SFUInteger association) {
    /* Added number of glyphs must be less than total number of characters. */
    SFAssert(collection->elementCount < collection->codePointCount);

    /* Initialize glyph along with its details. */
    SFCollectionSetGlyph(collection, collection->elementCount, glyph);
    SFCollectionSetTraits(collection, collection->elementCount, association);
    SFCollectionSetAssociation(collection, collection->elementCount, SFGlyphTraitNone);

    /* Increment glyph count. */
    collection->elementCount++;
}

SF_INTERNAL void SFCollectionReserveGlyphs(SFCollectionRef collection, SFUInteger index, SFUInteger glyphCount) {
    /* Index must be less than or equal to total number of glyphs.*/
    SFAssert(index <= collection->elementCount);
    /* The number of glyphs to be inserted must be greater than zero. */
    SFAssert(glyphCount > 0);

    /* Increase capacity if there is no room for new glyphs. */
    if (collection->_elementCapacity < (collection->elementCount + glyphCount)) {
        _SFIncreaseCapacity(collection, glyphCount);
    }

    /* Make room for new glyphs at the given index. */
    _SFMakeRoomForGlyphs(collection, index, glyphCount);
}

static void SFValidateCollectionIndex(SFCollectionRef collection, SFIndex index)
{
    SFAssert(index < collection->elementCount);
}

SF_INTERNAL SFGlyph SFCollectionGetGlyph(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_glyphArray[index];
}

SF_INTERNAL void SFCollectionSetGlyph(SFCollectionRef collection, SFIndex index, SFGlyph glyph)
{
    SFValidateCollectionIndex(collection, index);
    collection->_glyphArray[index] = glyph;
}

SF_INTERNAL SFGlyphTrait SFCollectionGetTraits(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_detailArray[index].traits;
}

SF_INTERNAL void SFCollectionSetTraits(SFCollectionRef collection, SFIndex index, SFGlyphTrait traits)
{
    SFValidateCollectionIndex(collection, index);
    collection->_detailArray[index].traits = traits;
}

SF_INTERNAL SFIndex SFCollectionGetAssociation(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_detailArray[index].association;
}

SF_INTERNAL void SFCollectionSetAsociation(SFCollectionRef collection, SFIndex index, SFIndex association)
{
    SFValidateCollectionIndex(collection, index);
    collection->_detailArray[index].association = association;
}

SF_INTERNAL SFPoint SFCollectionGetPosition(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_positionArray[index];
}

SF_INTERNAL void SFCollectionSetPosition(SFCollectionRef collection, SFIndex index, SFPoint position)
{
    SFValidateCollectionIndex(collection, index);
    collection->_positionArray[index] = position;
}

SF_INTERNAL SFInteger SFCollectionGetAdvance(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_advanceArray[index]
}

SF_INTERNAL void SFCollectionSetAdvance(SFCollectionRef collection, SFIndex index, SFInteger advance)
{
    SFValidateCollectionIndex(collection, index);
    collection->_advanceArray[index] = advance;
}

SF_INTERNAL SFUInt16 SFCollectionGetOffset(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return collection->_detailArray[index].offset;
}

SF_INTERNAL void SFCollectionSetOffset(SFCollectionRef collection, SFIndex index, SFUInt16 offset)
{
    SFValidateCollectionIndex(collection, index);
    collection->_detailArray[index].offset = offset;
}

SF_INTERNAL void SFCollectionFinalize(SFCollectionRef collection) {
    free(collection->_glyphArray);
    free(collection->_detailArray);
    free(collection->_positionArray);
    free(collection->_advanceArray);
}
