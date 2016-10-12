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

static SFBoolean _SFApplyChainRule(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData chainRule);
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

                        if (_SFApplyChainRule(processor, featureKind, chainRule)) {
                            return SFTrue;
                        }
                    }
                }
            }
            break;
        }

        case 3:
            return _SFApplyChainContextF3(processor, featureKind, chainContext);
    }

    return SFFalse;
}

static SFBoolean _SFApplyChainRule(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData chainRule)
{
    SFData backtrackRecord = SFChainRule_BacktrackRecord(chainRule);
    SFUInt16 backtrackCount = SFBacktrackRecord_GlyphCount(backtrackRecord);
    SFData inputRecord = SFBacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = SFInputRecord_GlyphCount(inputRecord);

    /* Make sure that input record has at least one input glyph. */
    if (inputCount > 0) {
        SFData lookaheadRecord = SFInputRecord_LookaheadRecord(inputRecord, inputCount - 1);
        SFUInt16 lookaheadCount = SFInputRecord_GlyphCount(lookaheadRecord);
        SFData contextRecord = SFLookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);
        SFAlbumRef album = processor->_album;
        SFLocatorRef locator = &processor->_locator;
        SFUInteger backtrackIndex;
        SFUInteger inputIndex;
        SFUInteger lookaheadIndex;
        SFUInteger recordIndex;

        inputIndex = locator->index;

        /* Match the remaining input glyphs. */
        for (recordIndex = 1; recordIndex < inputCount; recordIndex++) {
            inputIndex = SFLocatorGetAfter(locator, inputIndex);

            if (inputIndex != SFInvalidIndex) {
                SFGlyphID inputGlyph = SFAlbumGetGlyph(album, inputIndex);
                SFGlyphID matchingGlyph = SFInputRecord_Value(inputRecord, recordIndex - 1);

                if (inputGlyph != matchingGlyph) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        backtrackIndex = locator->index;

        /* Match the backtrack glyphs. */
        for (recordIndex = 0; recordIndex < backtrackCount; recordIndex++) {
            backtrackIndex = SFLocatorGetBefore(locator, backtrackIndex);

            if (backtrackIndex != SFInvalidIndex) {
                SFGlyphID inputGlyph = SFAlbumGetGlyph(album, backtrackIndex);
                SFGlyphID matchingGlyph = SFBacktrackRecord_Value(backtrackRecord, recordIndex);

                if (inputGlyph != matchingGlyph) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        lookaheadIndex = inputIndex;

        /* Match the lookahead glyphs. */
        for (recordIndex = 0; recordIndex < lookaheadCount; recordIndex++) {
            lookaheadIndex = SFLocatorGetAfter(locator, lookaheadIndex);

            if (lookaheadIndex != SFInvalidIndex) {
                SFGlyphID inputGlyph = SFAlbumGetGlyph(album, lookaheadIndex);
                SFGlyphID matchingGlyph = SFLookaheadRecord_Value(lookaheadRecord, recordIndex);

                if (inputGlyph != matchingGlyph) {
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

static SFBoolean _SFApplyChainContextF3(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData chainContext)
{
    SFData backtrackRecord = SFChainContextF3_BacktrackRecord(chainContext);
    SFUInt16 backtrackCount = SFBacktrackRecord_GlyphCount(backtrackRecord);
    SFData inputRecord = SFBacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = SFInputRecord_GlyphCount(inputRecord);
    SFData lookaheadRecord = SFInputRecord_LookaheadRecord(inputRecord, inputCount);
    SFUInt16 lookaheadCount = SFInputRecord_GlyphCount(lookaheadRecord);
    SFData contextRecord = SFLookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);

    /* Make sure that input record has at least one input glyph. */
    if (inputCount > 0) {
        SFAlbumRef album = processor->_album;
        SFLocatorRef locator = &processor->_locator;
        SFGlyphID inputGlyph;
        SFOffset offset;
        SFData coverage;
        SFUInteger coverageIndex;

        offset = SFInputRecord_Value(inputRecord, 0);
        coverage = SFData_Subdata(chainContext, offset);

        inputGlyph = SFAlbumGetGlyph(album, locator->index);
        coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

        /* Proceed if first glyph exists in coverage. */
        if (coverageIndex != SFInvalidIndex) {
            SFUInteger backtrackIndex;
            SFUInteger inputIndex;
            SFUInteger lookaheadIndex;
            SFUInteger recordIndex;

            inputIndex = locator->index;

            /* Match the remaining input glyphs. */
            for (recordIndex = 1; recordIndex < inputCount; recordIndex++) {
                inputIndex = SFLocatorGetAfter(locator, inputIndex);

                if (inputIndex != SFInvalidIndex) {
                    offset = SFInputRecord_Value(inputRecord, recordIndex);
                    coverage = SFData_Subdata(chainContext, offset);

                    inputGlyph = SFAlbumGetGlyph(album, inputIndex);
                    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

                    if (coverageIndex == SFInvalidIndex) {
                        goto NotMatched;
                    }
                } else {
                    goto NotMatched;
                }
            }

            backtrackIndex = locator->index;

            /* Match the backtrack glyphs. */
            for (recordIndex = 0; recordIndex < backtrackCount; recordIndex++) {
                backtrackIndex = SFLocatorGetBefore(locator, backtrackIndex);

                if (backtrackIndex != SFInvalidIndex) {
                    offset = SFBacktrackRecord_Value(backtrackRecord, recordIndex);
                    coverage = SFData_Subdata(chainContext, offset);

                    inputGlyph = SFAlbumGetGlyph(album, backtrackIndex);
                    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

                    if (coverageIndex == SFInvalidIndex) {
                        goto NotMatched;
                    }
                } else {
                    goto NotMatched;
                }
            }

            lookaheadIndex = inputIndex;

            /* Match the lookahead glyphs. */
            for (recordIndex = 0; recordIndex < lookaheadCount; recordIndex++) {
                lookaheadIndex = SFLocatorGetAfter(locator, lookaheadIndex);

                if (lookaheadIndex != SFInvalidIndex) {
                    offset = SFLookaheadRecord_Value(lookaheadRecord, recordIndex);
                    coverage = SFData_Subdata(chainContext, offset);

                    inputGlyph = SFAlbumGetGlyph(album, lookaheadIndex);
                    coverageIndex = SFOpenTypeSearchCoverageIndex(coverage, inputGlyph);

                    if (coverageIndex == SFInvalidIndex) {
                        goto NotMatched;
                    }
                } else {
                    goto NotMatched;
                }
            }

            _SFApplyContextRecord(processor, featureKind, contextRecord, locator->index, (inputIndex - locator->index) + 1);
            return SFTrue;
        }
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

    /* Apply the lookup records sequentially as they are ordered by */
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
