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

#ifndef __SHEEN_FIGURE__TESTER__UTILITIES__CONVERTER_H
#define __SHEEN_FIGURE__TESTER__UTILITIES__CONVERTER_H

extern "C" {
#include <Source/SFJoiningType.h>
}

namespace SheenFigure {
namespace Tester {
namespace Utilities {

class Convert {
public:
    static char toChar(SFJoiningType joiningType);
};

}
}
}

#endif
