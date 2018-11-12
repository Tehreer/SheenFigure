/*
 * Copyright (C) 2018 Muhammad Tayyab Akram
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
#include <vector>

extern "C" {
#include <Source/OpenType.h>
}

#include "OpenType/Builder.h"
#include "OpenType/Writer.h"
#include "MiscTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

MiscTester::MiscTester()
{
}

void MiscTester::testDevicePixels()
{
    Builder builder;

    /* Test the first format. */
    {
        DeviceTable *device = &builder.createDevice({11, 22}, { -2, 1, -1, 0, 1, 0, -2, -1, 0, 1, -1, -2 });

        Writer writer;
        writer.write(device);

        Data data = writer.data();

        /* Test with out of range values. */
        assert(GetDevicePixels(data, 10) == 0);
        assert(GetDevicePixels(data, 23) == 0);
        /* Test with inside the range values. */
        assert(GetDevicePixels(data, 11) == -2);
        assert(GetDevicePixels(data, 12) == 1);
        assert(GetDevicePixels(data, 13) == -1);
        assert(GetDevicePixels(data, 14) == 0);
        assert(GetDevicePixels(data, 15) == 1);
        assert(GetDevicePixels(data, 16) == 0);
        assert(GetDevicePixels(data, 17) == -2);
        assert(GetDevicePixels(data, 18) == -1);
        assert(GetDevicePixels(data, 19) == 0);
        assert(GetDevicePixels(data, 20) == 1);
        assert(GetDevicePixels(data, 21) == -1);
        assert(GetDevicePixels(data, 22) == -2);
    }

    /* Test the second format. */
    {
        DeviceTable *device = &builder.createDevice({11, 16}, { -8, 0, 7, 1, -1, -7 });

        Writer writer;
        writer.write(device);

        Data data = writer.data();

        /* Test with out of range values. */
        assert(GetDevicePixels(data, 10) == 0);
        assert(GetDevicePixels(data, 17) == 0);
        /* Test with inside the range values. */
        assert(GetDevicePixels(data, 11) == -8);
        assert(GetDevicePixels(data, 12) == 0);
        assert(GetDevicePixels(data, 13) == 7);
        assert(GetDevicePixels(data, 14) == 1);
        assert(GetDevicePixels(data, 15) == -1);
        assert(GetDevicePixels(data, 16) == -7);
    }

    /* Test the third format. */
    {
        DeviceTable *device = &builder.createDevice({11, 13}, { -128, 0, 127 });

        Writer writer;
        writer.write(device);

        Data data = writer.data();

        /* Test with out of range values. */
        assert(GetDevicePixels(data, 10) == 0);
        assert(GetDevicePixels(data, 14) == 0);
        /* Test with inside the range values. */
        assert(GetDevicePixels(data, 11) == -128);
        assert(GetDevicePixels(data, 12) == 0);
        assert(GetDevicePixels(data, 13) == 127);
    }
}

void MiscTester::test()
{
    testDevicePixels();
}
