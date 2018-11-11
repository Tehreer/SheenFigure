/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#include "SFAlbum.h"
#include "SFBase.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGDEF.h"
#include "SFGSUB.h"
#include "SFLocator.h"
#include "SFOpenType.h"

#include "SFGlyphDiscovery.h"
#include "SFGlyphManipulation.h"
#include "SFGlyphPositioning.h"
#include "SFGlyphSubstitution.h"
#include "SFTextProcessor.h"

enum {
    SFGlyphZoneInput = 0,
    SFGlyphZoneBacktrack = 1,
    SFGlyphZoneLookahead = 2
};
typedef SFUInt8 SFGlyphZone;

typedef struct {
    void *helperPtr;
    SFUInt16 recordValue;
    SFGlyphID glyphID;
    SFGlyphZone glyphZone;
} SFGlyphAgent;

typedef SFBoolean (*SFGlyphAssessment)(SFGlyphAgent *glyphAgent);

static SFBoolean ApplyRuleSetTable(TextProcessorRef textProcessor,
    Data ruleSet, SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean ApplyRuleTable(TextProcessorRef textProcessor,
    Data rule, SFBoolean includeFirst, SFGlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean ApplyChainRuleSetTable(TextProcessorRef textProcessor,
    Data chainRuleSet, SFGlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean ApplyChainRuleTable(TextProcessorRef textProcessor,
    Data chainRule, SFBoolean includeFirst, SFGlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean ApplyContextLookups(TextProcessorRef textProcessor,
    Data lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextEnd);

static SFBoolean AssessGlyphByEquality(SFGlyphAgent *glyphAgent)
{
    return (glyphAgent->glyphID == glyphAgent->recordValue);
}

static SFBoolean AssessGlyphByClass(SFGlyphAgent *glyphAgent)
{
    Data classDef = NULL;
    SFUInt16 glyphClass;

    switch (glyphAgent->glyphZone) {
        case SFGlyphZoneInput:
            classDef = ((Data *)glyphAgent->helperPtr)[0];
            break;

        case SFGlyphZoneBacktrack:
            classDef = ((Data *)glyphAgent->helperPtr)[1];
            break;

        case SFGlyphZoneLookahead:
            classDef = ((Data *)glyphAgent->helperPtr)[2];
            break;
    }

    glyphClass = SearchGlyphClass(classDef, glyphAgent->glyphID);

    return (glyphClass == glyphAgent->recordValue);
}

static SFBoolean AssessGlyphByCoverage(SFGlyphAgent *glyphAgent)
{
    Data coverage = Data_Subdata((Data)glyphAgent->helperPtr, glyphAgent->recordValue);
    SFUInteger covIndex;

    covIndex = SearchCoverageIndex(coverage, glyphAgent->glyphID);

    return (covIndex != SFInvalidIndex);
}

static SFBoolean AssessBacktrackGlyphs(TextProcessorRef textProcessor,
    Data valueArray, SFUInteger valueCount, SFGlyphAssessment glyphAssessment, void *helperPtr)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger backIndex = locator->index;
    SFUInteger valueIndex;
    SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = SFGlyphZoneBacktrack;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        backIndex = LocatorGetBefore(locator, backIndex, SFFalse);

        if (backIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, backIndex);
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

static SFBoolean AssessInputGlyphs(TextProcessorRef textProcessor,
    Data valueArray, SFUInteger valueCount, SFBoolean includeFirst,
    SFGlyphAssessment glyphAssessment, void *helperPtr, SFUInteger *contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInteger valueIndex = 0;
    SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = SFGlyphZoneInput;

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
        inputIndex = LocatorGetAfter(locator, inputIndex, SFTrue);

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

static SFBoolean AssessLookaheadGlyphs(TextProcessorRef textProcessor,
    Data valueArray, SFUInteger valueCount,
    SFGlyphAssessment glyphAssessment, void *helperPtr, SFUInteger contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger aheadIndex = contextEnd;
    SFUInteger valueIndex;
    SFGlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = SFGlyphZoneLookahead;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        aheadIndex = LocatorGetAfter(locator, aheadIndex, SFFalse);

        if (aheadIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, aheadIndex);
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

SF_PRIVATE SFBoolean ApplyContextSubtable(TextProcessorRef textProcessor, Data context)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 tblFormat;

    tblFormat = SFContext_Format(context);

    switch (tblFormat) {
        case 1: {
            Data coverage = SFContextF1_CoverageTable(context);
            SFUInt16 ruleSetCount = SFContextF1_RuleSetCount(context);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex < ruleSetCount) {
                Data ruleSet = SFContextF1_RuleSetTable(context, covIndex);
                return ApplyRuleSetTable(textProcessor, ruleSet, AssessGlyphByEquality, NULL);
            }
            break;
        }

        case 2: {
            Data coverage = SFContextF2_CoverageTable(context);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data classDef = SFContextF2_ClassDefTable(context);
                SFUInt16 ruleSetCount = SFContextF2_RuleSetCount(context);
                SFUInt16 locClass;

                locClass = SearchGlyphClass(classDef, locGlyph);

                if (locClass < ruleSetCount) {
                    Data ruleSet = SFContextF2_RuleSetTable(context, locClass);
                    return ApplyRuleSetTable(textProcessor, ruleSet, AssessGlyphByClass, &classDef);
                }
            }
            break;
        }

        case 3: {
            Data rule = SFContextF3_Rule(context);
            return ApplyRuleTable(textProcessor, rule, SFTrue, AssessGlyphByCoverage,
                                  (void *) context);
        }
    }

    return SFFalse;
}

static SFBoolean ApplyRuleSetTable(TextProcessorRef textProcessor,
    Data ruleSet, SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = SFRuleSet_RuleCount(ruleSet);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        SFOffset ruleOffset = SFRuleSet_RuleOffset(ruleSet, ruleIndex);

        if (ruleOffset) {
            Data rule = Data_Subdata(ruleSet, ruleOffset);

            if (ApplyRuleTable(textProcessor, rule, SFFalse, glyphAsessment, helperPtr)) {
                return SFTrue;
            }
        }
    }

    return SFFalse;
}

static SFBoolean ApplyRuleTable(TextProcessorRef textProcessor,
    Data rule, SFBoolean includeFirst, SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 glyphCount = SFRule_GlyphCount(rule);

    /* Make sure that rule table contains at least one glyph. */
    if (glyphCount > 0) {
        SFUInt16 lookupCount = SFRule_LookupCount(rule);
        Data valueArray = SFRule_ValueArray(rule);
        Data lookupArray = SFRule_LookupArray(rule, glyphCount - !includeFirst);
        SFUInteger contextStart = textProcessor->_locator.index;
        SFUInteger contextEnd;

        return (AssessInputGlyphs(textProcessor, valueArray, glyphCount, includeFirst, glyphAsessment, helperPtr, &contextEnd)
             && ApplyContextLookups(textProcessor, lookupArray, lookupCount, contextStart, contextEnd));
    }

    return SFFalse;
}

SF_PRIVATE SFBoolean ApplyChainContextSubtable(TextProcessorRef textProcessor, Data chainContext)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 tblFormat;

    tblFormat = SFChainContext_Format(chainContext);

    switch (tblFormat) {
        case 1: {
            Data coverage = SFChainContextF1_CoverageTable(chainContext);
            SFUInt16 ruleSetCount = SFChainContextF1_ChainRuleSetCount(chainContext);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex < ruleSetCount) {
                Data chainRuleSet = SFChainContextF1_ChainRuleSetTable(chainContext, covIndex);
                return ApplyChainRuleSetTable(textProcessor, chainRuleSet, AssessGlyphByEquality, NULL);
            }
            break;
        }

        case 2: {
            Data coverage = SFChainContextF2_CoverageTable(chainContext);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data backtrackClassDef = SFChainContextF2_BacktrackClassDefTable(chainContext);
                Data inputClassDef = SFChainContextF2_InputClassDefTable(chainContext);
                Data lookaheadClassDef = SFChainContextF2_LookaheadClassDefTable(chainContext);
                SFUInt16 chainRuleSetCount = SFChainContextF2_ChainRuleSetCount(chainContext);
                SFUInt16 inputClass;

                inputClass = SearchGlyphClass(inputClassDef, locGlyph);

                if (inputClass < chainRuleSetCount) {
                    Data chainRuleSet = SFChainContextF2_ChainRuleSetTable(chainContext, inputClass);
                    Data helpers[3];

                    helpers[0] = inputClassDef;
                    helpers[1] = backtrackClassDef;
                    helpers[2] = lookaheadClassDef;

                    return ApplyChainRuleSetTable(textProcessor, chainRuleSet, AssessGlyphByClass,
                                                  helpers);
                }
            }
            break;
        }

        case 3: {
            Data chainRule = SFChainContextF3_ChainRuleTable(chainContext);
            return ApplyChainRuleTable(textProcessor, chainRule, SFTrue, AssessGlyphByCoverage,
                                       (void *) chainContext);
        }
    }

    return SFFalse;
}

static SFBoolean ApplyChainRuleSetTable(TextProcessorRef textProcessor,
    Data chainRuleSet, SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = SFChainRuleSet_ChainRuleCount(chainRuleSet);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        Data chainRule = SFChainRuleSet_ChainRuleTable(chainRuleSet, ruleIndex);

        if (ApplyChainRuleTable(textProcessor, chainRule, SFFalse, glyphAsessment, helperPtr)) {
            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean ApplyChainRuleTable(TextProcessorRef textProcessor,
    Data chainRule, SFBoolean includeFirst, SFGlyphAssessment glyphAsessment, void *helperPtr)
{
    Data backtrackRecord = SFChainRule_BacktrackRecord(chainRule);
    SFUInt16 backtrackCount = SFBacktrackRecord_GlyphCount(backtrackRecord);
    Data backtrackArray = SFBacktrackRecord_ValueArray(backtrackRecord);
    Data inputRecord = SFBacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = SFInputRecord_GlyphCount(inputRecord);

    /* Make sure that input record has at least one glyph. */
    if (inputCount > 0) {
        Data inputArray = SFInputRecord_ValueArray(inputRecord);
        Data lookaheadRecord = SFInputRecord_LookaheadRecord(inputRecord, inputCount - !includeFirst);
        SFUInt16 lookaheadCount = SFLookaheadRecord_GlyphCount(lookaheadRecord);
        Data lookaheadArray = SFLookaheadRecord_ValueArray(lookaheadRecord);
        Data contextRecord = SFLookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);
        SFUInteger lookupCount = SFContextRecord_LookupCount(contextRecord);
        Data lookupArray = SFContextRecord_LookupArray(contextRecord);
        SFUInteger contextStart = textProcessor->_locator.index;
        SFUInteger contextEnd;

        return (AssessInputGlyphs(textProcessor, inputArray, inputCount, includeFirst, glyphAsessment, helperPtr, &contextEnd)
             && AssessBacktrackGlyphs(textProcessor, backtrackArray, backtrackCount, glyphAsessment, helperPtr)
             && AssessLookaheadGlyphs(textProcessor, lookaheadArray, lookaheadCount, glyphAsessment, helperPtr, contextEnd)
             && ApplyContextLookups(textProcessor, lookupArray, lookupCount, contextStart, contextEnd));
    }

    return SFFalse;
}

static SFBoolean ApplyContextLookups(TextProcessorRef textProcessor,
    Data lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextEnd)
{
    LocatorRef contextLocator = &textProcessor->_locator;
    Locator originalLocator = *contextLocator;
    SFUInteger lookupIndex;

    /* Make the context locator cover only context range. */
    LocatorReset(contextLocator, contextStart, (contextEnd - contextStart) + 1);

    /* Apply the lookup records sequentially as they are ordered by preference. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        Data lookupRecord = SFLookupArray_Value(lookupArray, lookupIndex);
        SFUInt16 sequenceIndex = SFLookupRecord_SequenceIndex(lookupRecord);
        SFUInt16 lookupListIndex = SFLookupRecord_LookupListIndex(lookupRecord);

        /* Jump the locator to context index. */
        LocatorJumpTo(contextLocator, contextStart);

        if (LocatorMoveNext(contextLocator)) {
            /* Skip the glyphs till sequence index and apply the lookup. */
            if (LocatorSkip(contextLocator, sequenceIndex)) {
                ApplyLookup(textProcessor, lookupListIndex);
            }
        }
    }

    /* Take the state of context locator so that input glyphs are skipped properly. */
    LocatorTakeState(&originalLocator, contextLocator);
    /* Switch back to the original locator. */
    textProcessor->_locator = originalLocator;

    return SFTrue;
}

SF_PRIVATE SFBoolean ApplyExtensionSubtable(TextProcessorRef textProcessor, Data extension)
{
    SFUInt16 tblFormat = SFExtension_Format(extension);

    switch (tblFormat) {
        case 1: {
            SFLookupType lookupType = SFExtensionF1_LookupType(extension);
            Data innerSubtable = SFExtensionF1_ExtensionData(extension);

            return textProcessor->_lookupOperation(textProcessor, lookupType, innerSubtable);
        }
    }

    return SFFalse;
}

SF_PRIVATE SFBoolean ApplyReverseChainSubst(TextProcessorRef textProcessor, Data reverseChain)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInt16 substFormat;

    substFormat = SFReverseChainSubst_Format(reverseChain);

    switch (substFormat) {
        case 1: {
            Data coverage = SFReverseChainSubstF1_CoverageTable(reverseChain);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data backtrackRecord = SFReverseChainSubstF1_RevBacktrackRecord(reverseChain);
                SFUInt16 backtrackCount = SFRevBacktrackRecord_GlyphCount(backtrackRecord);
                Data backtrackOffsets = SFRevBacktrackRecord_CoverageOffsets(backtrackRecord);
                Data lookaheadRecord = SFRevBacktrackRecord_RevLookaheadRecord(backtrackRecord, backtrackCount);
                SFUInt16 lookaheadCount = SFRevLookaheadRecord_GlyphCount(lookaheadRecord);
                Data lookaheadOffsets = SFRevLookaheadRecord_CoverageOffsets(lookaheadRecord);
                Data substRecord = SFRevLookaheadRecord_RevSubstRecord(lookaheadRecord, lookaheadCount);
                SFUInt16 substCount = SFRevSubstRecord_GlyphCount(substRecord);

                if (AssessBacktrackGlyphs(textProcessor, backtrackOffsets, backtrackCount, AssessGlyphByCoverage, (void *)reverseChain)
                    && AssessLookaheadGlyphs(textProcessor, lookaheadOffsets, lookaheadCount, AssessGlyphByCoverage, (void *)reverseChain, locator->index)
                    && covIndex < substCount) {
                    SFGlyphID subGlyph = SFRevSubstRecord_Substitute(substRecord, covIndex);
                    GlyphTraits subTraits;

                    subTraits = GetGlyphTraits(textProcessor, subGlyph);

                    /* Substitute the glyph and set its traits. */
                    SFAlbumSetGlyph(album, locator->index, subGlyph);
                    SFAlbumReplaceBasicTraits(album, locator->index, subTraits);

                    return SFTrue;
                }
            }
            break;
        }
    }

    return SFFalse;
}
