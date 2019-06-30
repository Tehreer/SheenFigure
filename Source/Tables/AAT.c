/*
 * Copyright (C) 2019 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (dhe "License");
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

#include <SFConfig.h>
#include <stdlib.h>

#include "Data.h"
#include "SFBase.h"
#include "Tables/AAT.h"

static int LookupSegmentComparison(const void *item1, const void *item2)
{
    SFGlyphID *ref1 = (SFGlyphID *)item1;
    SFUInt16 val1 = *ref1;
    Data ref2 = (Data)item2;
    SFUInt16 lastGlyph = LookupSegment_LastGlyph(ref2);
    SFUInt16 firstGlyph = LookupSegment_FirstGlyph(ref2);

    if (val1 < firstGlyph) {
        return -1;
    }
    if (val1 > lastGlyph) {
        return 1;
    }

    return 0;
}

static Data BinarySearchLookupSegment(Data binSrchHeader, Data segmentArray, SFGlyphID glyphID)
{
    SFUInt16 unitSize = BinSrchHeader_UnitSize(binSrchHeader);
    SFUInt16 numUnits = BinSrchHeader_NUnits(binSrchHeader);

    return bsearch(&glyphID, segmentArray, numUnits, unitSize, LookupSegmentComparison);
}

static int LookupSingleComparison(const void *item1, const void *item2)
{
    SFGlyphID *ref1 = (SFGlyphID *)item1;
    SFGlyphID val1 = *ref1;
    Data ref2 = (Data)item2;
    SFUInt16 glyph = LookupSingle_Glyph(ref2);

    return (int)val1 - (int)glyph;
}

static Data BinarySearchLookupSingle(Data binSrchHeader, Data entryArray, SFGlyphID glyphID)
{
    SFUInt16 unitSize = BinSrchHeader_UnitSize(binSrchHeader);
    SFUInt16 numUnits = BinSrchHeader_NUnits(binSrchHeader);

    return bsearch(&glyphID, entryArray, numUnits, unitSize, LookupSingleComparison);
}

SF_INTERNAL Data GetLookupValueData(Data lookupTable, SFGlyphID glyphID, SFUInt16 valueSize)
{
    SFUInt16 format = Lookup_Format(lookupTable);

    switch (format) {
        case 0: {
            Data valueArray = LookupF0_ValueArray(lookupTable);
            return Data_Subdata(valueArray, glyphID * valueSize);
        }

        case 2: {
            Data binSrchHeader = LookupF2_BinSrchHeader(lookupTable);
            Data segmentArray = LookupF2_Segments(lookupTable);
            Data lookupSegment;

            lookupSegment = BinarySearchLookupSegment(binSrchHeader, segmentArray, glyphID);

            if (lookupSegment) {
                return LookupSegment_ValueData(lookupSegment);
            }
            break;
        }

        case 4: {
            Data binSrchHeader = LookupF4_BinSrchHeader(lookupTable);
            Data segmentArray = LookupF4_Segments(lookupTable);
            Data lookupSegment;

            lookupSegment = BinarySearchLookupSegment(binSrchHeader, segmentArray, glyphID);

            if (lookupSegment) {
                SFUInt16 firstGlyph = LookupSegment_FirstGlyph(lookupSegment);
                Data offsetData = LookupSegment_ValueData(lookupSegment);
                SFUInt16 arrayOffset = Data_UInt16(offsetData, 0);
                Data arrayData = Data_Subdata(lookupTable, arrayOffset);
                SFUInt16 valueIndex = glyphID - firstGlyph;

                return Data_Subdata(arrayData, valueIndex * valueSize);
            }
            break;
        }

        case 6: {
            Data binSrchHeader = LookupF6_BinSrchHeader(lookupTable);
            Data entryArray = LookupF6_Entries(lookupTable);
            Data lookupSingle;

            lookupSingle = BinarySearchLookupSingle(binSrchHeader, entryArray, glyphID);

            if (lookupSingle) {
                return LookupSegment_ValueData(lookupSingle);
            }
            break;
        }

        case 8: {
            SFUInt16 firstGlyph = LookupF8_FirstGlyph(lookupTable);
            SFUInt16 glyphCount = LookupF8_GlyphCount(lookupTable);
            Data valueArray = LookupF8_ValueArray(lookupTable);
            SFUInt16 valueIndex = glyphID - firstGlyph;

            if (valueIndex < glyphCount) {
                return Data_Subdata(valueArray, valueIndex * valueSize);
            }
            break;
        }

        case 10: {
            SFUInt16 unitSize = LookupF10_UnitSize(lookupTable);
            SFUInt16 firstGlyph = LookupF10_FirstGlyph(lookupTable);
            SFUInt16 glyphCount = LookupF10_GlyphCount(lookupTable);
            Data valueArray = LookupF10_ValueArray(lookupTable);
            SFUInt16 valueIndex = glyphID - firstGlyph;

            if (valueIndex < glyphCount) {
                return Data_Subdata(valueArray, valueIndex * unitSize);
            }
            break;
        }
    }

    return NULL;
}
