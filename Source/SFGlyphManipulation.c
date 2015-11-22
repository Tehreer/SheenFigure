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

#include <stddef.h>

#include "SFAssert.h"
#include "SFCollection.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGDEF.h"

#include "SFGlyphManipulation.h"
#include "SFShapingEngine.h"

SF_INTERNAL SFUInteger SFBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    SFUInteger min = 0;
    SFUInteger max = length - 1;

    /* The length of array must be greater than zero. */
    SFAssert(length > 0);

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

    return SFInvalidIndex;
}

static SFUInteger _SFBinarySearchGlyphRange(SFData rangeArray, SFUInteger length, SFUInt16 value)
{
    SFUInteger min = 0;
    SFUInteger max = length - 1;

    /* The length of array must be greater than zero. */
    SFAssert(length > 0);

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

    return SFInvalidIndex;
}

SF_PRIVATE SFUInteger _SFSearchCoverageIndex(SFData coverage, SFGlyph glyph)
{
    SFUInt16 format;

    /* The coverage table must not be NULL. */
    SFAssert(coverage != NULL);

    format = SF_COVERAGE_FORMAT(coverage);

    switch (format) {
    case 1:
        {
            SFUInt16 glyphCount = SF_COVERAGE_F1__GLYPH_COUNT(coverage);
            if (glyphCount) {
                SFData glyphArray = SF_COVERAGE_F1__GLYPH_ARRAY(coverage);
                return SFBinarySearchUInt16(glyphArray, glyphCount, glyph);
            }
        }

    case 2:
        {
            SFUInt16 rangeCount = SF_COVERAGE_F2__RANGE_COUNT(coverage);
            if (rangeCount) {
                SFData rangeRecord = SF_COVERAGE_F2__RANGE_RECORD(coverage, 0);
                return _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);
            }
        }
        break;
    }

    return SFInvalidIndex;
}

SF_PRIVATE SFUInt16 _SFSearchGlyphClass(SFData classDef, SFGlyph glyph)
{
    SFUInt16 format;

    /* The class definition table must not be NULL. */
    SFAssert(classDef != NULL);

    format = SF_CLASS_DEF_FORMAT(classDef);

    switch (format) {
    case 1:
        {
            SFGlyph startGlyph = SF_CLASS_DEF_F1__START_GLYPH(classDef);
            SFUInt16 glyphCount = SF_CLASS_DEF_F1__GLYPH_COUNT(classDef);
            SFUInteger limit = startGlyph + glyphCount;
            SFUInteger index = glyph - startGlyph;

            if (index < limit) {
                SFData classArray = SF_CLASS_DEF_F1__CLASS_VALUE_ARRAY(classDef);
                return SF_UINT16_ARRAY__VALUE(classArray, index);
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SF_CLASS_DEF_F2__CLASS_RANGE_COUNT(classDef);
            if (rangeCount) {
                SFData rangeRecord = SF_CLASS_DEF_F2__RANGE_RECORD_ARRAY(classDef);
                SFUInteger recordIndex = _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);

                if (recordIndex != SFInvalidIndex) {
                    rangeRecord = SF_CLASS_DEF_F2__CLASS_RANGE_RECORD(classDef, recordIndex);
                    return SF_CLASS_RANGE_RECORD__CLASS(rangeRecord);
                }
            }

        }
        break;
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;
}

static SFGlyphTrait _SFGlyphClassToGlyphTrait(SFUInt16 glyphClass)
{
    switch (glyphClass) {
    case SFGlyphClassValueBase:
        return SFGlyphTraitBase;

    case SFGlyphClassValueLigature:
        return SFGlyphTraitLigature;

    case SFGlyphClassValueMark:
        return SFGlyphTraitMark;

    case SFGlyphClassValueComponent:
        return SFGlyphTraitComponent;
    }

    return SFGlyphTraitNone;
}

SF_PRIVATE SFGlyphTrait _SFGetGlyphTrait(SFShapingEngineRef engine, SFGlyph glyph)
{
    if (engine->_glyphClassDef) {
        SFUInt16 glyphClass = _SFSearchGlyphClass(engine->_glyphClassDef, glyph);
        return _SFGlyphClassToGlyphTrait(glyphClass);
    }

    return SFGlyphTraitNone;
}
