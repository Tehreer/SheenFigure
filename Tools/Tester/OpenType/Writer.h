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

#ifndef __SHEEN_FIGURE__TESTER__OPEN_TYPE__WRITER_H
#define __SHEEN_FIGURE__TESTER__OPEN_TYPE__WRITER_H

#include <stack>
#include "DataTypes.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

class Writer {
public:
    Writer();
    ~Writer();

    void enter();
    void exit();

    int reserveOffset();
    int reserveLong();
    void writeTable(Table *value, int reference = -1, bool isLong = false);

    void write(UInt8 value);
    void write(UInt16 value);
    void write(UInt32 value);
    void write(Table *array, int count);
    void write(UInt8 *array, int count);
    void write(UInt16 *array, int count);
    void write(UInt32 *array, int count);

    int size() { return m_size; }
    UInt8 *data() { return m_data; };

private:
    UInt8 *m_data;
    int m_capacity;
    int m_size;
    std::stack<int> m_enteries;
    std::stack<int> m_offsets;

    void increaseSize(int size);
};

}
}
}

#endif
