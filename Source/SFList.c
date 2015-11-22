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

SF_INTERNAL void SFListInitialize(SFListRef list, SFUInteger itemSize)
{
    list->_data = NULL;
    list->count = 0;
    list->capacity = 0;
    list->_itemSize = itemSize;
}

static void *_SFListItemPtr(SFListRef list, SFUInteger index)
{
    /* The index must fall within allocated capacity. */
    SFAssert(index < list->capacity);

    return list->_data + (index * list->_itemSize);
}

static void _SFSetListItem(SFListRef list, SFUInteger index, void *item)
{
    /* The index must fall within allocated capacity. */
    SFAssert(index < list->capacity);

    memcpy(_SFListItemPtr(list, index), item, list->_itemSize);
}

static void _SFMoveItems(SFListRef list, SFUInteger srcIndex, SFUInteger dstIndex, SFUInteger itemCount)
{
    /* The capacity must be available to move the block. */
    SFAssert((srcIndex + itemCount) <= list->capacity && (dstIndex + itemCount) <= list->capacity);

    memmove(_SFListItemPtr(list, dstIndex), _SFListItemPtr(list, srcIndex), list->_itemSize * itemCount);
}

SF_INTERNAL void SFListSetCapacity(SFListRef list, SFUInteger capacity)
{
    /* The new capacity must be larger than total number of elements in the list. */
    SFAssert(capacity > list->count);

    if (capacity != list->capacity) {
        list->_data = realloc(list->_data, list->_itemSize * capacity);
    }
}

static void _SFEnsureCapacity(SFListRef list, SFUInteger capacity)
{
    if (list->count < capacity) {
        SFUInteger newCapacity = (list->capacity ? list->count * 2 : SF_DEFAULT_LIST_CAPACITY);
        if (newCapacity < capacity) {
            newCapacity = capacity;
        }

        SFListSetCapacity(list, newCapacity);
    }
}

SF_INTERNAL void *SFListGetItem(SFListRef list, SFUInteger index)
{
    /* The index must belong to an item in the list. */
    SFAssert(index < list->count);

    return _SFListItemPtr(list, index);
}

SF_INTERNAL void SFListSetItem(SFListRef list, SFUInteger index, void *item)
{
    /* The index must belong to an item in the list. */
    SFAssert(index < list->count);

    _SFSetListItem(list, index, item);
}

SF_INTERNAL void SFListAdd(SFListRef list, void *item)
{
    SFListInsert(list, list->count + 1, item);
}

SF_INTERNAL void SFListInsert(SFListRef list, SFUInteger index, void *item)
{
    /* The index must be valid. */
    SFAssert(index <= list->count);

    if (list->capacity == list->count) {
        _SFEnsureCapacity(list, list->count + 1);
    }

    if (index < list->count) {
        _SFMoveItems(list, index, index + 1, list->count - index);
    }

    _SFSetListItem(list, index, item);
    list->count++;
}

SF_INTERNAL void SFListRemoveAt(SFListRef list, SFUInteger index)
{
    /* The index must belong to an item in the list. */
    SFAssert(index < list->count);

    list->count--;
    _SFMoveItems(list, index + 1, index, list->count - index);
}

SF_INTERNAL void SFListClear(SFListRef list)
{
    list->count = 0;
}

SF_INTERNAL void SFListTrimExcess(SFListRef list)
{
    SFListSetCapacity(list, list->count);
}

SF_INTERNAL void SFListInvalidate(SFListRef list)
{
    free(list->_data);
}
