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

#include <stddef.h>
#include <stdlib.h>

#include "SFAssert.h"
#include "SFAlbum.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGDEF.h"
#include "SFLocator.h"
#include "SFOpenType.h"

#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

enum {
    _SFGlyphZoneInput = 0,
    _SFGlyphZoneBacktrack = 1,
    _SFGlyphZoneLookahead = 2
};
typedef SFUInt8 _SFGlyphZone;

typedef struct {
    void *helperPtr;
    SFUInt16 recordValue;
    SFGlyphID glyphID;
    _SFGlyphZone glyphZone;
} _SFGlyphAgent;

typedef SFBoolean (*_SFGlyphAssessment)(_SFGlyphAgent *glyphAgent);

static SFBoolean _SFApplyRuleSetTable(SFTextProcessorRef textProcessor,
    SFData ruleSetTable, _SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean _SFApplyRuleTable(SFTextProcessorRef textProcessor,
    SFData ruleTable, SFBoolean includeFirst, _SFGlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean _SFApplyChainRuleSetTable(SFTextProcessorRef textProcessor,
    SFData chainRuleSetTable, _SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean _SFApplyChainRuleTable(SFTextProcessorRef textProcessor,
    SFData chainRuleTable, SFBoolean includeFirst, _SFGlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean _SFApplyContextLookups(SFTextProcessorRef textProcessor,
    SFData lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextEnd);

static SFBoolean _SFAssessGlyphByEquality(_SFGlyphAgent *glyphAgent)
{
    return (glyphAgent->glyphID == glyphAgent->recordValue);
}

static SFBoolean _SFAssessGlyphByClass(_SFGlyphAgent *glyphAgent)
{
    SFData classDefTable = NULL;
    SFUInt16 glyphClass;

    switch (glyphAgent->glyphZone) {
        case _SFGlyphZoneInput:
            classDefTable = ((SFData *)glyphAgent->helperPtr)[0];
            break;

        case _SFGlyphZoneBacktrack:
            classDefTable = ((SFData *)glyphAgent->helperPtr)[1];
            break;

        case _SFGlyphZoneLookahead:
            classDefTable = ((SFData *)glyphAgent->helperPtr)[2];
            break;
    }

    glyphClass = SFOpenTypeSearchGlyphClass(classDefTable, glyphAgent->glyphID);

    return (glyphClass == glyphAgent->recordValue);
}

static SFBoolean _SFAssessGlyphByCoverage(_SFGlyphAgent *glyphAgent)
{
    SFData subtable = glyphAgent->helperPtr;
    SFData coverageTable;
    SFUInteger coverageIndex;

    coverageTable = SFData_Subdata(subtable, glyphAgent->recordValue);
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, glyphAgent->glyphID);

    return (coverageIndex != SFInvalidIndex);
}

static SFBoolean _SFAssessBacktrackGlyphs(SFTextProcessorRef textProcessor,
    SFData valueArray, SFUInteger valueCount, _SFGlyphAssessment glyphAssessment, void *helperPtr)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger backtrackIndex = locator->index;
    SFUInteger valueIndex;
    _SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = _SFGlyphZoneBacktrack;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        backtrackIndex = SFLocatorGetBefore(locator, backtrackIndex);

        if (backtrackIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, backtrackIndex);
            glyphAgent.recordValue = SFUInt16Array_Value(valueArray, valueIndex);

            if (!glyphAssessment(&glyphAgent)) {
                return SFFalse;
            }
        } else {
            return SFFalse;
        }
    }

    return SFTrue;
}

static SFBoolean _SFAssessInputGlyphs(SFTextProcessorRef textProcessor,
    SFData valueArray, SFUInteger valueCount, SFBoolean includeFirst,
   _SFGlyphAssessment glyphAssessment, void *helperPtr, SFUInteger *contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInteger valueIndex = 0;
    _SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = _SFGlyphZoneInput;

    if (includeFirst) {
        glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
        glyphAgent.recordValue = SFUInt16Array_Value(valueArray, 0);

        if (!glyphAssessment(&glyphAgent)) {
            return SFFalse;
        }

        valueIndex += 1;
    } else {
        valueCount -= 1;
    }

    for (; valueIndex < valueCount; valueIndex++) {
        inputIndex = SFLocatorGetAfter(locator, inputIndex);

        if (inputIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
            glyphAgent.recordValue = SFUInt16Array_Value(valueArray, valueIndex);

            if (!glyphAssessment(&glyphAgent)) {
                return SFFalse;
            }
        } else {
            return SFFalse;
        }
    }

    *contextEnd = inputIndex;
    return SFTrue;
}

static SFBoolean _SFAssessLookaheadGlyphs(SFTextProcessorRef textProcessor,
    SFData valueArray, SFUInteger valueCount,
    _SFGlyphAssessment glyphAssessment, void *helperPtr, SFUInteger contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    SFLocatorRef locator = &textProcessor->_locator;
    SFUInteger lookaheadIndex = contextEnd;
    SFUInteger valueIndex;
    _SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = _SFGlyphZoneLookahead;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        lookaheadIndex = SFLocatorGetAfter(locator, lookaheadIndex);

        if (lookaheadIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, lookaheadIndex);
            glyphAgent.recordValue = SFUInt16Array_Value(valueArray, valueIndex);

            if (!glyphAssessment(&glyphAgent)) {
                return SFFalse;
            }
        } else {
            return SFFalse;
        }
    }

    return SFTrue;
}

SF_PRIVATE SFBoolean _SFApplyContextSubtable(SFTextProcessorRef processor, SFData contextSubtable)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, locator->index);
    SFUInt16 format;

    format = SFContext_Format(contextSubtable);

    switch (format) {
        case 1: {
            SFData coverageTable = SFContextF1_CoverageTable(contextSubtable);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 chainRuleSetCount = SFContextF1_RuleSetCount(contextSubtable);

                if (coverageIndex < chainRuleSetCount) {
                    SFData ruleSetTable = SFContextF1_RuleSetTable(contextSubtable, coverageIndex);

                    return _SFApplyRuleSetTable(processor, ruleSetTable, _SFAssessGlyphByEquality, NULL);
                }
            }
            break;
        }

        case 2: {
            SFData coverageTable = SFContextF2_CoverageTable(contextSubtable);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFData classDefTable = SFContextF2_ClassDefTable(contextSubtable);
                SFUInt16 ruleSetCount = SFContextF2_RuleSetCount(contextSubtable);
                SFUInt16 inputClass;

                inputClass = SFOpenTypeSearchGlyphClass(classDefTable, inputGlyph);

                if (inputClass < ruleSetCount) {
                    SFData ruleSetTable = SFContextF2_RuleSetTable(contextSubtable, inputClass);
                    SFData helperTables[1];

                    helperTables[0] = classDefTable;

                    return _SFApplyRuleSetTable(processor, ruleSetTable, _SFAssessGlyphByClass, helperTables);
                }
            }
            break;
        }

        case 3: {
            SFData ruleTable = SFContextF3_Rule(contextSubtable);
            return _SFApplyRuleTable(processor, ruleTable, SFTrue, _SFAssessGlyphByCoverage, (void *)contextSubtable);
        }
    }
    
    return SFFalse;
}

static SFBoolean _SFApplyRuleSetTable(SFTextProcessorRef textProcessor,
    SFData ruleSetTable, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = SFRuleSet_RuleCount(ruleSetTable);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        SFOffset ruleOffset = SFRuleSet_RuleOffset(ruleSetTable, ruleIndex);

        if (ruleOffset) {
            SFData ruleTable = SFData_Subdata(ruleSetTable, ruleOffset);

            if (_SFApplyRuleTable(textProcessor, ruleTable, SFFalse, glyphAsessment, helperPtr)) {
                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyRuleTable(SFTextProcessorRef textProcessor,
    SFData ruleTable, SFBoolean includeFirst, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 glyphCount = SFRule_GlyphCount(ruleTable);

    /* Make sure that rule table contains at least one glyph. */
    if (glyphCount > 0) {
        SFUInt16 lookupCount = SFRule_LookupCount(ruleTable);
        SFData valueArray = SFRule_ValueArray(ruleTable);
        SFData lookupArray = SFRule_LookupArray(ruleTable, glyphCount - !includeFirst);
        SFUInteger contextStart = textProcessor->_locator.index;
        SFUInteger contextEnd;

        return (_SFAssessInputGlyphs(textProcessor, valueArray, glyphCount, includeFirst, glyphAsessment, helperPtr, &contextEnd)
                && _SFApplyContextLookups(textProcessor, lookupArray, lookupCount, contextStart, contextEnd));
    }

    return SFFalse;
}

SF_PRIVATE SFBoolean _SFApplyChainContextSubtable(SFTextProcessorRef textProcessor,
    SFData chainContextSubtable)
{
    SFGlyphID inputGlyph = SFAlbumGetGlyph(textProcessor->_album, textProcessor->_locator.index);
    SFUInt16 format;
    
    format = SFChainContext_Format(chainContextSubtable);

    switch (format) {
        case 1: {
            SFData coverageTable = SFChainContextF1_CoverageTable(chainContextSubtable);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 chainRuleSetCount = SFChainContextF1_ChainRuleSetCount(chainContextSubtable);

                if (coverageIndex < chainRuleSetCount) {
                    SFData chainRuleSetTable = SFChainContextF1_ChainRuleSetTable(chainContextSubtable, coverageIndex);

                    return _SFApplyChainRuleSetTable(textProcessor, chainRuleSetTable, _SFAssessGlyphByEquality, NULL);
                }
            }
            break;
        }

        case 2: {
            SFData coverageTable = SFChainContextF2_CoverageTable(chainContextSubtable);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFData backtrackClassDefTable = SFChainContextF2_BacktrackClassDefTable(chainContextSubtable);
                SFData inputClassDefTable = SFChainContextF2_InputClassDefTable(chainContextSubtable);
                SFData lookaheadClassDefTable = SFChainContextF2_LookaheadClassDefTable(chainContextSubtable);
                SFUInt16 chainRuleSetCount = SFChainContextF2_ChainRuleSetCount(chainContextSubtable);
                SFUInt16 inputClass;

                inputClass = SFOpenTypeSearchGlyphClass(inputClassDefTable, inputGlyph);

                if (inputClass < chainRuleSetCount) {
                    SFData chainRuleSetTable = SFChainContextF2_ChainRuleSetTable(chainContextSubtable, inputClass);
                    SFData helperTables[3];

                    helperTables[0] = inputClassDefTable;
                    helperTables[1] = backtrackClassDefTable;
                    helperTables[2] = lookaheadClassDefTable;

                    return _SFApplyChainRuleSetTable(textProcessor, chainRuleSetTable, _SFAssessGlyphByClass, helperTables);
                }
            }
            break;
        }

        case 3: {
            SFData chainRuleTable = SFChainContextF3_ChainRuleTable(chainContextSubtable);
            return _SFApplyChainRuleTable(textProcessor, chainRuleTable, SFTrue, _SFAssessGlyphByCoverage, (void *)chainContextSubtable);
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyChainRuleSetTable(SFTextProcessorRef textProcessor,
    SFData chainRuleSetTable, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 chainRuleCount = SFChainRuleSet_ChainRuleCount(chainRuleSetTable);
    SFUInteger chainRuleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (chainRuleIndex = 0; chainRuleIndex < chainRuleCount; chainRuleIndex++) {
        SFData chainRuleTable = SFChainRuleSet_ChainRuleTable(chainRuleSetTable, chainRuleIndex);

        if (_SFApplyChainRuleTable(textProcessor, chainRuleTable, SFFalse, glyphAsessment, helperPtr)) {
            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyChainRuleTable(SFTextProcessorRef textProcessor,
    SFData chainRuleTable, SFBoolean includeFirst, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFData backtrackRecord = SFChainRule_BacktrackRecord(chainRuleTable);
    SFUInt16 backtrackCount = SFBacktrackRecord_GlyphCount(backtrackRecord);
    SFData backtrackArray = SFBacktrackRecord_ValueArray(backtrackRecord);
    SFData inputRecord = SFBacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = SFInputRecord_GlyphCount(inputRecord);

    /* Make sure that input record has at least one glyph. */
    if (inputCount > 0) {
        SFData inputArray = SFInputRecord_ValueArray(inputRecord);
        SFData lookaheadRecord = SFInputRecord_LookaheadRecord(inputRecord, inputCount - !includeFirst);
        SFUInt16 lookaheadCount = SFInputRecord_GlyphCount(lookaheadRecord);
        SFData lookaheadArray = SFLookaheadRecord_ValueArray(lookaheadRecord);
        SFData contextRecord = SFLookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);
        SFUInteger lookupCount = SFContextRecord_LookupCount(contextRecord);
        SFData lookupArray = SFContextRecord_LookupArray(contextRecord);
        SFUInteger contextStart = textProcessor->_locator.index;
        SFUInteger contextEnd;

        return (_SFAssessInputGlyphs(textProcessor, inputArray, inputCount, includeFirst, glyphAsessment, helperPtr, &contextEnd)
                && _SFAssessBacktrackGlyphs(textProcessor, backtrackArray, backtrackCount,glyphAsessment, helperPtr)
                && _SFAssessLookaheadGlyphs(textProcessor, lookaheadArray, lookaheadCount, glyphAsessment, helperPtr, contextEnd)
                && _SFApplyContextLookups(textProcessor, lookupArray, lookupCount, contextStart, contextEnd));
    }

    return SFFalse;
}

static SFBoolean _SFApplyContextLookups(SFTextProcessorRef textProcessor,
    SFData lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextEnd)
{
    SFLocatorRef contextLocator = &textProcessor->_locator;
    SFLocator originalLocator = *contextLocator;
    SFUInteger lookupIndex;

    /* Make the context locator cover only context range. */
    SFLocatorReset(contextLocator, contextStart, (contextEnd - contextStart) + 1);

    /* Apply the lookup records sequentially as they are ordered by preference. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFData lookupRecord = SFLookupArray_Value(lookupArray, lookupIndex);
        SFUInt16 sequenceIndex = SFLookupRecord_SequenceIndex(lookupRecord);
        SFUInt16 lookupListIndex = SFLookupRecord_LookupListIndex(lookupRecord);

        /* Jump the locator to context index. */
        SFLocatorJumpTo(contextLocator, contextStart);
        
        if (SFLocatorMoveNext(contextLocator)) {
            /* Skip the glyphs till sequence index and apply the lookup. */
            if (SFLocatorSkip(contextLocator, sequenceIndex)) {
                _SFApplyLookup(textProcessor, lookupListIndex);
            }
        }
    }

    /* Take the state of context locator so that input glyphs are skipped properly. */
    SFLocatorTakeState(&originalLocator, contextLocator);
    /* Switch back to the original locator. */
    textProcessor->_locator = originalLocator;

    return SFTrue;
}

SF_PRIVATE SFBoolean _SFApplyExtensionSubtable(SFTextProcessorRef textProcessor,
    SFData extensionSubtable)
{
    SFUInt16 format = SFExtension_Format(extensionSubtable);

    switch (format) {
        case 1: {
            SFLookupType lookupType = SFExtensionF1_LookupType(extensionSubtable);
            SFData innerSubtable = SFExtensionF1_ExtensionData(extensionSubtable);

            return textProcessor->_lookupOperation(textProcessor, lookupType, innerSubtable);
        }
    }
    
    return SFFalse;
}
