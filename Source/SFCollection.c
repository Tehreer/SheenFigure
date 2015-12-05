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

SF_INTERNAL void SFCollectionInitialize(SFCollectionRef collection, SFCodePoint *codePointArray, SFUInteger codePointCount)
{
	/* There must be some code points. */
	SFAssert(codePointArray != NULL && codePointCount > 0);

    collection->codePointArray = codePointArray;
	collection->mapArray = NULL;
    collection->codePointCount = codePointCount;
    collection->elementCount = 0;

    SFListInitialize(&collection->_glyphs, sizeof(SFGlyph));
    SFListInitialize(&collection->_details, sizeof(SFGlyphDetail));
    SFListInitialize(&collection->_positions, sizeof(SFPoint));
    SFListInitialize(&collection->_advances, sizeof(SFInteger));
}

SF_INTERNAL void SFCollectionAllocateGlyphs(SFCollectionRef collection)
{
	SFUInteger capacity = collection->codePointCount * 2;

    SFListReserveRange(&collection->_glyphs, 0, capacity);
    SFListReserveRange(&collection->_details, 0, capacity);

	collection->elementCount = collection->codePointCount;
}

SF_INTERNAL void SFCollectionAllocatePositions(SFCollectionRef collection)
{
    SFListReserveRange(&collection->_positions, 0, collection->elementCount);
    SFListReserveRange(&collection->_advances, 0, collection->elementCount);
}

SF_INTERNAL void SFCollectionAddGlyph(SFCollectionRef collection, SFGlyph glyph, SFUInteger association) {
    /* Initialize glyph along with its details. */
    SFCollectionSetGlyph(collection, collection->elementCount, glyph);
    SFCollectionSetTraits(collection, collection->elementCount, association);
    SFCollectionSetAssociation(collection, collection->elementCount, SFGlyphTraitNone);

    /* Increment element count. */
    collection->elementCount++;
}

SF_INTERNAL void SFCollectionReserveGlyphs(SFCollectionRef collection, SFUInteger index, SFUInteger count)
{
    SFListReserveRange(&collection->_glyphs, index, count);
    SFListReserveRange(&collection->_details, index, count);
}

static void SFValidateCollectionIndex(SFCollectionRef collection, SFIndex index)
{
    SFAssert(index < collection->elementCount);
}

SF_INTERNAL SFGlyph SFCollectionGetGlyph(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGet(&collection->_glyphs, index);
}

SF_INTERNAL void SFCollectionSetGlyph(SFCollectionRef collection, SFIndex index, SFGlyph glyph)
{
    SFValidateCollectionIndex(collection, index);
    SFListSet(&collection->_glyphs, index, glyph);
}

SF_INTERNAL SFGlyphTrait SFCollectionGetTraits(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGetRef(&collection->_details, index)->traits;
}

SF_INTERNAL void SFCollectionSetTraits(SFCollectionRef collection, SFIndex index, SFGlyphTrait traits)
{
    SFValidateCollectionIndex(collection, index);
    SFListGetRef(&collection->_details, index)->traits = traits;
}

SF_INTERNAL SFIndex SFCollectionGetAssociation(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGetRef(&collection->_details, index)->association;
}

SF_INTERNAL void SFCollectionSetAsociation(SFCollectionRef collection, SFIndex index, SFIndex association)
{
    SFValidateCollectionIndex(collection, index);
    SFListGetRef(&collection->_details, index)->association = association;
}

SF_INTERNAL SFPoint SFCollectionGetPosition(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGet(&collection->_positions, index);
}

SF_INTERNAL void SFCollectionSetPosition(SFCollectionRef collection, SFIndex index, SFPoint position)
{
    SFValidateCollectionIndex(collection, index);
    SFListSet(&collection->_positions, index, position);
}

SF_INTERNAL SFInteger SFCollectionGetAdvance(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGet(&collection->_advances, index);
}

SF_INTERNAL void SFCollectionSetAdvance(SFCollectionRef collection, SFIndex index, SFInteger advance)
{
    SFValidateCollectionIndex(collection, index);
    SFListSet(&collection->_advances, index, advance);
}

SF_INTERNAL SFUInt16 SFCollectionGetOffset(SFCollectionRef collection, SFIndex index)
{
    SFValidateCollectionIndex(collection, index);
    return SFListGetRef(&collection->_details, index)->offset;
}

SF_INTERNAL void SFCollectionSetOffset(SFCollectionRef collection, SFIndex index, SFUInt16 offset)
{
    SFValidateCollectionIndex(collection, index);
    SFListGetRef(&collection->_details, index)->offset = offset;
}

SF_INTERNAL void SFCollectionFinalize(SFCollectionRef collection) {
    SFListFinalize(&collection->_glyphs);
    SFListFinalize(&collection->_details);
    SFListFinalize(&collection->_positions);
    SFListFinalize(&collection->_advances);
}
