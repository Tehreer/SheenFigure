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

#include <cstddef>
#include <cstdint>
#include <map>
#include <stack>

#include "Base.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

class Writer {
public:
    Writer();
    ~Writer();

    void enter();
    void exit();

    void defer(Table *table, bool largeOffset = false);

    void write(UInt8 value);
    void write(UInt16 value);
    void write(UInt32 value);
    void write(Table *table);
    void write(UInt8 *array, size_t count);
    void write(UInt16 *array, size_t count);
    void write(UInt32 *array, size_t count);

    size_t size() { return m_size; }
    UInt8 *data() { return m_data; };

private:
    struct Deferral {
        size_t entryIndex;
        bool largeOffset;
        size_t reference;
        Table *table;
    };

    UInt8 *m_data;
    size_t m_capacity;
    size_t m_size;
    std::stack<size_t> m_enteries;
    std::stack<Deferral> m_deferrals;

    void increaseSize(size_t size);

    size_t reserveOffset(bool large = false);
    void writeTable(Table *value, size_t reference = SIZE_MAX, bool largeOffset = false);

};

}
}
}

#endif
