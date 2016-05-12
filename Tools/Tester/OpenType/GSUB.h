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

#ifndef __SHEENFIGURE_TESTER__OPEN_TYPE__GSUB_H
#define __SHEENFIGURE_TESTER__OPEN_TYPE__GSUB_H

#include "Common.h"
#include "DataTypes.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

struct GSUB : public Table {
    UInt32 version;                 // Version of the GSUB table-initially set to 0x00010000
    ScriptListTable *scriptList;    // Offset to ScriptList table-from beginning of GSUB table
    FeatureListTable *featureList;  // Offset to FeatureList table-from beginning of GSUB table
    LookupListTable *lookupList;    // Offset to LookupList table-from beginning of GSUB table

    void write(Writer &writer) override {
        writer.enter();

        writer.write(version);
        int scriptListRef = writer.reserveOffset();
        int featureListRef = writer.reserveOffset();
        int lookupListRef = writer.reserveOffset();

        writer.writeTable(scriptList, scriptListRef);
        writer.writeTable(featureList, featureListRef);
        writer.writeTable(lookupList, lookupListRef);
        writer.exit();
    }
};

}
}
}

#endif
