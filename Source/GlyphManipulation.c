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
#include "Common.h"
#include "Data.h"
#include "GDEF.h"
#include "GSUB.h"
#include "Locator.h"
#include "OpenType.h"

#include "GlyphDiscovery.h"
#include "GlyphManipulation.h"
#include "GlyphPositioning.h"
#include "GlyphSubstitution.h"
#include "TextProcessor.h"

enum {
    GlyphZoneInput = 0,
    GlyphZoneBacktrack = 1,
    GlyphZoneLookahead = 2
};
typedef SFUInt8 GlyphZone;

typedef struct {
    void *helperPtr;
    SFUInt16 recordValue;
    SFGlyphID glyphID;
    GlyphZone glyphZone;
} GlyphAgent;

typedef SFBoolean (*GlyphAssessment)(GlyphAgent *glyphAgent);

static SFBoolean ApplyRuleSetTable(TextProcessorRef textProcessor,
    Data ruleSet, GlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean ApplyRuleTable(TextProcessorRef textProcessor,
    Data rule, SFBoolean includeFirst, GlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean ApplyChainRuleSetTable(TextProcessorRef textProcessor,
    Data chainRuleSet, GlyphAssessment glyphAsessment, void *helperPtr);
static SFBoolean ApplyChainRuleTable(TextProcessorRef textProcessor,
    Data chainRule, SFBoolean includeFirst, GlyphAssessment glyphAsessment, void *helperPtr);

static SFBoolean ApplyContextLookups(TextProcessorRef textProcessor,
    Data lookupArray, SFUInteger lookupCount, SFUInteger contextStart, SFUInteger contextEnd);

static SFBoolean AssessGlyphByEquality(GlyphAgent *glyphAgent)
{
    return (glyphAgent->glyphID == glyphAgent->recordValue);
}

static SFBoolean AssessGlyphByClass(GlyphAgent *glyphAgent)
{
    Data classDef = NULL;
    SFUInt16 glyphClass;

    switch (glyphAgent->glyphZone) {
        case GlyphZoneInput:
            classDef = ((Data *)glyphAgent->helperPtr)[0];
            break;

        case GlyphZoneBacktrack:
            classDef = ((Data *)glyphAgent->helperPtr)[1];
            break;

        case GlyphZoneLookahead:
            classDef = ((Data *)glyphAgent->helperPtr)[2];
            break;
    }

    glyphClass = SearchGlyphClass(classDef, glyphAgent->glyphID);

    return (glyphClass == glyphAgent->recordValue);
}

static SFBoolean AssessGlyphByCoverage(GlyphAgent *glyphAgent)
{
    Data coverage = Data_Subdata((Data)glyphAgent->helperPtr, glyphAgent->recordValue);
    SFUInteger covIndex;

    covIndex = SearchCoverageIndex(coverage, glyphAgent->glyphID);

    return (covIndex != SFInvalidIndex);
}

static SFBoolean AssessBacktrackGlyphs(TextProcessorRef textProcessor,
    Data valueArray, SFUInteger valueCount, GlyphAssessment glyphAssessment, void *helperPtr)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger backIndex = locator->index;
    SFUInteger valueIndex;
    GlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = GlyphZoneBacktrack;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        backIndex = LocatorGetBefore(locator, backIndex, SFFalse);

        if (backIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, backIndex);
            glyphAgent.recordValue = UInt16Array_Value(valueArray, valueIndex);

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
    GlyphAssessment glyphAssessment, void *helperPtr, SFUInteger *contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger inputIndex = locator->index;
    SFUInteger valueIndex = 0;
    GlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = GlyphZoneInput;

    if (includeFirst) {
        glyphAgent.glyphID = SFAlbumGetGlyph(album, inputIndex);
        glyphAgent.recordValue = UInt16Array_Value(valueArray, 0);

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
            glyphAgent.recordValue = UInt16Array_Value(valueArray, valueIndex);

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
    GlyphAssessment glyphAssessment, void *helperPtr, SFUInteger contextEnd)
{
    SFAlbumRef album = textProcessor->_album;
    LocatorRef locator = &textProcessor->_locator;
    SFUInteger aheadIndex = contextEnd;
    SFUInteger valueIndex;
    GlyphAgent glyphAgent;

    glyphAgent.helperPtr = helperPtr;
    glyphAgent.glyphZone = GlyphZoneLookahead;

    for (valueIndex = 0; valueIndex < valueCount; valueIndex++) {
        aheadIndex = LocatorGetAfter(locator, aheadIndex, SFFalse);

        if (aheadIndex != SFInvalidIndex) {
            glyphAgent.glyphID = SFAlbumGetGlyph(album, aheadIndex);
            glyphAgent.recordValue = UInt16Array_Value(valueArray, valueIndex);

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

    tblFormat = Context_Format(context);

    switch (tblFormat) {
        case 1: {
            Data coverage = ContextF1_CoverageTable(context);
            SFUInt16 ruleSetCount = ContextF1_RuleSetCount(context);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex < ruleSetCount) {
                Data ruleSet = ContextF1_RuleSetTable(context, covIndex);
                return ApplyRuleSetTable(textProcessor, ruleSet, AssessGlyphByEquality, NULL);
            }
            break;
        }

        case 2: {
            Data coverage = ContextF2_CoverageTable(context);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data classDef = ContextF2_ClassDefTable(context);
                SFUInt16 ruleSetCount = ContextF2_RuleSetCount(context);
                SFUInt16 locClass;

                locClass = SearchGlyphClass(classDef, locGlyph);

                if (locClass < ruleSetCount) {
                    Data ruleSet = ContextF2_RuleSetTable(context, locClass);
                    return ApplyRuleSetTable(textProcessor, ruleSet, AssessGlyphByClass, &classDef);
                }
            }
            break;
        }

        case 3: {
            Data rule = ContextF3_Rule(context);
            return ApplyRuleTable(textProcessor, rule, SFTrue, AssessGlyphByCoverage,
                                  (void *) context);
        }
    }

    return SFFalse;
}

static SFBoolean ApplyRuleSetTable(TextProcessorRef textProcessor,
    Data ruleSet, GlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = RuleSet_RuleCount(ruleSet);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        SFOffset ruleOffset = RuleSet_RuleOffset(ruleSet, ruleIndex);

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
    Data rule, SFBoolean includeFirst, GlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 glyphCount = Rule_GlyphCount(rule);

    /* Make sure that rule table contains at least one glyph. */
    if (glyphCount > 0) {
        SFUInt16 lookupCount = Rule_LookupCount(rule);
        Data valueArray = Rule_ValueArray(rule);
        Data lookupArray = Rule_LookupArray(rule, glyphCount - !includeFirst);
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

    tblFormat = ChainContext_Format(chainContext);

    switch (tblFormat) {
        case 1: {
            Data coverage = ChainContextF1_CoverageTable(chainContext);
            SFUInt16 ruleSetCount = ChainContextF1_ChainRuleSetCount(chainContext);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex < ruleSetCount) {
                Data chainRuleSet = ChainContextF1_ChainRuleSetTable(chainContext, covIndex);
                return ApplyChainRuleSetTable(textProcessor, chainRuleSet, AssessGlyphByEquality, NULL);
            }
            break;
        }

        case 2: {
            Data coverage = ChainContextF2_CoverageTable(chainContext);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data backtrackClassDef = ChainContextF2_BacktrackClassDefTable(chainContext);
                Data inputClassDef = ChainContextF2_InputClassDefTable(chainContext);
                Data lookaheadClassDef = ChainContextF2_LookaheadClassDefTable(chainContext);
                SFUInt16 chainRuleSetCount = ChainContextF2_ChainRuleSetCount(chainContext);
                SFUInt16 inputClass;

                inputClass = SearchGlyphClass(inputClassDef, locGlyph);

                if (inputClass < chainRuleSetCount) {
                    Data chainRuleSet = ChainContextF2_ChainRuleSetTable(chainContext, inputClass);
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
            Data chainRule = ChainContextF3_ChainRuleTable(chainContext);
            return ApplyChainRuleTable(textProcessor, chainRule, SFTrue, AssessGlyphByCoverage,
                                       (void *) chainContext);
        }
    }

    return SFFalse;
}

static SFBoolean ApplyChainRuleSetTable(TextProcessorRef textProcessor,
    Data chainRuleSet, GlyphAssessment glyphAsessment, void *helperPtr)
{
    SFUInt16 ruleCount = ChainRuleSet_ChainRuleCount(chainRuleSet);
    SFUInteger ruleIndex;

    /* Match each rule sequentially as they are ordered by preference. */
    for (ruleIndex = 0; ruleIndex < ruleCount; ruleIndex++) {
        Data chainRule = ChainRuleSet_ChainRuleTable(chainRuleSet, ruleIndex);

        if (ApplyChainRuleTable(textProcessor, chainRule, SFFalse, glyphAsessment, helperPtr)) {
            return SFTrue;
        }
    }

    return SFFalse;
}

static SFBoolean ApplyChainRuleTable(TextProcessorRef textProcessor,
    Data chainRule, SFBoolean includeFirst, GlyphAssessment glyphAsessment, void *helperPtr)
{
    Data backtrackRecord = ChainRule_BacktrackRecord(chainRule);
    SFUInt16 backtrackCount = BacktrackRecord_GlyphCount(backtrackRecord);
    Data backtrackArray = BacktrackRecord_ValueArray(backtrackRecord);
    Data inputRecord = BacktrackRecord_InputRecord(backtrackRecord, backtrackCount);
    SFUInt16 inputCount = InputRecord_GlyphCount(inputRecord);

    /* Make sure that input record has at least one glyph. */
    if (inputCount > 0) {
        Data inputArray = InputRecord_ValueArray(inputRecord);
        Data lookaheadRecord = InputRecord_LookaheadRecord(inputRecord, inputCount - !includeFirst);
        SFUInt16 lookaheadCount = LookaheadRecord_GlyphCount(lookaheadRecord);
        Data lookaheadArray = LookaheadRecord_ValueArray(lookaheadRecord);
        Data contextRecord = LookaheadRecord_ContextRecord(lookaheadRecord, lookaheadCount);
        SFUInteger lookupCount = ContextRecord_LookupCount(contextRecord);
        Data lookupArray = ContextRecord_LookupArray(contextRecord);
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
    LocatorRef locator = &textProcessor->_locator;
    LocatorFilter orgFilter = locator->filter;
    SFRange orgRange = locator->range;
    SFUInteger contextLength = (contextEnd - contextStart) + 1;
    SFUInteger lookupIndex;

    /* Make the locator cover only context range. */
    LocatorReset(locator, contextStart, contextLength);

    /* Apply the lookup records sequentially as they are ordered by preference. */
    for (lookupIndex = 0; lookupIndex < lookupCount; lookupIndex++) {
        Data lookupRecord = LookupArray_Value(lookupArray, lookupIndex);
        SFUInt16 sequenceIndex = LookupRecord_SequenceIndex(lookupRecord);
        SFUInt16 lookupListIndex = LookupRecord_LookupListIndex(lookupRecord);

        /* Jump the locator to context index. */
        LocatorJumpTo(locator, contextStart);

        /* Skip the glyphs till sequence index. */
        if (LocatorSkip(locator, sequenceIndex + 1)) {
            /* Apply the specified lookup. */
            ApplyLookup(textProcessor, lookupListIndex);
            /* Restore the original filter. */
            LocatorUpdateFilter(locator, &orgFilter);
        }
    }

    /* Adjust the locator range. */
    LocatorAdjustRange(locator, orgRange.start, orgRange.count + locator->range.count - contextLength);

    return SFTrue;
}

SF_PRIVATE SFBoolean ApplyExtensionSubtable(TextProcessorRef textProcessor, Data extension)
{
    SFUInt16 tblFormat = Extension_Format(extension);

    switch (tblFormat) {
        case 1: {
            LookupType lookupType = ExtensionF1_LookupType(extension);
            Data innerSubtable = ExtensionF1_ExtensionData(extension);

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

    substFormat = ReverseChainSubst_Format(reverseChain);

    switch (substFormat) {
        case 1: {
            Data coverage = ReverseChainSubstF1_CoverageTable(reverseChain);
            SFGlyphID locGlyph;
            SFUInteger covIndex;

            locGlyph = SFAlbumGetGlyph(album, locator->index);
            covIndex = SearchCoverageIndex(coverage, locGlyph);

            if (covIndex != SFInvalidIndex) {
                Data backtrackRecord = ReverseChainSubstF1_RevBacktrackRecord(reverseChain);
                SFUInt16 backtrackCount = RevBacktrackRecord_GlyphCount(backtrackRecord);
                Data backtrackOffsets = RevBacktrackRecord_CoverageOffsets(backtrackRecord);
                Data lookaheadRecord = RevBacktrackRecord_RevLookaheadRecord(backtrackRecord, backtrackCount);
                SFUInt16 lookaheadCount = RevLookaheadRecord_GlyphCount(lookaheadRecord);
                Data lookaheadOffsets = RevLookaheadRecord_CoverageOffsets(lookaheadRecord);
                Data substRecord = RevLookaheadRecord_RevSubstRecord(lookaheadRecord, lookaheadCount);
                SFUInt16 substCount = RevSubstRecord_GlyphCount(substRecord);

                if (AssessBacktrackGlyphs(textProcessor, backtrackOffsets, backtrackCount, AssessGlyphByCoverage, (void *)reverseChain)
                    && AssessLookaheadGlyphs(textProcessor, lookaheadOffsets, lookaheadCount, AssessGlyphByCoverage, (void *)reverseChain, locator->index)
                    && covIndex < substCount) {
                    SFGlyphID subGlyph = RevSubstRecord_Substitute(substRecord, covIndex);
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
