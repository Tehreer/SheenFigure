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

#ifndef __SHEENFIGURE_TESTER__LIST_TESTER_H
#define __SHEENFIGURE_TESTER__LIST_TESTER_H

namespace SheenFigure {
namespace Tester {

class ListTester {
private:

public:
    ListTester();

    void testInitialize();
    void testSetCapacity();
    void testReserveRange();
    void testRemoveRange();
    void testSet();
    void testAdd();
    void testInsert();
    void testRemoveAt();
    void testClear();
    void testTrimExcess();

    void test();
};

}
}

#endif
