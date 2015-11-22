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
            SFUInt16 element = SF_UINT16_ARRAY__VALUE(uint16Array, mid);

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
            SFData range = SF_DATA__SUBDATA(rangeArray, mid * _SF_GLYPH_RANGE__SIZE());
            SFUInt16 start = _SF_GLYPH_RANGE__START(range);
            SFUInt16 end = _SF_GLYPH_RANGE__END(range);

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

SF_INTERNAL SFUInteger SFOpenTypeSearchGlyphIndex(SFData coverage, SFGlyph glyph)
{
    SFUInt16 format = SF_COVERAGE_FORMAT(coverage);
    switch (format) {
    case 1:
        {
            SFUInt16 glyphCount = SF_COVERAGE_F1__GLYPH_COUNT(coverage);
            SFData glyphArray = SF_COVERAGE_F1__GLYPH_ARRAY(coverage);

            return SFOpenTypeBinarySearchUInt16(glyphArray, glyphCount, glyph);
        }

    case 2:
        {
            SFUInt16 rangeCount = SF_COVERAGE_F2__RANGE_COUNT(coverage);
            SFData rangeRecord = SF_COVERAGE_F2__RANGE_RECORD(coverage, 0);

            return _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFBoolean SFOpenTypeSearchGlyphClass(SFData classDef, SFGlyph glyph, SFUInt16 *glyphClass)
{
    SFUInt16 format = SF_CLASS_DEF_FORMAT(classDef);
    switch (format) {
    case 1:
        {
            SFGlyph startGlyph = SF_CLASS_DEF_F1__START_GLYPH(classDef);
            SFUInt16 glyphCount = SF_CLASS_DEF_F1__GLYPH_COUNT(classDef);
            SFData classArray = SF_CLASS_DEF_F1__CLASS_VALUE_ARRAY(classDef);

            if (glyph >= startGlyph && glyph < (glyphCount - startGlyph)) {
                SFUInteger index = glyph - startGlyph;
                *glyphClass = SF_UINT16_ARRAY__VALUE(classArray, index);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SF_CLASS_DEF_F2__CLASS_RANGE_COUNT(classDef);
            SFData rangeRecord = SF_CLASS_DEF_F2__CLASS_RANGE_RECORD(classDef, 0);
            SFUInteger recordIndex = _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);

            if (recordIndex != SFInvalidIndex) {
                rangeRecord = SF_CLASS_DEF_F2__CLASS_RANGE_RECORD(classDef, recordIndex);
                *glyphClass = SF_CLASS_RANGE_RECORD__CLASS(rangeRecord);

                return SFTrue;
            }
        }
        break;
    }

    return SFFalse;
}
