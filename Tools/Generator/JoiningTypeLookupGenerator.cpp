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

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Utilities/Math.h"
#include "Utilities/Converter.h"
#include "Utilities/ArrayBuilder.h"
#include "Utilities/FileBuilder.h"

#include "JoiningTypeLookupGenerator.h"

using namespace std;
using namespace SheenFigure::Parser;
using namespace SheenFigure::Generator;
using namespace SheenFigure::Generator::Utilities;

static const char UNAVAILABLE_JOINING_TYPE = 'F';

static const string BYTE_MACRO = "BYTE";
static const string JOINING_TYPE_ARRAY_TYPE = "static const SFUInt8";
static const string JOINING_TYPE_ARRAY_NAME = "_SFJoiningTypeData";

class ByteElement {
private:
    ArrayBuilder &_builder;
    size_t _index;
    int _count;
    string _v1;
    string _v2;

public:
    ByteElement(ArrayBuilder &builder) :
        _builder(builder),
        _index(0),
        _count(0),
        _v1("0"),
        _v2("0")
    {
    }

    bool add(char val) {
        switch(_count) {
        case 0:
            _v1[0] = val;
            _count = 1;
            return false;

        case 1:
            _v2[0] = val;
            _builder.appendElement(BYTE_MACRO + "(" + _v1 + ", " + _v2 + ")");
            _index++;
            _count = 0;
            return true;
        }

        return false;
    }

    void fill() {
        if (_count == 1) {
            add(UNAVAILABLE_JOINING_TYPE);
        }
    }

    bool empty() {
        return (_count == 0);
    }

    size_t index() {
        return _index;
    }
};

struct Range {
    uint32_t start;
    uint32_t end;
    size_t index;
};

class Lookup {
private:
    TextBuilder &_builder;
    const vector<Range> &_ranges;

    bool write(int min, int max, size_t tabs) {
        if (min <= max) {
            int mid = (min + max) / 2;
            const Range &range = _ranges[mid];

            _builder.appendTabs(tabs).append("if (codepoint <= 0x" + Converter::toHex(range.end, 4) + ") {").newLine();
            if (!write(min, mid - 1, tabs + 1)) {
                _builder.appendTabs(tabs + 1).append("diff = 0x" + Converter::toHex(range.end, 4) + " - codepoint;").newLine();
                _builder.appendTabs(tabs + 1).append("block = 0x" + Converter::toHex(range.index, 4) + ";").newLine();
            }
            _builder.appendTabs(tabs).append("} else {").newLine();
            if (!write(mid + 1, max, tabs + 1)) {
                _builder.appendTabs(tabs + 1).append("diff = 0x" + Converter::toHex(range.end, 4) + " - codepoint;").newLine();
                _builder.appendTabs(tabs + 1).append("block = 0x" + Converter::toHex(_ranges[mid + 1].index, 4) + ";").newLine();
            }
            _builder.appendTabs(tabs).append("}").newLine();

            return true;
        }

        return false;
    }

public:
    Lookup(TextBuilder &builder, const vector<Range> &ranges) :
        _builder(builder),
        _ranges(ranges)
    {
    }

    void write(size_t tabs) {
        _builder.appendTabs(tabs).append("SFUInteger diff;").newLine();
        _builder.appendTabs(tabs).append("SFUInteger block;").newLine();
        _builder.appendTabs(tabs).append("SFUInt8 value;").newLine();
        _builder.newLine();
        write(0, (int)(_ranges.size() - 2), tabs);
        _builder.newLine();
        _builder.appendTabs(tabs).append("value = " + JOINING_TYPE_ARRAY_NAME + "[block - (diff >> 1)];").newLine();
        _builder.newLine();
        _builder.appendTabs(tabs).append("if (diff & 1) {").newLine();
        _builder.appendTabs(tabs + 1).append("return (SFJoiningType)(value & 0xF);").newLine();
        _builder.appendTabs(tabs).append("}").newLine();
        _builder.newLine();
        _builder.appendTabs(tabs).append("return (SFJoiningType)(value >> 4);").newLine();
    }
};

JoiningTypeLookupGenerator::JoiningTypeLookupGenerator(const ArabicShaping &arabicShaping) :
    m_arabicShaping(arabicShaping)
{
}

void JoiningTypeLookupGenerator::setAllowedGap(size_t allowedGap) {
    m_allowedGap = allowedGap;
}

void JoiningTypeLookupGenerator::displayGaps() {
    uint32_t first = m_arabicShaping.firstCodePoint();
    uint32_t last = m_arabicShaping.lastCodePoint();
    uint32_t previous = first;

    for (uint32_t codepoint = previous + 1; codepoint < last; codepoint++) {
        char joiningType = m_arabicShaping.joiningTypeForCodePoint(codepoint);
        if (joiningType != '\0') {
            if (previous < (codepoint - 1)) {
                cout << "Gap: " << ((codepoint - 1) - previous) << ", "
                << "[0x" << Converter::toHex(previous) << ", 0x" << Converter::toHex(codepoint) << "]" << endl;
            }

            previous = codepoint;
        }
    }
}

void JoiningTypeLookupGenerator::generateFile(const std::string &directory) {
    TextBuilder byteMacro;
    byteMacro.append("#define " + BYTE_MACRO + "(l, h)  \\").newLine();
    byteMacro.append("(SFUInt8)           \\").newLine();
    byteMacro.append("(                   \\").newLine();
    byteMacro.append("   ((l & 0xF) << 4) \\").newLine();
    byteMacro.append(" | ((h & 0xF) << 0) \\").newLine();
    byteMacro.append(")").newLine();

    ArrayBuilder arrJoiningTypes;
    arrJoiningTypes.setDataType(JOINING_TYPE_ARRAY_TYPE);
    arrJoiningTypes.setName(JOINING_TYPE_ARRAY_NAME);

    set<char> joiningTypes;
    joiningTypes.insert(UNAVAILABLE_JOINING_TYPE);

    vector<Range> ranges;
    uint32_t first = m_arabicShaping.firstCodePoint();
    uint32_t last = m_arabicShaping.lastCodePoint();
    uint32_t previous = first;
    uint32_t block = first;
    ByteElement element(arrJoiningTypes);

    for (uint32_t codepoint = previous + 1; codepoint < last; codepoint++) {
        char joiningType = m_arabicShaping.joiningTypeForCodePoint(codepoint);
        if (joiningType != '\0') {
            joiningTypes.insert(joiningType);

            if (previous < (codepoint - 1)) {
                size_t gap = (codepoint - 1) - previous;
                if (gap < m_allowedGap) {
                    for (size_t j = 0; j < gap; j++) {
                        if (element.add(UNAVAILABLE_JOINING_TYPE)) {
                            arrJoiningTypes.newElement();
                        }
                    }
                } else {
                    if (!element.empty()) {
                        element.fill();
                        arrJoiningTypes.newElement();
                    }
                    ranges.push_back({ block, previous, element.index()});

                    arrJoiningTypes.newLine();
                    arrJoiningTypes.append("#define " + JOINING_TYPE_ARRAY_NAME + "_" + Converter::toHex(element.index(), 4)).newLine();

                    block = codepoint;
                }
            }

            if (element.add(joiningType)) {
                if (codepoint < (last - 1)) {
                    arrJoiningTypes.newElement();
                }
            }
            previous = codepoint;
        }
    }
    element.fill();
    ranges.push_back({ block, previous, element.index() });

    TextBuilder lookupFunction;
    Lookup lookup(lookupFunction, ranges);
    lookupFunction.append("SF_INTERNAL SFJoiningType SFJoiningTypeDetermine(SFCodepoint codepoint) {").newLine();
    lookup.write(1);
    lookupFunction.append("}").newLine();

    FileBuilder header(directory + "/SFJoiningTypeLookup.h");
    header.append("/*").newLine();
    header.append(" * Automatically generated by SheenFigureGenerator tool. ").newLine();
    header.append(" * DO NOT EDIT!!").newLine();
    header.append(" */").newLine();
    header.newLine();
    header.append("#ifndef _SF_JOINING_TYPE_LOOKUP_H").newLine();
    header.append("#define _SF_JOINING_TYPE_LOOKUP_H").newLine();
    header.newLine();
    header.append("#include <SFConfig.h>").newLine();
    header.append("#include <SFTypes.h>").newLine();
    header.newLine();
    header.append("#include \"SFJoiningType.h\"").newLine();
    header.newLine();
    header.append("SF_INTERNAL SFJoiningType SFJoiningTypeDetermine(SFCodepoint codepoint);").newLine();
    header.newLine();
    header.append("#endif").newLine();

    FileBuilder source(directory + "/SFJoiningTypeLookup.c");
    source.append("/*").newLine();
    source.append(" * Automatically generated by SheenFigureGenerator tool. ").newLine();
    source.append(" * DO NOT EDIT!!").newLine();
    source.append(" */").newLine();
    source.newLine();
    source.append("#include <SFConfig.h>").newLine();
    source.append("#include <SFTypes.h>").newLine();
    source.newLine();
    source.append("#include \"SFJoiningType.h\"").newLine();
    source.append("#include \"SFJoiningTypeLookup.h\"").newLine();
    source.newLine();
    source.append(byteMacro).newLine();

    for (char n : joiningTypes) {
        string upper(1, toupper(n));
        source.append("#define " + upper).appendTab().append(" SFJoiningType" + upper).newLine();
    }
    source.newLine();

    source.append(arrJoiningTypes).newLine();
    source.append(lookupFunction).newLine();

    source.append("#undef " + BYTE_MACRO).newLine();
    for (char n : joiningTypes) {
        string upper(1, toupper(n));
        source.append("#undef " + upper).newLine();
    }
}
