/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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
#include <Source/List.h>
}

#include "ListTester.h"

using namespace SheenFigure::Tester;

ListTester::ListTester()
{
}

void ListTester::testInitialize()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    assert(list.items == NULL);
    assert(list.count == 0);
    assert(list.capacity == 0);
}

void ListTester::testSetCapacity()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    /* Test by setting a finite capacity. */
    ListSetCapacity(&list, 1024);
    assert(list.capacity == 1024);

    /* Test by setting zero capacity. */
    ListSetCapacity(&list, 0);
    assert(list.capacity == 0);

    ListFinalize(&list);
}

void ListTester::testReserveRange()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    /* Test by reserving some items. */
    ListReserveRange(&list, 0, 10);
    assert(list.count == 10);

    /* Test by reserving items at the end of the list. */
    ListSetVal(&list, 0, 100);
    ListSetVal(&list, 9, 200);
    ListReserveRange(&list, 10, 5);
    assert(list.count == 15);
    assert(list.items[0] == 100);
    assert(list.items[9] == 200);

    /* Test by reserving items at the center of the list. */
    ListSetVal(&list, 4, 300);
    ListSetVal(&list, 5, 400);
    ListReserveRange(&list, 5, 5);
    assert(list.count == 20);
    assert(list.items[0] == 100);
    assert(list.items[4] == 300);
    assert(list.items[10] == 400);
    assert(list.items[14] == 200);

    /* Test by reserving items at the start of the list. */
    ListSetVal(&list, 19, 500);
    ListReserveRange(&list, 0, 5);
    assert(list.count == 25);
    assert(list.items[5] == 100);
    assert(list.items[9] == 300);
    assert(list.items[15] == 400);
    assert(list.items[19] == 200);
    assert(list.items[24] == 500);

    ListFinalize(&list);
}

void ListTester::testAdd()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    ListAdd(&list, 100);
    ListAdd(&list, 200);
    ListAdd(&list, 300);
    ListAdd(&list, 400);
    ListAdd(&list, 500);

    SFInteger expected[] = { 100, 200, 300, 400, 500 };
    assert(memcmp(list.items, expected, sizeof(expected)) == 0);

    ListFinalize(&list);
}

void ListTester::testInsert()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    ListInsert(&list, 0, 100);
    ListInsert(&list, 0, 200);
    ListInsert(&list, 0, 300);
    ListInsert(&list, 0, 400);
    ListInsert(&list, 0, 500);

    SFInteger expected[] = { 500, 400, 300, 200, 100 };
    assert(memcmp(list.items, expected, sizeof(expected)) == 0);

    ListFinalize(&list);
}

void ListTester::testRemoveAt()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    ListReserveRange(&list, 0, 5);
    ListSetVal(&list, 0, 100);
    ListSetVal(&list, 1, 200);
    ListSetVal(&list, 2, 300);
    ListSetVal(&list, 3, 400);
    ListSetVal(&list, 4, 500);

    /* Test by removing last item. */
    ListRemoveAt(&list, 4);
    assert(list.count == 4);
    assert(list.items[0] == 100);
    assert(list.items[1] == 200);
    assert(list.items[2] == 300);
    assert(list.items[3] == 400);

    /* Test by removing middle item. */
    ListRemoveAt(&list, 2);
    assert(list.count == 3);
    assert(list.items[0] == 100);
    assert(list.items[1] == 200);
    assert(list.items[2] == 400);

    /* Test by removing first item. */
    ListRemoveAt(&list, 0);
    assert(list.count == 2);
    assert(list.items[0] == 200);
    assert(list.items[1] == 400);

    ListFinalize(&list);
}

void ListTester::testRemoveRange()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));
    ListReserveRange(&list, 0, 25);

    /* Test by removing items at the end of the list. */
    ListSetVal(&list, 0, 100);
    ListSetVal(&list, 19, 200);
    ListRemoveRange(&list, 20, 5);
    assert(list.count == 20);
    assert(list.items[0] == 100);
    assert(list.items[19] == 200);

    /* Test by removing items at the center of the list. */
    ListSetVal(&list, 9, 300);
    ListSetVal(&list, 15, 400);
    ListRemoveRange(&list, 10, 5);
    assert(list.count == 15);
    assert(list.items[0] == 100);
    assert(list.items[9] == 300);
    assert(list.items[10] == 400);
    assert(list.items[14] == 200);

    /* Test by removing items at the start of the list. */
    ListSetVal(&list, 5, 500);
    ListRemoveRange(&list, 0, 5);
    assert(list.count == 10);
    assert(list.items[0] == 500);
    assert(list.items[4] == 300);
    assert(list.items[5] == 400);
    assert(list.items[9] == 200);

    /* Test by removing all items. */
    ListRemoveRange(&list, 0, 10);
    assert(list.count == 0);

    ListFinalize(&list);
}

void ListTester::testClear()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    ListReserveRange(&list, 0, 5);
    ListSetVal(&list, 0, 100);
    ListSetVal(&list, 1, 200);
    ListSetVal(&list, 2, 300);
    ListSetVal(&list, 3, 400);
    ListSetVal(&list, 4, 500);

    ListClear(&list);
    assert(list.count == 0);

    ListFinalize(&list);
}

void ListTester::testTrimExcess()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));
    ListSetCapacity(&list, 1024);

    ListReserveRange(&list, 0, 5);
    ListSetVal(&list, 0, 100);
    ListSetVal(&list, 1, 200);
    ListSetVal(&list, 2, 300);
    ListSetVal(&list, 3, 400);
    ListSetVal(&list, 4, 500);

    ListTrimExcess(&list);
    assert(list.capacity == 5);

    ListFinalize(&list);
}

void ListTester::testIndexOfItem()
{
    LIST(SFInteger) list;
    SFInteger item;
    SFUInteger index;

    ListInitialize(&list, sizeof(SFInteger));

    ListAdd(&list, -500);
    ListAdd(&list, -400);
    ListAdd(&list, -300);
    ListAdd(&list, -200);
    ListAdd(&list, -100);
    ListAdd(&list, 0);
    ListAdd(&list, 100);
    ListAdd(&list, 200);
    ListAdd(&list, 300);
    ListAdd(&list, 400);
    ListAdd(&list, 500);

    /* Test extreme entries in whole list. */
    item = -500;
    index = ListIndexOfItem(&list, &item, 0, list.count);
    assert(index == 0);

    item = 500;
    index = ListIndexOfItem(&list, &item, 0, list.count);
    assert(index == 10);

    /* Test a valid entry in a range. */
    item = 0;
    index = ListIndexOfItem(&list, &item, 5, 1);
    assert(index == 5);

    /* Test invalid entries in whole list. */
    item = -700;
    index = ListIndexOfItem(&list, &item, 0, list.count);
    assert(index == SFInvalidIndex);

    item = 700;
    index = ListIndexOfItem(&list, &item, 0, list.count);
    assert(index == SFInvalidIndex);

    /* Test valid entries not in provided range. */
    item = -300;
    index = ListIndexOfItem(&list, &item, 0, 2);
    assert(index == SFInvalidIndex);

    item = 300;
    index = ListIndexOfItem(&list, &item, 9, 2);
    assert(index == SFInvalidIndex);

    ListFinalize(&list);
}

static int SFIntegerComparison(const void *item1, const void *item2)
{
    SFInteger *ref1 = (SFInteger *)item1;
    SFInteger *ref2 = (SFInteger *)item2;

    return (int)(*ref1 - *ref2);
}

void ListTester::testSort()
{
    LIST(SFInteger) list;
    ListInitialize(&list, sizeof(SFInteger));

    ListAdd(&list, 300);
    ListAdd(&list, -100);
    ListAdd(&list, -500);
    ListAdd(&list, 400);
    ListAdd(&list, -300);
    ListAdd(&list, 0);
    ListAdd(&list, 500);
    ListAdd(&list, 200);
    ListAdd(&list, -200);
    ListAdd(&list, 100);
    ListAdd(&list, -400);

    ListSort(&list, 0, list.count, SFIntegerComparison);

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
