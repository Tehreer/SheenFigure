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
#include "SFShapingEngine.h"

enum {
    _SFGlyphZoneBacktrack = 0,
    _SFGlyphZoneInput = 1,
    _SFGlyphZoneLookahead = 2
};
typedef SFUInt8 _SFGlyphZone;

typedef struct {
    _SFGlyphZone glyphZone;
    SFGlyphID glyphID;
    SFUInt16 recordValue;
} _SFGlyphAgent;

typedef SFBoolean (*_SFGlyphAssessment)(_SFGlyphAgent *glyphAgent, void *helperPtr);

static SFBoolean _SFApplyChainRule(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData chainRule, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean _SFApplyChainContextF3(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData chainContext);
static void _SFApplyContextRecord(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData contextRecord, SFUInteger startIndex, SFUInteger endIndex);

SF_PRIVATE SFBoolean _SFApplyExtensionSubtable(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData extension)
{
    SFUInt16 format = SFExtension_Format(extension);

    switch (format) {
        case 1: {
            SFLookupType lookupType = SFExtensionF1_LookupType(extension);
            SFUInt32 offset = SFExtensionF1_ExtensionOffset(extension);
            SFData innerSubtable = SFData_Subdata(extension, offset);

            switch (featureKind) {
                case SFFeatureKindSubstitution:
                    return _SFApplySubstitutionSubtable(processor, lookupType, innerSubtable);

                case SFFeatureKindPositioning:
                    return _SFApplyPositioningSubtable(processor, lookupType, innerSubtable);
            }
            break;
        }
    }

    return SFFalse;
}

static SFBoolean _SFAssessGlyphsByEquality(_SFGlyphAgent *glyphAgent, void *helperPtr)
{
    return (glyphAgent->glyphID == glyphAgent->recordValue);
}

static SFBoolean _SFAssessGlyphsByCoverage(_SFGlyphAgent *glyphAgent, void *helperPtr)
{
    SFData chainContext = helperPtr;
    SFData coverage = SFData_Subdata(chainContext, glyphAgent->recordValue);
    SFUInteger coverageIndex;

    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, glyphAgent->glyphID);

    return (coverageIndex != SFInvalidIndex);
}

SF_PRIVATE SFBoolean _SFApplyChainContextSubtable(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData chainContext)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, locator->index);
    SFUInt16 format;
    
    format = SFChainContext_Format(chainContext);

    switch (format) {
        case 1: {
            SFOffset offset = SFChainContextF1_CoverageOffset(chainContext);
            SFData coverage = SFData_Subdata(chainContext, offset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 chainRuleSetCount = SFChainContextF1_ChainRuleSetCount(chainContext);
                SFData chainRuleSet;

                if (coverageIndex < chainRuleSetCount) {
                    SFUInt16 chainRuleCount;
                    SFData chainRule;
                    SFUInteger ruleIndex;

                    offset = SFChainContextF1_ChainRuleSetOffset(chainContext, coverageIndex);
                    chainRuleSet = SFData_Subdata(chainContext, offset);
                    chainRuleCount = SFChainRuleSet_ChainRuleCount(chainRuleSet);

                    /* Match each rule sequentially as they are ordered by preference. */
                    for (ruleIndex = 0; ruleIndex < chainRuleCount; ruleIndex++) {
                        offset = SFChainRuleSet_ChainRuleOffset(chainRuleSet, ruleIndex);
                        chainRule = SFData_Subdata(chainRuleSet, offset);

                        if (_SFApplyChainRule(processor, featureKind, chainRule, SFTrue, _SFAssessGlyphsByEquality, NULL)) {
                            return SFTrue;
                        }
                    }
                }
            }
            break;
        }

        case 3: {
            SFData chainRule = SFChainContextF3_ChainRule(chainContext);
            return _SFApplyChainRule(processor, featureKind, chainRule, SFFalse, _SFAssessGlyphsByCoverage, chainContext);
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyChainRule(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData chainRule, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFData backtrackRecord = SFChainRule_BacktrackRecord(chainRule);
    SFUInt16 backtrackCount = SFBacktrackRecord_GlyphCount(backtrackRecord);
    SFData inputRecord = SFBacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = SFInputRecord_GlyphCount(inputRecord);

    /* Make sure that input record has at least one glyph. */
    if (inputCount > 0) {
        SFData lookaheadRecord = SFInputRecord_LookaheadRecord(inputRecord, inputCount - firstGlyphAssessed);
        SFUInt16 lookaheadCount = SFInputRecord_GlyphCount(lookaheadRecord);
        SFData contextRecord = SFLookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);
        SFAlbumRef album;
        SFLocatorRef locator;
        _SFGlyphAgent glyphAgent;
        SFUInteger inputIndex;
        SFUInteger backtrackIndex;
        SFUInteger lookaheadIndex;
        SFUInteger recordIndex;

        album = processor->_album;
        locator = &processor->_locator;
        inputIndex = locator->index;
        glyphAgent.glyphZone = _SFGlyphZoneInput;

        /* Match the remaining input glyphs. */
        for (recordIndex = 1; recordIndex < inputCount; recordIndex++) {
            inputIndex = SFLocatorGetAfter(locator, inputIndex);

            if (inputIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
                glyphAgent.recordValue = SFInputRecord_Value(inputRecord, recordIndex - firstGlyphAssessed);

                if (!glyphAsessment(&glyphAgent, helperPtr)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        backtrackIndex = locator->index;
        glyphAgent.glyphZone = _SFGlyphZoneBacktrack;

        /* Match the backtrack glyphs. */
        for (recordIndex = 0; recordIndex < backtrackCount; recordIndex++) {
            backtrackIndex = SFLocatorGetBefore(locator, backtrackIndex);

            if (backtrackIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, backtrackIndex);
                glyphAgent.recordValue = SFBacktrackRecord_Value(backtrackRecord, recordIndex);

                if (!glyphAsessment(&glyphAgent, helperPtr)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        lookaheadIndex = inputIndex;
        glyphAgent.glyphZone = _SFGlyphZoneLookahead;

        /* Match the lookahead glyphs. */
        for (recordIndex = 0; recordIndex < lookaheadCount; recordIndex++) {
            lookaheadIndex = SFLocatorGetAfter(locator, lookaheadIndex);

            if (lookaheadIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, lookaheadIndex);
                glyphAgent.recordValue = SFLookaheadRecord_Value(lookaheadRecord, recordIndex);

                if (!glyphAsessment(&glyphAgent, helperPtr)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        _SFApplyContextRecord(processor, featureKind, contextRecord, locator->index, (inputIndex - locator->index) + 1);
        return SFTrue;
    }

NotMatched:
    return SFFalse;
}

static void _SFApplyContextRecord(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData contextRecord, SFUInteger index, SFUInteger count) {
    SFLocatorRef contextLocator = &processor->_locator;
    SFLocator originalLocator = *contextLocator;
    SFUInt16 lookupCount;
    SFUInteger lookupIndex;

    lookupCount = SFContextRecord_LookupCount(contextRecord);

    /* Make the context locator cover only context range. */
    SFLocatorReset(contextLocator, index, count);

    /* Apply the lookup records sequentially as they are ordered by preference. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        SFData lookupRecord = SFContextRecord_LookupRecord(contextRecord, lookupIndex);
        SFUInt16 sequenceIndex = SFLookupRecord_SequenceIndex(lookupRecord);
        SFUInt16 lookupListIndex = SFLookupRecord_LookupListIndex(lookupRecord);

        /* Jump the locator to context index. */
        SFLocatorJumpTo(contextLocator, index);
        
        if (SFLocatorMoveNext(contextLocator)) {
            /* Skip the glyphs till sequence index and apply the lookup. */
            if (SFLocatorSkip(contextLocator, sequenceIndex)) {
                _SFApplyLookup(processor, featureKind, lookupListIndex);
            }
        }
    }

    /* Take the state of context locator so that input glyphs are skipped properly. */
    SFLocatorTakeState(&originalLocator, contextLocator);
    /* Switch back to the original locator. */
    processor->_locator = originalLocator;
}
