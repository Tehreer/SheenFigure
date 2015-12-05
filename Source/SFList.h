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

#ifndef SF_LIST_INTERNAL_H
#define SF_LIST_INTERNAL_H

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

#define SF_LIST(type)           \
union {                         \
    _SFList _base;              \
    struct {                    \
        type *at;               \
        SFUInteger count;       \
        SFUInteger capacity;    \
    } items;                    \
}

SF_PRIVATE void _SFListInitialize(_SFListRef list, SFUInteger itemSize);
SF_PRIVATE void _SFListFinalize(_SFListRef list);

SF_PRIVATE void _SFListSetCapacity(_SFListRef list, SFUInteger capacity);

SF_PRIVATE void _SFListReserveRange(_SFListRef list, SFUInteger index, SFUInteger count);
SF_PRIVATE void _SFListRemoveRange(_SFListRef list, SFUInteger index, SFUInteger count);

SF_PRIVATE void _SFListClear(_SFListRef list);
SF_PRIVATE void _SFListTrimExcess(_SFListRef list);

#define _SFListSet(list_, index_, item_)                \
do {                                                    \
    SFUInteger __setIndex = index_;                     \
    SFAssert(__setIndex < (list_)->items.count);        \
    (list_)->items.at[__setIndex] = item_;              \
} while (0)

#define _SFListInsert(list_, index_, item_)             \
do {                                                    \
    SFUInteger __insertIndex = index_;                  \
    _SFListReserveRange(&(list_)->_base, __insertIndex, 1); \
    _SFListSet(list_, __insertIndex, item_);            \
} while (0)

#define _SFListAdd(list_, item_)                        \
        _SFListInsert(list_, (list_)->items.count, item_)

#define SFListInitialize(list, itemSize)            _SFListInitialize(&(list)->_base, itemSize)
#define SFListFinalize(list)                        _SFListFinalize(&(list)->_base)

#define SFListSetCapacity(list, capacity)           _SFListSetCapacity(&(list)->_base, capacity)
#define SFListReserveRange(list, index, count)      _SFListReserveRange(&(list)->_base, index, count)
#define SFListRemoveRange(list, index, count)       _SFListRemoveRange(&(list)->_base, index, count)

#define SFListSet(list, index, item)                _SFListSet(list, index, item)
#define SFListAdd(list, item)                       _SFListAdd(list, item)
#define SFListInsert(list, index, item)             _SFListInsert(list, index, item)
#define SFListRemoveAt(list, index)                 _SFListRemoveRange(&(list)->_base, index, 1)

#define SFListClear(list)                           _SFListClear(&(list)->_base)
#define SFListTrimExcess(list)                      _SFListTrimExcess(&(list)->_base)

#endif
