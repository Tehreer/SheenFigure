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

#include <cstddef>

#include "ListTester.h"

extern "C" {
#include <SFAssert.h>
#include <SFList.h>
}

using namespace SheenFigure::Tester;

ListTester::ListTester()
{
}

void ListTester::testInitialize()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFAssert(list.items == NULL);
    SFAssert(list.count == 0);
    SFAssert(list.capacity == 0);
}

void ListTester::testSetCapacity()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListSetCapacity(&list, 1024);
    SFAssert(list.capacity == 1024);

    SFListSetCapacity(&list, 0);
    SFAssert(list.capacity == 0);

    SFListFinalize(&list);
}

void ListTester::testReserveRange()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    /* Test by reserving some items. */
    SFListReserveRange(&list, 0, 10);
    SFAssert(list.count == 10);

    /* Test by reserving items at the end of the list. */
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 9, 200);
    SFListReserveRange(&list, 10, 5);
    SFAssert(list.count == 15);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[9] == 200);

    /* Test by reserving items at the center of the list. */
    SFListSetVal(&list, 4, 300);
    SFListSetVal(&list, 5, 400);
    SFListReserveRange(&list, 5, 5);
    SFAssert(list.count == 20);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[4] == 300);
    SFAssert(list.items[10] == 400);
    SFAssert(list.items[14] == 200);

    /* Test by reserving items at the start of the list. */
    SFListSetVal(&list, 19, 500);
    SFListReserveRange(&list, 0, 5);
    SFAssert(list.count == 25);
    SFAssert(list.items[5] == 100);
    SFAssert(list.items[9] == 300);
    SFAssert(list.items[15] == 400);
    SFAssert(list.items[19] == 200);
    SFAssert(list.items[24] == 500);

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
    SFAssert(list.count == 20);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[19] == 200);

    /* Test by removing items at the center of the list. */
    SFListSetVal(&list, 9, 300);
    SFListSetVal(&list, 15, 400);
    SFListRemoveRange(&list, 10, 5);
    SFAssert(list.count == 15);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[9] == 300);
    SFAssert(list.items[10] == 400);
    SFAssert(list.items[14] == 200);

    /* Test by removing items at the start of the list. */
    SFListSetVal(&list, 5, 500);
    SFListRemoveRange(&list, 0, 5);
    SFAssert(list.count == 10);
    SFAssert(list.items[0] == 500);
    SFAssert(list.items[4] == 300);
    SFAssert(list.items[5] == 400);
    SFAssert(list.items[9] == 200);

    /* Test by removing all items. */
    SFListRemoveRange(&list, 0, 10);
    SFAssert(list.count == 0);

    SFListFinalize(&list);
}

void ListTester::testSet()
{
    SF_LIST(SFInteger) list;
    SFListInitialize(&list, sizeof(SFInteger));

    SFListReserveRange(&list, 0, 5);
    SFListSetVal(&list, 0, 100);
    SFListSetVal(&list, 1, 200);
    SFListSetVal(&list, 2, 300);
    SFListSetVal(&list, 3, 400);
    SFListSetVal(&list, 4, 500);

    SFAssert(list.items[0] == 100);
    SFAssert(list.items[1] == 200);
    SFAssert(list.items[2] == 300);
    SFAssert(list.items[3] == 400);
    SFAssert(list.items[4] == 500);

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

    SFAssert(list.items[0] == 100);
    SFAssert(list.items[1] == 200);
    SFAssert(list.items[2] == 300);
    SFAssert(list.items[3] == 400);
    SFAssert(list.items[4] == 500);

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

    SFAssert(list.items[0] == 500);
    SFAssert(list.items[1] == 400);
    SFAssert(list.items[2] == 300);
    SFAssert(list.items[3] == 200);
    SFAssert(list.items[4] == 100);

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

    SFListRemoveAt(&list, 4);
    SFAssert(list.count == 4);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[1] == 200);
    SFAssert(list.items[2] == 300);
    SFAssert(list.items[3] == 400);

    SFListRemoveAt(&list, 2);
    SFAssert(list.count == 3);
    SFAssert(list.items[0] == 100);
    SFAssert(list.items[1] == 200);
    SFAssert(list.items[2] == 400);

    SFListRemoveAt(&list, 0);
    SFAssert(list.count == 2);
    SFAssert(list.items[0] == 200);
    SFAssert(list.items[1] == 400);

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
    SFAssert(list.count == 0);

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
    SFAssert(list.capacity == 5);

    SFListFinalize(&list);
}

void ListTester::test()
{
    testInitialize();
    testSetCapacity();
    testReserveRange();
    testRemoveRange();
    testSet();
    testAdd();
    testInsert();
    testRemoveAt();
    testClear();
    testTrimExcess();
}
