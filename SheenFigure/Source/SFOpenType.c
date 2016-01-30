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

#include <stdlib.h>

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFOpenType.h"

static int _SFUInt16Comparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 val2 = SFData_UInt16(ref2, 0);

    return (int)(val1 - val2);
}

SF_INTERNAL SFUInteger SFOpenTypeBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    void *item = bsearch(&value, uint16Array, length, sizeof(SFUInt16), _SFUInt16Comparison);
    if (!item) {
        return SFInvalidIndex;
    }

    return (SFUInteger)((SFData)item - uint16Array) / sizeof(SFUInt16);
}

static int _SFRangeComparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 rangeStart = _SFGlyphRange_Start(ref2);
    SFUInt16 rangeEnd = _SFGlyphRange_End(ref2);

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
    return bsearch(&value, rangeArray, length, _SFGlyphRange_Size(), _SFRangeComparison);
}

SF_INTERNAL SFUInteger SFOpenTypeSearchGlyphIndex(SFData coverage, SFGlyphID glyph)
{
    SFUInt16 format;

    /* The coverage table must NOT be null. */
    SFAssert(coverage != NULL);

    format = SFCoverage_Format(coverage);

    switch (format) {
    case 1:
        {
            SFUInt16 glyphCount = SFCoverageF1_GlyphCount(coverage);
            if (glyphCount) {
                SFData glyphArray = SFCoverageF1_GlyphArray(coverage);
                return SFOpenTypeBinarySearchUInt16(glyphArray, glyphCount, glyph);
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SFCoverageF2_RangeCount(coverage);
            if (rangeCount) {
                SFData rangeArray = SFCoverageF2_RangeRecord(coverage, 0);
                SFData rangeRecord;

                rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyph);

                if (rangeRecord) {
                    SFUInt16 rangeStart = SFRangeRecord_Start(rangeRecord);
                    SFUInt16 startCoverageIndex = SFRangeRecord_StartCoverageIndex(rangeRecord);

                    return (glyph + startCoverageIndex - rangeStart);
                }

                return SFInvalidIndex;
            }
        }
        break;
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFBoolean SFOpenTypeSearchGlyphClass(SFData classDef, SFGlyphID glyph, SFUInt16 *glyphClass)
{
    SFUInt16 format;

    /* The class definition table must not be NULL. */
    SFAssert(classDef != NULL);

    format = SFClassDef_Format(classDef);

    switch (format) {
    case 1:
        {
            SFGlyphID startGlyph = SFClassDefF1_StartGlyph(classDef);
            SFUInt16 glyphCount = SFClassDefF1_GlyphCount(classDef);
            SFUInteger limit = startGlyph + glyphCount;
            SFUInteger index = glyph - startGlyph;

            if (index < limit) {
                SFData classArray = SFClassDefF1_ClassValueArray(classDef);
                *glyphClass = SFUInt16Array_Value(classArray, index);

                return SFTrue;
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SFClassDefF2_ClassRangeCount(classDef);
            if (rangeCount) {
                SFData rangeArray = SFClassDefF2_RangeRecordArray(classDef);
                SFData rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyph);

                if (rangeRecord) {
                    *glyphClass = SFClassRangeRecord_Class(rangeRecord);

                    return SFTrue;
                }
            }
        }
        break;
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;

    return SFFalse;
}