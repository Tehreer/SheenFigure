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

#include <cassert>
#include <cstddef>
#include <cstring>

extern "C" {
#include <Source/SFList.h>
}

#include "ListTester.h"

using namespace SheenFigure::Tester;

ListTester::ListTester()
{
}

void ListTester::testInitialize()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    assert(list.items == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);
}

void ListTester::testSetCapacity()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    /* Test by setting a finite capacity. */
    SFListSetCapacity(&list, 1024);
    assert(list.capacity == 1024);

    /* Test by setting zero capacity. */
    SFListSetCapacity(&list, 0);
    assert(list.capacity == 0);

    SFListFinalize(&list);
}

void ListTester::testReserveRange()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    /* Test by reserving some items. */
    SFListReserveRange(&list, 0, 10);
    assert(list.count == 10);

    /* Test by reserving items at the end of the list. */
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 9, 200);
    SFListReserveRange(&list, 10, 5);
    assert(list.count == 15);
    assert(list.items[0] == 100);
    assert(list.items[9] == 200);

    /* Test by reserving items at the center of the list. */
    SFListSetVal(&list, 4, 300);
    SFListSetVal(&list, 5, 400);
    SFListReserveRange(&list, 5, 5);
    assert(list.count == 20);
    assert(list.items[0] == 100);
    assert(list.items[4] == 300);
    assert(list.items[10] == 400);
    assert(list.items[14] == 200);

    /* Test by reserving items at the start of the list. */
    SFListSetVal(&list, 19, 500);
    SFListReserveRange(&list, 0, 5);
    assert(list.count == 25);
    assert(list.items[5] == 100);
    assert(list.items[9] == 300);
    assert(list.items[15] == 400);
    assert(list.items[19] == 200);
    assert(list.items[24] == 500);

    SFListFinalize(&list);
}

void ListTester::testAdd()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListAdd(&list, 100);
    SFListAdd(&list, 200);
    SFListAdd(&list, 300);
    SFListAdd(&list, 400);
    SFListAdd(&list, 500);

    SFInteger expected[] = { 100, 200, 300, 400, 500 };
    assert(memcmp(list.items, expected, sizeof(expected)) == 0);

    SFListFinalize(&list);
}

void ListTester::testInsert()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListInsert(&list, 0, 100);
    SFListInsert(&list, 0, 200);
    SFListInsert(&list, 0, 300);
    SFListInsert(&list, 0, 400);
    SFListInsert(&list, 0, 500);

    SFInteger expected[] = { 500, 400, 300, 200, 100 };
    assert(memcmp(list.items, expected, sizeof(expected)) == 0);

    SFListFinalize(&list);
}

void ListTester::testRemoveAt()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListReserveRange(&list, 0, 5);
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 1, 200);
    SFListSetVal(&list, 2, 300);
    SFListSetVal(&list, 3, 400);
    SFListSetVal(&list, 4, 500);

    /* Test by removing last item. */
    SFListRemoveAt(&list, 4);
    assert(list.count == 4);
    assert(list.items[0] == 100);
    assert(list.items[1] == 200);
    assert(list.items[2] == 300);
    assert(list.items[3] == 400);

    /* Test by removing middle item. */
    SFListRemoveAt(&list, 2);
    assert(list.count == 3);
    assert(list.items[0] == 100);
    assert(list.items[1] == 200);
    assert(list.items[2] == 400);

    /* Test by removing first item. */
    SFListRemoveAt(&list, 0);
    assert(list.count == 2);
    assert(list.items[0] == 200);
    assert(list.items[1] == 400);

    SFListFinalize(&list);
}

void ListTester::testRemoveRange()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));
    SFListReserveRange(&list, 0, 25);

    /* Test by removing items at the end of the list. */
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 19, 200);
    SFListRemoveRange(&list, 20, 5);
    assert(list.count == 20);
    assert(list.items[0] == 100);
    assert(list.items[19] == 200);

    /* Test by removing items at the center of the list. */
    SFListSetVal(&list, 9, 300);
    SFListSetVal(&list, 15, 400);
    SFListRemoveRange(&list, 10, 5);
    assert(list.count == 15);
    assert(list.items[0] == 100);
    assert(list.items[9] == 300);
    assert(list.items[10] == 400);
    assert(list.items[14] == 200);

    /* Test by removing items at the start of the list. */
    SFListSetVal(&list, 5, 500);
    SFListRemoveRange(&list, 0, 5);
    assert(list.count == 10);
    assert(list.items[0] == 500);
    assert(list.items[4] == 300);
    assert(list.items[5] == 400);
    assert(list.items[9] == 200);

    /* Test by removing all items. */
    SFListRemoveRange(&list, 0, 10);
    assert(list.count == 0);

    SFListFinalize(&list);
}

void ListTester::testClear()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListReserveRange(&list, 0, 5);
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 1, 200);
    SFListSetVal(&list, 2, 300);
    SFListSetVal(&list, 3, 400);
    SFListSetVal(&list, 4, 500);

    SFListClear(&list);
    assert(list.count == 0);

    SFListFinalize(&list);
}

void ListTester::testTrimExcess()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));
    SFListSetCapacity(&list, 1024);

    SFListReserveRange(&list, 0, 5);
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 1, 200);
    SFListSetVal(&list, 2, 300);
    SFListSetVal(&list, 3, 400);
    SFListSetVal(&list, 4, 500);

    SFListTrimExcess(&list);
    assert(list.capacity == 5);

    SFListFinalize(&list);
}

void ListTester::testIndexOfItem()
{
    SF_LIST(SFInteger) list;
    SFInteger item;
    SFUInteger index;

    SFListInitialize(&list, sizeof(SFInteger));

    SFListAdd(&list, -500);
    SFListAdd(&list, -400);
    SFListAdd(&list, -300);
    SFListAdd(&list, -200);
    SFListAdd(&list, -100);
    SFListAdd(&list, 0);
    SFListAdd(&list, 100);
    SFListAdd(&list, 200);
    SFListAdd(&list, 300);
    SFListAdd(&list, 400);
    SFListAdd(&list, 500);

    /* Test extreme entries in whole list. */
    item = -500;
    index = SFListIndexOfItem(&list, &item, 0, list.count);
    assert(index == 0);

    item = 500;
    index = SFListIndexOfItem(&list, &item, 0, list.count);
    assert(index == 10);

    /* Test a valid entry in a range. */
    item = 0;
    index = SFListIndexOfItem(&list, &item, 5, 1);
    assert(index == 5);

    /* Test invalid entries in whole list. */
    item = -700;
    index = SFListIndexOfItem(&list, &item, 0, list.count);
    assert(index == SFInvalidIndex);

    item = 700;
    index = SFListIndexOfItem(&list, &item, 0, list.count);
    assert(index == SFInvalidIndex);

    /* Test valid entries not in provided range. */
    item = -300;
    index = SFListIndexOfItem(&list, &item, 0, 2);
    assert(index == SFInvalidIndex);

    item = 300;
    index = SFListIndexOfItem(&list, &item, 9, 2);
    assert(index == SFInvalidIndex);

    SFListFinalize(&list);
}

static int SFIntegerComparison(const void *item1, const void *item2)
{
    SFInteger *ref1 = (SFInteger *)item1;
    SFInteger *ref2 = (SFInteger *)item2;

    return (int)(*ref1 - *ref2);
}

void ListTester::testSort()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListAdd(&list, 300);
    SFListAdd(&list, -100);
    SFListAdd(&list, -500);
    SFListAdd(&list, 400);
    SFListAdd(&list, -300);
    SFListAdd(&list, 0);
    SFListAdd(&list, 500);
    SFListAdd(&list, 200);
    SFListAdd(&list, -200);
    SFListAdd(&list, 100);
    SFListAdd(&list, -400);

    SFListSort(&list, 0, list.count, SFIntegerComparison);

    assert(list.items[0] == -500);
    assert(list.items[1] == -400);
    assert(list.items[2] == -300);
    assert(list.items[3] == -200);
    assert(list.items[4] == -100);
    assert(list.items[5] == 0);
    assert(list.items[6] == 100);
    assert(list.items[7] == 200);
    assert(list.items[8] == 300);
    assert(list.items[9] == 400);
    assert(list.items[10] == 500);
}

void ListTester::test()
{
    testInitialize();
    testSetCapacity();
    testReserveRange();
    testRemoveRange();
    testAdd();
    testInsert();
    testRemoveAt();
    testClear();
    testTrimExcess();
    testIndexOfItem();
    testSort();
}
