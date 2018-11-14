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

#ifndef __SHEENFIGURE_TESTER__OPEN_TYPE__VARIATIONS_H
#define __SHEENFIGURE_TESTER__OPEN_TYPE__VARIATIONS_H

#include "Base.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

struct RegionAxisCoordinatesRecord : public Table {
    F2DOT14 startCoord;     // The region start coordinate value for the current axis.
    F2DOT14 peakCoord;      // The region peak coordinate value for the current axis.
    F2DOT14 endCoord;       // The region end coordinate value for the current axis.

    void write(Writer &writer) override {
        writer.write((UInt16)startCoord);
        writer.write((UInt16)peakCoord);
        writer.write((UInt16)endCoord);
    }
};

struct VariationRegionRecord : public Table {
    RegionAxisCoordinatesRecord *regionAxes; // Array of region axis coordinates records, in the order of axes given in the 'fvar' table.

    void preset(UInt16 axisCount) {
        m_axisCount = axisCount;
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_axisCount; i++) {
            writer.write(&regionAxes[i]);
        }
    }

private:
    UInt16 m_axisCount;
};

struct VariationRegionList : public Table {
    UInt16 axisCount;                        // The number of variation axes for this font. This must be the same number as axisCount in the 'fvar' table.
    UInt16 regionCount;                      // The number of variation region tables in the variation region list.
    VariationRegionRecord *variationRegions; // Array of variation regions.

    void write(Writer &writer) override {
        for (int i = 0; i < regionCount; i++) {
            variationRegions[i].preset(axisCount);
        }

        writer.enter();

        writer.write(axisCount);
        writer.write(regionCount);
        for (int i = 0; i < regionCount; i++) {
            writer.write(&variationRegions[i]);
        }

        writer.exit();
    }
};

struct DeltaSetRecord : public Table {
    Int16 *i16Delta;
    Int8 *i8Delta;

    void preset(UInt16 shortDeltaCount, UInt16 regionIndexCount) {
        m_shortDeltaCount = shortDeltaCount;
        m_regionIndexCount = regionIndexCount;
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_shortDeltaCount; i++) {
            writer.write((UInt16)i16Delta[i]);
        }
        for (int i = 0; i < (m_regionIndexCount - m_shortDeltaCount); i++) {
            writer.write((UInt8)i8Delta[i]);
        }
    }

private:
    UInt16 m_shortDeltaCount;
    UInt16 m_regionIndexCount;
};

struct ItemVariationDataSubtable : public Table {
    UInt16 itemCount;           // The number of delta sets for distinct items.
    UInt16 shortDeltaCount;     // The number of deltas in each delta set that use a 16-bit representation. Must be less than or equal to regionIndexCount.
    UInt16 regionIndexCount;    // The number of variation regions referenced.
    UInt16 *regionIndexes;      // Array of indices into the variation region list for the regions referenced by this item variation data table.
    DeltaSetRecord *deltaSets;  // Delta-set rows.

    void write(Writer &writer) override {
        for (int i = 0; i < itemCount; i++) {
            deltaSets[i].preset(shortDeltaCount, regionIndexCount);
        }

        writer.enter();

        writer.write(itemCount);
        writer.write(shortDeltaCount);
        writer.write(regionIndexCount);
        for (int i = 0; i < regionIndexCount; i++) {
            writer.write(regionIndexes[i]);
        }
        for (int i = 0; i < itemCount; i++) {
            writer.write(&deltaSets[i]);
        }

        writer.exit();
    }
};

struct ItemVariationStoreTable : public Table {
    UInt16 format;                                  // Format — set to 1
    VariationRegionList *variationRegionList;       // Offset in bytes from the start of the item variation store to the variation region list.
    UInt16 itemVariationDataCount;                  // The number of item variation data subtables.
    ItemVariationDataSubtable *itemVariationData;   // Offsets in bytes from the start of the item variation store to each item variation data subtable.

    void write(Writer &writer) override {
        writer.enter();

        writer.write(format);
        writer.defer(variationRegionList, true);
        writer.write(itemVariationDataCount);
        writer.defer(itemVariationData, true);

        writer.exit();
    }
};

}
}
}

#endif
