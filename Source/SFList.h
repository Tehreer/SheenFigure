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

#ifndef _SF_INTERNAL_LIST_H
#define _SF_INTERNAL_LIST_H

#include <SFConfig.h>

#include "SFAssert.h"
#include "SFBase.h"

typedef struct {
    SFUInt8 *_data;
    SFUInteger count;
    SFUInteger capacity;
    SFUInteger _itemSize;
} List, *ListRef;

#define LIST(type)          \
struct {                    \
    type *items;            \
    SFUInteger count;       \
    SFUInteger capacity;    \
    SFUInteger _itemSize;   \
}

typedef int (*SFComparison)(const void *item1, const void *item2);

SF_PRIVATE void InitializeList(ListRef list, SFUInteger itemSize);
SF_PRIVATE void FinalizeItemsBuffer(ListRef list);
SF_PRIVATE void ExtractItemsBuffer(ListRef list, void **outArray, SFUInteger *outCount);

SF_PRIVATE void SetItemCapacity(ListRef list, SFUInteger capacity);
SF_PRIVATE void ReserveItemRange(ListRef list, SFUInteger index, SFUInteger count);
SF_PRIVATE void RemoveItemRange(ListRef list, SFUInteger index, SFUInteger count);

SF_PRIVATE void RemoveAllItems(ListRef list);
SF_PRIVATE void TrimExcessCapacity(ListRef list);

SF_PRIVATE SFUInteger SearchItemInRange(ListRef list, const void *itemPtr, SFUInteger index, SFUInteger count);
SF_PRIVATE void SortItemRange(ListRef list, SFUInteger index, SFUInteger count, SFComparison comparison);

#define CheckItemIndex(list_, index_)                   \
(                                                       \
    SFAssert(index_ < (list_)->count)                   \
)

#define GetItemReference(list_, index_)                 \
(                                                       \
    CheckItemIndex(list_, index_),                      \
    &(list_)->items[index_]                             \
)

#define GetItemAtIndex(list_, index_)                   \
(                                                       \
    CheckItemIndex(list_, index_),                      \
    (list_)->items[index_]                              \
)

#define SetItemAtIndex(list_, index_, item_)            \
do {                                                    \
    CheckItemIndex(list_, index_),                      \
    (list_)->items[index_] = item_;                     \
} while (0)

#define InsertItemAtIndex(list_, index_, item_)         \
do {                                                    \
    SFUInteger __insertIndex = index_;                  \
    ReserveItemRange((ListRef)(list_), __insertIndex, 1); \
    SetItemAtIndex(list_, __insertIndex, item_);        \
} while (0)

#define InsertItemAtEnd(list_, item_)                   \
    InsertItemAtIndex(list_, (list_)->count, item_)


#define ListInitialize(list, itemSize)              InitializeList((ListRef)(list), itemSize)
#define ListFinalize(list)                          FinalizeItemsBuffer((ListRef)(list))
#define ListFinalizeKeepingArray(list, outArray, outCount) \
    ExtractItemsBuffer((ListRef)(list), (void **)outArray, outCount)

#define ListSetCapacity(list, capacity)             SetItemCapacity((ListRef)(list), capacity)
#define ListReserveRange(list, index, count)        ReserveItemRange((ListRef)(list), index, count)
#define ListRemoveRange(list, index, count)         RemoveItemRange((ListRef)(list), index, count)

#define ListClear(list)                             RemoveAllItems((ListRef)(list))
#define ListTrimExcess(list)                        TrimExcessCapacity((ListRef)(list))

#define ListGetRef(list, index)                     GetItemReference(list, index)
#define ListGetVal(list, index)                     GetItemAtIndex(list, index)
#define ListSetVal(list, index, item)               SetItemAtIndex(list, index, item)

#define ListAdd(list, item)                         InsertItemAtEnd(list, item)
#define ListInsert(list, index, item)               InsertItemAtIndex(list, index, item)
#define ListRemoveAt(list, index)                   RemoveItemRange((ListRef)(list), index, 1)

#define ListIndexOfItem(list, item, index, count)   SearchItemInRange((ListRef)(list), item, index, count)
#define ListContainsItem(list, item) \
    (SearchItemInRange((ListRef)(list), item, 0, (list)->count) != SFInvalidIndex)

#define ListSort(list, index, count, comparison)    SortItemRange((ListRef)(list), index, count, comparison);

#endif
