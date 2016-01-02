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

#include <stddef.h>

#include "SFAssert.h"
#include "SFAlbum.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGDEF.h"
#include "SFLocator.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFShapingEngine.h"

static SFUInteger SFBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
{
    SFUInteger min = 0;
    SFUInteger max = length - 1;

    /* The length of array must be greater than zero. */
    SFAssert(length > 0);

    while (min < max) {
        SFUInteger mid = (min + max) >> 1;
        SFUInt16 element = SFUInt16Array_Value(uint16Array, mid);

        if (element < value) {
            min = mid + 1;
        } else if (element > value) {
            max = mid;
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

    while (min < max) {
        SFUInteger mid = (min + max) >> 1;
        SFData range = SFData_Subdata(rangeArray, mid * _SFGlyphRange_Size());
        SFUInt16 start = _SFGlyphRange_Start(range);
        SFUInt16 end = _SFGlyphRange_End(range);

        if (value < start) {
            max = mid;
        } else if (value > end) {
            min = mid + 1;
        } else {
            return mid;
        }
    }

    return SFInvalidIndex;
}

SF_PRIVATE SFUInteger _SFSearchCoverageIndex(SFData coverage, SFGlyphID glyph)
{
    SFUInt16 format;

    /* The coverage table must not be NULL. */
    SFAssert(coverage != NULL);

    format = SFCoverage_Format(coverage);

    switch (format) {
    case 1:
        {
            SFUInt16 glyphCount = SFCoverageF1_GlyphCount(coverage);
            if (glyphCount) {
                SFData glyphArray = SFCoverageF1_GlyphArray(coverage);
                return SFBinarySearchUInt16(glyphArray, glyphCount, glyph);
            }
        }

    case 2:
        {
            SFUInt16 rangeCount = SFCoverageF2_RangeCount(coverage);
            if (rangeCount) {
                SFData rangeRecord = SFCoverageF2_RangeRecord(coverage, 0);
                return _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);
            }
        }
        break;
    }

    return SFInvalidIndex;
}

SF_PRIVATE SFUInt16 _SFSearchGlyphClass(SFData classDef, SFGlyphID glyph)
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
                return SFUInt16Array_Value(classArray, index);
            }
        }
        break;

    case 2:
        {
            SFUInt16 rangeCount = SFClassDefF2_ClassRangeCount(classDef);
            if (rangeCount) {
                SFData rangeRecord = SFClassDefF2_RangeRecordArray(classDef);
                SFUInteger recordIndex = _SFBinarySearchGlyphRange(rangeRecord, rangeCount, glyph);

                if (recordIndex != SFInvalidIndex) {
                    rangeRecord = SFClassDefF2_ClassRangeRecord(classDef, recordIndex);
                    return SFClassRangeRecord_Class(rangeRecord);
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

SF_PRIVATE SFGlyphTrait _SFGetGlyphTrait(SFTextProcessorRef processor, SFGlyphID glyph)
{
    if (processor->_glyphClassDef) {
        SFUInt16 glyphClass = _SFSearchGlyphClass(processor->_glyphClassDef, glyph);
        return _SFGlyphClassToGlyphTrait(glyphClass);
    }

    return SFGlyphTraitNone;
}

SF_PRIVATE SFBoolean _SFApplyExtensionSubtable(SFTextProcessorRef processor, SFData extensionSubtable)
{
    SFUInt16 format = SFExtension_Format(extensionSubtable);

    switch (format) {
    case 1:
        {
            SFLookupType lookupType = SFExtensionF1_LookupType(extensionSubtable);
            SFUInt32 offset = SFExtensionF1_ExtensionOffset(extensionSubtable);
            SFData innerSubtable = SFData_Subdata(extensionSubtable, offset);

            switch (processor->_headerKind) {
            case SFHeaderKindGSUB:
                return _SFApplySubst(processor, lookupType, innerSubtable);

            case SFHeaderKindGPOS:
                return _SFApplyPos(processor, lookupType, innerSubtable);
            }
        }
        break;
    }

    return SFFalse;
}
