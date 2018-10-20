/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#include <SFConfig.h>
#include <stdlib.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFOpenType.h"

static int _SFUInt16ItemsComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 val2 = SFData_UInt16(ref2, 0);

    return (int)val1 - (int)val2;
}

static SFUInteger _SFBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    void *item = bsearch(&value, uint16Array, length, sizeof(SFUInt16), _SFUInt16ItemsComparison);
    if (!item) {
        return SFInvalidIndex;
    }

    return (SFUInteger)((SFData)item - uint16Array) / sizeof(SFUInt16);
}

static int _SFGlyphRangeComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 rangeStart = SFGlyphRange_Start(ref2);
    SFUInt16 rangeEnd = SFGlyphRange_End(ref2);

    if (val1 < rangeStart) {
        return -1;
    }

    if (val1 > rangeEnd) {
        return 1;
    }

    return 0;
}

static SFData _SFBinarySearchGlyphRange(SFData rangeArray, SFUInteger length, SFUInt16 value)
{
    return bsearch(&value, rangeArray, length, SFGlyphRange_Size(), _SFGlyphRangeComparison);
}

SF_INTERNAL SFUInteger SFOpenTypeSearchCoverageIndex(SFData coverageTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The coverage table must NOT be null. */
    SFAssert(coverageTable != NULL);

    format = SFCoverage_Format(coverageTable);

    switch (format) {
        case 1: {
            SFUInt16 glyphCount = SFCoverageF1_GlyphCount(coverageTable);
            SFData glyphArray = SFCoverageF1_GlyphArray(coverageTable);

            return _SFBinarySearchUInt16(glyphArray, glyphCount, glyphID);
        }

        case 2: {
            SFUInt16 rangeCount = SFCoverageF2_RangeCount(coverageTable);
            SFData rangeArray = SFCoverageF2_GlyphRangeArray(coverageTable);
            SFData rangeRecord;

            rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                SFUInt16 startGlyphID = SFRangeRecord_StartGlyphID(rangeRecord);
                SFUInt16 startCoverageIndex = SFRangeRecord_StartCoverageIndex(rangeRecord);

                return (SFUInteger)(startCoverageIndex) + (glyphID - startGlyphID);
            }
            break;
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFUInt16 SFOpenTypeSearchGlyphClass(SFData classDefTable, SFGlyphID glyphID)
{
    SFUInt16 format;

    /* The class definition table must NOT be null. */
    SFAssert(classDefTable != NULL);

    format = SFClassDef_Format(classDefTable);

    switch (format) {
        case 1: {
            SFGlyphID startGlyphID = SFClassDefF1_StartGlyphID(classDefTable);
            SFUInt16 glyphCount = SFClassDefF1_GlyphCount(classDefTable);
            SFUInteger valueIndex;

            valueIndex = glyphID - startGlyphID;

            if (valueIndex < glyphCount) {
                SFData classArray = SFClassDefF1_ClassValueArray(classDefTable);
                return SFUInt16Array_Value(classArray, valueIndex);
            }
            break;
        }

        case 2: {
            SFUInt16 rangeCount = SFClassDefF2_ClassRangeCount(classDefTable);
            SFData rangeArray = SFClassDefF2_GlyphRangeArray(classDefTable);
            SFData rangeRecord;

            rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyphID);

            if (rangeRecord) {
                return SFClassRangeRecord_Class(rangeRecord);
            }
            break;
        }
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;
}

SF_INTERNAL SFInt32 SFOpenTypeGetDevicePixels(SFData deviceTable, SFUInt16 ppemSize)
{
    SFUInt16 startSize = SFDevice_StartSize(deviceTable);
    SFUInt16 endSize = SFDevice_EndSize(deviceTable);
    SFUInt16 deltaFormat = SFDevice_DeltaFormat(deviceTable);

    if (ppemSize >= startSize && ppemSize <= endSize) {
        SFUInt16 sizeIndex = ppemSize - startSize;

        switch (deltaFormat) {
            /* Signed 2-bit value, 8 values per UInt16 */
            case 1: {
                SFUInt16 valueIndex = sizeIndex >> 3;
                SFUInt16 leftShift = 16 + (2 * (sizeIndex & 0x0007));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 30;
            }

            /* Signed 4-bit value, 4 values per UInt16 */
            case 2: {
                SFUInt16 valueIndex = sizeIndex >> 2;
                SFUInt16 leftShift = 16 + (4 * (sizeIndex & 0x0003));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 28;
            }

            /* Signed 8-bit value, 2 values per UInt16 */
            case 3: {
                SFUInt16 valueIndex = sizeIndex >> 1;
                SFUInt16 leftShift = 16 + (8 * (sizeIndex & 0x0001));
                SFUInt16 deltaValue = SFDevice_DeltaValue(deviceTable, valueIndex);

                return (SFInt32)(deltaValue << leftShift) >> 24;
            }
        }
    }

    return 0;
}
