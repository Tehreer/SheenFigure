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

#ifndef _SF_LIST_INTERNAL_H
#define _SF_LIST_INTERNAL_H

#include <SFConfig.h>
#include <SFTypes.h>

struct _SFList;
typedef struct _SFList _SFList;
typedef _SFList *_SFListRef;

struct _SFList {
    SFUInt8 *_data;
    SFUInteger count;
    SFUInteger capacity;
    SFUInteger _itemSize;
};

#define SF_LIST(type)       \
struct {                    \
    type *items;            \
    SFUInteger count;       \
    SFUInteger capacity;    \
    SFUInteger _itemSize;   \
}

typedef int(*SFComparison)(const void *item1, const void *item2);

SF_PRIVATE void _SFListInitialize(_SFListRef list, SFUInteger itemSize);
SF_PRIVATE void _SFListFinalize(_SFListRef list);
SF_PRIVATE void _SFListFinalizeKeepingArray(_SFListRef list, void **outArray, SFUInteger *outCount);

SF_PRIVATE void _SFListSetCapacity(_SFListRef list, SFUInteger capacity);
SF_PRIVATE void _SFListReserveRange(_SFListRef list, SFUInteger index, SFUInteger count);
SF_PRIVATE void _SFListRemoveRange(_SFListRef list, SFUInteger index, SFUInteger count);

SF_PRIVATE void _SFListClear(_SFListRef list);
SF_PRIVATE void _SFListTrimExcess(_SFListRef list);

SF_PRIVATE SFUInteger _SFListIndexOfItem(_SFListRef list, const void *itemPtr, SFUInteger index, SFUInteger count);
SF_PRIVATE void _SFListSort(_SFListRef list, SFUInteger index, SFUInteger count, SFComparison comparison);

#define _SFListValidateIndex(list_, index_)             \
(                                                       \
    SFAssert(index_ < (list_)->count)                   \
)

#define _SFListGetRef(list_, index_)                    \
(                                                       \
    _SFListValidateIndex(list_, index_),                \
    &(list_)->items[index_]                             \
)

#define _SFListGetVal(list_, index_)                    \
(                                                       \
    _SFListValidateIndex(list_, index_),                \
    (list_)->items[index_]                              \
)

#define _SFListSetVal(list_, index_, item_)             \
do {                                                    \
    _SFListValidateIndex(list_, index_),                \
    (list_)->items[index_] = item_;                     \
} while (0)

#define _SFListInsert(list_, index_, item_)             \
do {                                                    \
    SFUInteger __insertIndex = index_;                  \
    _SFListReserveRange((_SFListRef)(list_), __insertIndex, 1); \
    _SFListSetVal(list_, __insertIndex, item_);         \
} while (0)

#define _SFListAdd(list_, item_)                        \
        _SFListInsert(list_, (list_)->count, item_)

#define _SFListIndexOf(list_, index_, item_)            \
do {                                                    \
    SFUInteger __safeIndex = index_;                    \
    _SFListReserveRange((_SFListRef)(list_), __safeIndex, 1);   \
    _SFListSetVal(list_, __safeIndex, item_);           \
} while (0)


#define SFListInitialize(list, itemSize)            _SFListInitialize((_SFListRef)(list), itemSize)
#define SFListFinalize(list)                        _SFListFinalize((_SFListRef)(list))
#define SFListFinalizeKeepingArray(list, outArray, outCount) \
                                    _SFListFinalizeKeepingArray((_SFListRef)(list), (void **)outArray, outCount)

#define SFListSetCapacity(list, capacity)           _SFListSetCapacity((_SFListRef)(list), capacity)
#define SFListReserveRange(list, index, count)      _SFListReserveRange((_SFListRef)(list), index, count)
#define SFListRemoveRange(list, index, count)       _SFListRemoveRange((_SFListRef)(list), index, count)

#define SFListClear(list)                           _SFListClear((_SFListRef)(list))
#define SFListTrimExcess(list)                      _SFListTrimExcess((_SFListRef)(list))

#define SFListGetRef(list, index)                   _SFListGetRef(list, index)
#define SFListGetVal(list, index)                   _SFListGetVal(list, index)
#define SFListSetVal(list, index, item)             _SFListSetVal(list, index, item)

#define SFListAdd(list, item)                       _SFListAdd(list, item)
#define SFListInsert(list, index, item)             _SFListInsert(list, index, item)
#define SFListRemoveAt(list, index)                 _SFListRemoveRange((_SFListRef)(list), index, 1)

#define SFListIndexOfItem(list, item, index, count) _SFListIndexOfItem((_SFListRef)(list), item, index, count)
#define SFListContainsItem(list, item)             (_SFListIndexOfItem((_SFListRef)(list), item, 0, (list)->count) != SFInvalidIndex)

#define SFListSort(list, index, count, comparison)  _SFListSort((_SFListRef)(list), index, count, comparison);

#endif
