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

#include <SFConfig.h>
#include <SFTypes.h>

#include "SFCommon.h"
#include "SFData.h"
#include "SFOpenType.h"

SF_INTERNAL SFUInteger SFOpenTypeBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    if (length) {
        SFUInteger min = 0;
        SFUInteger max = length - 1;

        while (min <= max) {
            SFUInteger mid = (min + max) >> 1;
            SFUInt16 element = SFUInt16Array_Value(uint16Array, mid);

            if (element < value) {
                min = mid + 1;
            } else if (element > value) {
                max = mid - 1;
            } else {
                return mid;
            }
        }
    }

    return SFInvalidIndex;
}

static SFUInteger _SFBinarySearchGlyphRange(SFData rangeArray, SFUInteger length, SFUInt16 value)
{
    if (length) {
        SFUInteger min = 0;
        SFUInteger max = length - 1;

        while (min <= max) {
            SFUInteger mid = (min + max) >> 1;
            SFData range = SFData_Subdata(rangeArray, mid * _SFGlyphRange_Size());
            SFUInt16 start = _SFGlyphRange_Start(range);
            SFUInt16 end = _SFGlyphRange_End(range);

            if (start < value) {
                min = mid + 1;
            } else if (end > value) {
                max = mid - 1;
            } else {
                return mid;
            }
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFUInteger SFOpenTypeSearchGlyphIndex(SFData coverage, SFGlyphID glyph)
{
    SFUInt16 format = SFCoverage_Format(coverage);
    switch (format) {
    case 1:
        {
            SFUInt16 glyphCount = SFCoverageF1_GlyphCount(coverage);
            SFData glyphArray = SFCoverageF1_GlyphArray(coverage);

            return SFOpenTypeBinarySearchUInt16(glyphArray, glyphCount, glyph);
        }

    case 2:
        {
            SFUInt16 rangeCount = SFCoverageF2_RangeCount(coverage);
            SFData rangeRecord = SFCoverageF2_RangeRecord(coverage, 0);

            return _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFBoolean SFOpenTypeSearchGlyphClass(SFData classDef, SFGlyphID glyph, SFUInt16 *glyphClass)
{
    SFUInt16 format = SFClassDef_Format(classDef);
    switch (format) {
    case 1:
        {
            SFGlyphID startGlyph = SFClassDefF1_StartGlyph(classDef);
            SFUInt16 glyphCount = SFClassDefF1_GlyphCount(classDef);
            SFData classArray = SFClassDefF1_ClassValueArray(classDef);

            if (glyph >= startGlyph && glyph < (glyphCount - startGlyph)) {
                SFUInteger index = glyph - startGlyph;
                *glyphClass = SFUInt16Array_Value(classArray, index);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SFClassDefF2_ClassRangeCount(classDef);
            SFData rangeRecord = SFClassDefF2_ClassRangeRecord(classDef, 0);
            SFUInteger recordIndex = _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);

            if (recordIndex != SFInvalidIndex) {
                rangeRecord = SFClassDefF2_ClassRangeRecord(classDef, recordIndex);
                *glyphClass = SFClassRangeRecord_Class(rangeRecord);

                return SFTrue;
            }
        }
        break;
    }

    return SFFalse;
}
