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
typedef struct _SFList SFList;
typedef SFList *SFListRef;

struct _SFList {
    SFUInt8 *_data;
    SFUInteger count;
    SFUInteger capacity;
    SFUInteger _itemSize;
};

#define SF_LIST(type)           \
union {                         \
    SFList _base;               \
    struct {                    \
        type *at;               \
        SFUInteger count;       \
    } items;                    \
}

SF_INTERNAL void SFListInitialize(SFListRef list, SFUInteger itemSize);

SF_INTERNAL void SFListSetCapacity(SFListRef list, SFUInteger capacity);

SF_INTERNAL void *SFListGetItem(SFListRef list, SFUInteger index);

SF_INTERNAL void SFListSetItem(SFListRef list, SFUInteger index, void *item);

SF_INTERNAL void SFListAdd(SFListRef list, void *item);

SF_INTERNAL void SFListInsert(SFListRef list, SFUInteger index, void *item);

SF_INTERNAL void SFListRemoveAt(SFListRef list, SFUInteger index);

SF_INTERNAL void SFListClear(SFListRef list);

SF_INTERNAL void SFListTrimExcess(SFListRef list);

SF_INTERNAL void SFListFinalize(SFListRef list);

#endif
