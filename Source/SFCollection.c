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
    collection->glyphArray = NULL;
    collection->detailArray = NULL;
	collection->positionArray = NULL;
	collection->advanceArray = NULL;
	collection->mapArray = NULL;
    collection->codePointCount = codePointCount;
    collection->elementCount = 0;
    collection->_elementCapacity = 0;
}

SF_INTERNAL void SFCollectionAllocateGlyphs(SFCollectionRef collection)
{
	SFUInteger capacity = collection->codePointCount * 1.5;

	collection->glyphArray = malloc(sizeof(SFGlyph) * capacity);
	collection->detailArray = malloc(sizeof(SFGlyphDetail) * capacity);
	collection->elementCount = collection->codePointCount;
	collection->_elementCapacity = capacity;
}

static void _SFIncreaseCapacity(SFCollectionRef collection, SFUInteger glyphCount)
{
    SFUInteger newCapacity = (collection->_elementCapacity + glyphCount) * _SF_CAPACITY_FACTOR;
    collection->glyphArray = realloc(collection->glyphArray, sizeof(SFGlyph) * newCapacity);
    collection->detailArray = realloc(collection->detailArray, sizeof(SFGlyphDetail) * newCapacity);
    collection->_elementCapacity = newCapacity;
}

static void _SFMakeRoomForGlyphs(SFCollectionRef collection, SFUInteger sourceIndex, SFUInteger glyphCount) {
    memmove(collection->glyphArray + sourceIndex + glyphCount,
            collection->glyphArray + sourceIndex + 0,
            sizeof(SFGlyph));
    memmove(collection->detailArray + sourceIndex + glyphCount,
            collection->detailArray + sourceIndex + 0,
            sizeof(SFGlyphDetail));
}

SF_INTERNAL void SFCollectionAddGlyph(SFCollectionRef collection, SFGlyph glyph, SFUInteger association) {
    SFElementDetailRef detail;

    /* Added number of glyphs must be less than total number of characters. */
    SFAssert(collection->elementCount < collection->codePointCount);

    /* Initialize glyph along with its details. */
    collection->valueArray[collection->elementCount].glyph = glyph;
    detail = &collection->detailArray[collection->elementCount];
    detail->association = association;
    detail->traits = SFGlyphTraitNone;

    /* Increment glyph count. */
    collection->elementCount++;
}

SF_INTERNAL void SFCollectionReserveElements(SFCollectionRef collection, SFUInteger index, SFUInteger elementCount) {
    /* Index must be less than or equal to total number of glyphs.*/
    SFAssert(index <= collection->elementCount);
    /* The number of glyphs to be inserted must be greater than zero. */
    SFAssert(elementCount > 0);

    /* Increase capacity if there is no room for new glyphs. */
    if (collection->_elementCapacity < (collection->elementCount + elementCount)) {
        _SFIncreaseCapacity(collection, elementCount);
    }

    /* Make room for new glyphs at the given index. */
    _SFMakeRoomForGlyphs(collection, index, elementCount);
}

SF_INTERNAL void SFCollectionInvalidate(SFCollectionRef collection) {
    free(collection->valueArray);
    free(collection->detailArray);
}
