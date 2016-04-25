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

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SFAssert.h"
#include "SFList.h"

#define SF_DEFAULT_LIST_CAPACITY 4

static void _SFListEnsureCapacity(_SFListRef list, SFUInteger capacity);
static void *_SFListGetItemPtr(_SFListRef list, SFUInteger index);
static void _SFListMoveItems(_SFListRef list, SFUInteger srcIndex, SFUInteger dstIndex, SFUInteger itemCount);

SF_PRIVATE void _SFListInitialize(_SFListRef list, SFUInteger itemSize)
{
    /* Item size MUST be greater than 0. */
    SFAssert(itemSize > 0);

    list->_data = NULL;
    list->count = 0;
    list->capacity = 0;
    list->_itemSize = itemSize;
}

SF_PRIVATE void _SFListFinalize(_SFListRef list)
{
    free(list->_data);
}

SF_PRIVATE void _SFListFinalizeKeepingArray(_SFListRef list, void **outArray, SFUInteger *outCount)
{
    if (list->count > 0) {
        _SFListSetCapacity(list, list->count);

        *outArray = list->_data;
        *outCount = list->count;
    } else {
        _SFListFinalize(list);

        *outArray = NULL;
        *outCount = 0;
    }
}

SF_PRIVATE void _SFListSetCapacity(_SFListRef list, SFUInteger capacity)
{
    /* The new capacity must be larger than total number of elements in the list. */
    SFAssert(capacity >= list->count);

    if (capacity != list->capacity) {
        list->_data = realloc(list->_data, list->_itemSize * capacity);
        list->capacity = capacity;
    }
}

static void _SFListEnsureCapacity(_SFListRef list, SFUInteger capacity)
{
    if (list->capacity < capacity) {
        SFUInteger newCapacity = (list->capacity ? list->count * 2 : SF_DEFAULT_LIST_CAPACITY);
        if (newCapacity < capacity) {
            newCapacity = capacity;
        }

        _SFListSetCapacity(list, newCapacity);
    }
}

static void *_SFListGetItemPtr(_SFListRef list, SFUInteger index)
{
    /* The index must fall within allocated capacity. */
    SFAssert(index < list->capacity);

    return list->_data + (index * list->_itemSize);
}

static void _SFListMoveItems(_SFListRef list, SFUInteger srcIndex, SFUInteger dstIndex, SFUInteger itemCount)
{
    /* The capacity must be available to move the block. */
    SFAssert((srcIndex + itemCount) <= list->capacity && (dstIndex + itemCount) <= list->capacity);

    if (itemCount) {
        memmove(_SFListGetItemPtr(list, dstIndex), _SFListGetItemPtr(list, srcIndex), list->_itemSize * itemCount);
    }
}

SF_PRIVATE void _SFListReserveRange(_SFListRef list, SFUInteger index, SFUInteger count)
{
    /* The index must be valid and there should be no integer overflow. */
    SFAssert(index <= list->count && index <= (index + count));

    _SFListEnsureCapacity(list, list->count + count);
    _SFListMoveItems(list, index, index + count, list->count - index);
    list->count += count;
}

SF_PRIVATE void _SFListRemoveRange(_SFListRef list, SFUInteger index, SFUInteger count)
{
    SFUInteger nextIndex = index + count;

    /* The specified item indexes must be valid and there should be no integer overflow. */
    SFAssert(nextIndex <= list->count && index <= nextIndex);

    _SFListMoveItems(list, nextIndex, index, list->count - nextIndex);
    list->count -= count;
}

SF_PRIVATE void _SFListClear(_SFListRef list)
{
    list->count = 0;
}

SF_PRIVATE void _SFListTrimExcess(_SFListRef list)
{
    _SFListSetCapacity(list, list->count);
}

SF_PRIVATE SFUInteger _SFListIndexOfItem(_SFListRef list, const void *itemPtr, SFUInteger index, SFUInteger count)
{
    /* The range must be valid and there should be no integer overflow. */
    SFAssert((list->count > 0 ? (index + count) <= list->count : (index + count) == 0)
             && index <= (index + count));

    for (; index < count; index++) {
        void *currentPtr = _SFListGetItemPtr(list, index);
        if (memcmp(currentPtr, itemPtr, list->_itemSize) == 0) {
            return index;
        }
    }

    return SFInvalidIndex;
}

SF_PRIVATE void _SFListSort(_SFListRef list, SFUInteger index, SFUInteger count, SFComparison comparison)
{
    /* The range must be valid and there should be no integer overflow. */
    SFAssert((list->count > 0 ? (index + count) <= list->count : (index + count) == 0)
             && index <= (index + count));

    qsort(list->_data, list->count, list->_itemSize, comparison);
}
