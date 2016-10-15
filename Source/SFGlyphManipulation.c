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

static SFBoolean _SFApplyRuleSetTable(SFTextProcessorRef processor, SFFeatureKind featureKind,
                                      SFData ruleSetTable, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean _SFApplyRuleTable(SFTextProcessorRef processor, SFFeatureKind featureKind,
                                   SFData ruleTable, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean _SFApplyChainRule(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData chainRule, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean _SFApplyContextLookups(SFTextProcessorRef processor, SFFeatureKind featureKind,
                                   SFData lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextLength);

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
    SFData chainContextTable = glyphAgent->helperPtr;
    SFData coverageTable;
    SFUInteger coverageIndex;

    coverageTable = SFData_Subdata(chainContextTable, glyphAgent->recordValue);
    coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, glyphAgent->glyphID);

    return (coverageIndex != SFInvalidIndex);
}

SF_PRIVATE SFBoolean _SFApplyContextSubtable(SFTextProcessorRef processor, SFFeatureKind featureKind, SFData contextSubtable)
{
    SFAlbumRef album = processor->_album;
    SFLocatorRef locator = &processor->_locator;
    SFGlyphID inputGlyph = SFAlbumGetGlyph(album, locator->index);
    SFUInt16 format;

    format = SFContext_Format(contextSubtable);

    switch (format) {
        case 1: {
            SFOffset coverageOffset = SFContextF1_CoverageOffset(contextSubtable);
            SFData coverageTable = SFData_Subdata(contextSubtable, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 chainRuleSetCount = SFContextF1_RuleSetCount(contextSubtable);

                if (coverageIndex < chainRuleSetCount) {
                    SFOffset ruleSetOffset = SFContextF1_RuleSetOffset(contextSubtable, coverageIndex);
                    SFData ruleSetTable = SFData_Subdata(contextSubtable, ruleSetOffset);
                    SFUInt16 ruleCount;
                    SFUInteger ruleIndex;

                    return _SFApplyRuleSetTable(processor, featureKind, ruleSetTable, SFTrue, _SFAssessGlyphByEquality, NULL);
                }
            }
            break;
        }

        case 2: {
            SFOffset coverageOffset = SFContextF2_CoverageOffset(contextSubtable);
            SFData coverageTable = SFData_Subdata(contextSubtable, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFOffset classDefOffset = SFContextF2_ClassDefOffset(contextSubtable);
                SFUInt16 ruleSetCount = SFContextF2_RuleSetCount(contextSubtable);

                if (coverageIndex < ruleSetCount) {
                    SFOffset ruleSetOffset = SFChainContextF2_ChainRuleSetOffset(contextSubtable, coverageIndex);

                    /* The offset might be NULL if no contexts begin with this class. */
                    if (ruleSetOffset) {
                        SFData ruleSetTable = SFData_Subdata(contextSubtable, ruleSetOffset);
                        SFData classDefTables[] = {
                            SFData_Subdata(contextSubtable, classDefOffset)
                        };

                        return _SFApplyRuleSetTable(processor, featureKind, ruleSetTable, SFTrue, _SFAssessGlyphByClass, classDefTables);
                    }
                }
            }
            break;
        }

        case 3: {
            SFData ruleTable = SFContextF3_Rule(contextSubtable);
            return _SFApplyChainRule(processor, featureKind, ruleTable, SFFalse, _SFAssessGlyphByCoverage, contextSubtable);
        }
    }
    
    return SFFalse;
}

static SFBoolean _SFApplyRuleSetTable(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData ruleSetTable, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = SFRuleSet_RuleCount(ruleSetTable);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        SFOffset ruleOffset = SFRuleSet_RuleOffset(ruleSetTable, ruleIndex);

        if (ruleOffset) {
            SFData ruleTable = SFData_Subdata(ruleSetTable, ruleOffset);

            if (_SFApplyRuleTable(processor, featureKind, ruleTable, firstGlyphAssessed, _SFAssessGlyphByClass, helperPtr)) {
                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean _SFApplyRuleTable(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData ruleTable, SFBoolean firstGlyphAssessed, _SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 glyphCount = SFRule_GlyphCount(ruleTable);

    /* Make sure that rule table contains at least one glyph. */
    if (glyphCount > 0) {
        SFUInt16 lookupCount = SFRule_LookupCount(ruleTable);
        SFData lookupArray = SFRule_LookupArray(ruleTable, glyphCount - firstGlyphAssessed);
        SFAlbumRef album;
        SFLocatorRef locator;
        _SFGlyphAgent glyphAgent;
        SFUInteger inputIndex;
        SFUInteger recordIndex;

        album = processor->_album;
        locator = &processor->_locator;
        glyphAgent.helperPtr = helperPtr;
        glyphAgent.glyphZone = _SFGlyphZoneInput;

        inputIndex = locator->index;

        /* Match the remaining input glyphs. */
        for (recordIndex = 1; recordIndex < glyphCount; recordIndex++) {
            inputIndex = SFLocatorGetAfter(locator, inputIndex);

            if (inputIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
                glyphAgent.recordValue = SFRule_Value(ruleTable, recordIndex - firstGlyphAssessed);

                if (!glyphAsessment(&glyphAgent)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        return _SFApplyContextLookups(processor, featureKind, lookupArray, lookupCount, locator->index, (inputIndex - locator->index) + 1);
    }
    
NotMatched:
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
            SFOffset coverageOffset = SFChainContextF1_CoverageOffset(chainContext);
            SFData coverageTable = SFData_Subdata(chainContext, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFUInt16 chainRuleSetCount = SFChainContextF1_ChainRuleSetCount(chainContext);

                if (coverageIndex < chainRuleSetCount) {
                    SFOffset chainRuleSetOffset = SFChainContextF1_ChainRuleSetOffset(chainContext, coverageIndex);
                    SFData chainRuleSetTable = SFData_Subdata(chainContext, chainRuleSetOffset);
                    SFUInt16 chainRuleCount;
                    SFUInteger ruleIndex;

                    chainRuleCount = SFChainRuleSet_ChainRuleCount(chainRuleSetTable);

                    /* Match each rule sequentially as they are ordered by preference. */
                    for (ruleIndex = 0; ruleIndex < chainRuleCount; ruleIndex++) {
                        SFOffset chainRuleOffset = SFChainRuleSet_ChainRuleOffset(chainRuleSetTable, ruleIndex);
                        SFData chainRuleTable = SFData_Subdata(chainRuleSetTable, chainRuleOffset);

                        if (_SFApplyChainRule(processor, featureKind, chainRuleTable, SFTrue, _SFAssessGlyphByEquality, NULL)) {
                            return SFTrue;
                        }
                    }
                }
            }
            break;
        }

        case 2: {
            SFOffset coverageOffset = SFChainContextF2_CoverageOffset(chainContext);
            SFData coverageTable = SFData_Subdata(chainContext, coverageOffset);
            SFUInteger coverageIndex;

            coverageIndex = SFOpenTypeSearchCoverageIndex(coverageTable, inputGlyph);

            if (coverageIndex != SFInvalidIndex) {
                SFOffset backtrackClassDefOffset = SFChainContextF2_BacktrackClassDefOffset(chainContext);
                SFOffset inputClassDefOffset = SFChainContextF2_InputClassDefOffset(chainContext);
                SFOffset lookaheadClassDefOffset = SFChainContextF2_LookaheadClassDefOffset(chainContext);
                SFUInt16 chainRuleSetCount = SFChainContextF2_ChainRuleSetCount(chainContext);

                if (coverageIndex < chainRuleSetCount) {
                    SFOffset chainRuleSetOffset = SFChainContextF2_ChainRuleSetOffset(chainContext, coverageIndex);
                    SFData chainRuleSetTable = SFData_Subdata(chainContext, chainRuleSetOffset);
                    SFData classDefTables[3];
                    SFUInt16 chainRuleCount;
                    SFUInteger ruleIndex;

                    classDefTables[0] = SFData_Subdata(chainContext, inputClassDefOffset);
                    classDefTables[1] = SFData_Subdata(chainContext, backtrackClassDefOffset);
                    classDefTables[2] = SFData_Subdata(chainContext, lookaheadClassDefOffset);

                    chainRuleCount = SFChainRuleSet_ChainRuleCount(chainRuleSetTable);

                    /* Match each rule sequentially as they are ordered by preference. */
                    for (ruleIndex = 0; ruleIndex < chainRuleCount; ruleIndex++) {
                        SFOffset chainRuleOffset = SFChainRuleSet_ChainRuleOffset(chainRuleSetTable, ruleIndex);
                        SFData chainRuleTable = SFData_Subdata(chainRuleSetTable, chainRuleOffset);

                        if (_SFApplyChainRule(processor, featureKind, chainRuleTable, SFTrue, _SFAssessGlyphByClass, classDefTables)) {
                            return SFTrue;
                        }
                    }
                }
            }
            break;
        }

        case 3: {
            SFData chainRuleTable = SFChainContextF3_ChainRule(chainContext);
            return _SFApplyChainRule(processor, featureKind, chainRuleTable, SFFalse, _SFAssessGlyphByCoverage, chainContext);
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
        SFUInteger lookupCount = SFContextRecord_LookupCount(contextRecord);
        SFData lookupArray = SFContextRecord_LookupRecord(contextRecord, 0);
        SFAlbumRef album;
        SFLocatorRef locator;
        _SFGlyphAgent glyphAgent;
        SFUInteger inputIndex;
        SFUInteger backtrackIndex;
        SFUInteger lookaheadIndex;
        SFUInteger recordIndex;

        album = processor->_album;
        locator = &processor->_locator;
        glyphAgent.helperPtr = helperPtr;
        glyphAgent.glyphZone = _SFGlyphZoneInput;

        inputIndex = locator->index;

        /* Match the remaining input glyphs. */
        for (recordIndex = 1; recordIndex < inputCount; recordIndex++) {
            inputIndex = SFLocatorGetAfter(locator, inputIndex);

            if (inputIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
                glyphAgent.recordValue = SFInputRecord_Value(inputRecord, recordIndex - firstGlyphAssessed);

                if (!glyphAsessment(&glyphAgent)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        glyphAgent.glyphZone = _SFGlyphZoneBacktrack;
        backtrackIndex = locator->index;

        /* Match the backtrack glyphs. */
        for (recordIndex = 0; recordIndex < backtrackCount; recordIndex++) {
            backtrackIndex = SFLocatorGetBefore(locator, backtrackIndex);

            if (backtrackIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, backtrackIndex);
                glyphAgent.recordValue = SFBacktrackRecord_Value(backtrackRecord, recordIndex);

                if (!glyphAsessment(&glyphAgent)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        glyphAgent.glyphZone = _SFGlyphZoneLookahead;
        lookaheadIndex = inputIndex;

        /* Match the lookahead glyphs. */
        for (recordIndex = 0; recordIndex < lookaheadCount; recordIndex++) {
            lookaheadIndex = SFLocatorGetAfter(locator, lookaheadIndex);

            if (lookaheadIndex != SFInvalidIndex) {
                glyphAgent.glyphID = SFAlbumGetGlyph(album, lookaheadIndex);
                glyphAgent.recordValue = SFLookaheadRecord_Value(lookaheadRecord, recordIndex);

                if (!glyphAsessment(&glyphAgent)) {
                    goto NotMatched;
                }
            } else {
                goto NotMatched;
            }
        }

        return _SFApplyContextLookups(processor, featureKind, lookupArray, lookupCount, locator->index, (inputIndex - locator->index) + 1);
    }

NotMatched:
    return SFFalse;
}

static SFBoolean _SFApplyContextLookups(SFTextProcessorRef processor, SFFeatureKind featureKind,
    SFData lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextLength)
{
    SFLocatorRef contextLocator = &processor->_locator;
    SFLocator originalLocator = *contextLocator;
    SFUInteger lookupIndex;

    /* Make the context locator cover only context range. */
    SFLocatorReset(contextLocator, contextStart, contextLength);

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
                _SFApplyLookup(processor, featureKind, lookupListIndex);
            }
        }
    }

    /* Take the state of context locator so that input glyphs are skipped properly. */
    SFLocatorTakeState(&originalLocator, contextLocator);
    /* Switch back to the original locator. */
    processor->_locator = originalLocator;

    return SFTrue;
}
