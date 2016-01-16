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
#include <stdlib.h>

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

static SFBoolean _SFApplyChainContextF3(SFTextProcessorRef processor, SFData chainContext);
static void _SFApplyContextRecord(SFTextProcessorRef processor, SFData contextRecord, SFUInteger startIndex, SFUInteger endIndex);

static int _SFUInt16Comparison(const void *item1, const void *item2)
{
    SFUInt16 *ref1 = (SFUInt16 *)item1;
    SFUInt16 val1 = *ref1;
    SFData ref2 = (SFData)item2;
    SFUInt16 val2 = SFData_UInt16(ref2, 0);

    return (int)(val1 - val2);
}

static SFUInteger SFBinarySearchUInt16(SFData uint16Array, SFUInteger length, SFUInt16 value)
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
                SFData rangeArray = SFClassDefF2_RangeRecordArray(classDef);
                SFData rangeRecord = _SFBinarySearchGlyphRange(rangeArray, rangeCount, glyph);

                if (rangeRecord) {
                    return SFClassRangeRecord_Class(rangeRecord);
                }
            }
        }
        break;
    }

    /* A glyph not assigned a class value falls into Class 0. */
    return 0;
}

static SFGlyphTraits _SFGlyphClassToGlyphTrait(SFUInt16 glyphClass)
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

    return 0;
}

SF_PRIVATE SFGlyphTraits _SFGetGlyphTraits(SFTextProcessorRef processor, SFGlyphID glyph)
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

            switch (processor->_featureKind) {
            case SFFeatureKindSubstitution:
                return _SFApplySubstitutionSubtable(processor, lookupType, innerSubtable);

            case SFFeatureKindPositioning:
                return _SFApplyPositioningSubtable(processor, lookupType, innerSubtable);
            }
        }
        break;
    }

    return SFFalse;
}

SF_PRIVATE SFBoolean _SFApplyChainContextSubtable(SFTextProcessorRef processor, SFData chainContext)
{
    SFUInt16 format = SFChainContext_Format(chainContext);

    switch (format) {
    case 3:
        return _SFApplyChainContextF3(processor, chainContext);
    }

    return SFFalse;
}

static SFBoolean _SFApplyChainContextF3(SFTextProcessorRef processor, SFData chainContext)
{
    SFLocatorRef locator = &processor->_locator;
    SFAlbumRef album = processor->_album;
    SFData contextRecord;
    SFUInt16 recordCount;
    SFUInteger inputIndex;
    SFUInteger recordIndex;
    SFUInteger lastInput;

    contextRecord = SFChainContextF3_BacktrackRecord(chainContext);
    recordCount = SFBacktrackRecord_GlyphCount(contextRecord);
    inputIndex = locator->index;

    /* Match the backtrack glyphs. */
    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        inputIndex = SFLocatorGetBefore(locator, inputIndex, locator->lookupFlag);
        if (inputIndex != SFInvalidIndex) {
            SFOffset offset = SFBacktrackRecord_Value(contextRecord, recordIndex);
            SFData coverage = SFData_Subdata(chainContext, offset);
            SFGlyphID inputGlyph;
            SFUInteger coverageIndex;

            inputGlyph = SFAlbumGetGlyph(album, inputIndex);
            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex == SFInvalidIndex) {
                return SFFalse;
            }
        }
    }

    contextRecord = SFBacktrackRecord_InputRecord(contextRecord, recordCount);
    recordCount = SFInputRecord_GlyphCount(contextRecord);
    inputIndex = locator->index;

    /* Match the input glyphs. */
    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        inputIndex = SFLocatorGetAfter(locator, inputIndex, locator->lookupFlag);
        if (inputIndex != SFInvalidIndex) {
            SFOffset offset = SFInputRecord_Value(contextRecord, recordIndex);
            SFData coverage = SFData_Subdata(chainContext, offset);
            SFGlyphID inputGlyph;
            SFUInteger coverageIndex;

            inputGlyph = SFAlbumGetGlyph(album, inputIndex);
            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex == SFInvalidIndex) {
                return SFFalse;
            }
        }
    }

    lastInput = inputIndex;
    contextRecord = SFInputRecord_LookaheadRecord(contextRecord, recordCount);
    recordCount = SFInputRecord_GlyphCount(contextRecord);

    /* Match the lookahead glyphs. */
    for (recordIndex = 0; recordIndex < recordCount; recordIndex++) {
        inputIndex = SFLocatorGetAfter(locator, inputIndex, locator->lookupFlag);
        if (inputIndex != SFInvalidIndex) {
            SFOffset offset = SFLookaheadRecord_Value(contextRecord, recordIndex);
            SFData coverage = SFData_Subdata(chainContext, offset);
            SFGlyphID inputGlyph;
            SFUInteger coverageIndex;

            inputGlyph = SFAlbumGetGlyph(album, inputIndex);
            coverageIndex = _SFSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex == SFInvalidIndex) {
                return SFFalse;
            }
        }
    }

    contextRecord = SFLookaheadRecord_ContextRecord(contextRecord, recordCount);
    _SFApplyContextRecord(processor, contextRecord, inputIndex, lastInput);

    return SFTrue;
}

static void _SFApplyContextRecord(SFTextProcessorRef processor, SFData contextRecord, SFUInteger startIndex, SFUInteger endIndex) {
    SFLocator previousLocator = processor->_locator;
    SFLocator contextLocator;
    SFUInt16 lookupCount;
    SFUInteger index;

    lookupCount = SFContextRecord_LookupCount(contextRecord);

    for (index = 0; index < lookupCount; index++) {
        SFData lookupRecord = SFContextRecord_LookupRecord(contextRecord, index);
        SFUInt16 lookupIndex = SFLookupRecord_SequenceIndex(lookupRecord);
        SFUInt16 sequenceIndex = SFLookupRecord_LookupListIndex(lookupRecord);

        contextLocator = previousLocator;
        SFLocatorReset(&contextLocator, startIndex, (endIndex + 1) - startIndex);
        if (SFLocatorSkip(&contextLocator, sequenceIndex)) {
            _SFApplyLookup(processor, lookupIndex);
        }
    }

    processor->_locator = previousLocator;
}
