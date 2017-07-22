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

#include <cstdlib>
#include <cstring>

extern "C" {
#include <Source/SFAlbum.h>
#include <Source/SFAssert.h>
#include <Source/SFLocator.h>
}

#include "OpenType/GDEF.h"
#include "LocatorTester.h"

using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

static const SFCodepoint CODEPOINT_ARRAY[] = { 'A' };
static const SFUInteger CODEPOINT_COUNT = 1;

static const SBCodepointSequence CODEPOINT_SEQUENCE = {
    SBStringEncodingUTF32,
    (void *)CODEPOINT_ARRAY,
    CODEPOINT_COUNT
};

static SFCodepoints SFCodepointsMake(const SBCodepointSequence *referral, SFBoolean backward)
{
    SFCodepoints codepoints;
    SFCodepointsInitialize(&codepoints, referral, backward);

    return codepoints;
}

static SFCodepoints CODEPOINT_HANDLER = SFCodepointsMake(&CODEPOINT_SEQUENCE, SFFalse);

static const SFGlyphTraits TRAIT_LIST_1[] = {
    SFGlyphTraitNone,
};

static const SFGlyphTraits TRAIT_LIST_2[] = {
    SFGlyphTraitBase,
};

static const SFGlyphTraits TRAIT_LIST_3[] = {
    SFGlyphTraitLigature,
};

static const SFGlyphTraits TRAIT_LIST_4[] = {
    SFGlyphTraitMark,
};

static const SFGlyphTraits TRAIT_LIST_5[] = {
    SFGlyphTraitComponent,
};

static const SFGlyphTraits TRAIT_LIST_6[] = {
    SFGlyphTraitPlaceholder,
};

static const SFGlyphTraits TRAIT_LIST_7[] = {
    SFGlyphTraitNone,
    SFGlyphTraitNone,
    SFGlyphTraitNone,
    SFGlyphTraitNone,
    SFGlyphTraitNone,
    SFGlyphTraitNone,
};

static const SFGlyphTraits TRAIT_LIST_8[] = {
    SFGlyphTraitBase,
    SFGlyphTraitBase,
    SFGlyphTraitBase,
    SFGlyphTraitBase,
    SFGlyphTraitBase,
    SFGlyphTraitBase,
};

static const SFGlyphTraits TRAIT_LIST_9[] = {
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
};

static const SFGlyphTraits TRAIT_LIST_10[] = {
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
};

static const SFGlyphTraits TRAIT_LIST_11[] = {
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
};

static const SFGlyphTraits TRAIT_LIST_12[] = {
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
};

static const SFGlyphTraits TRAIT_LIST_13[] = {
    SFGlyphTraitNone,
    SFGlyphTraitBase,
    SFGlyphTraitLigature,
    SFGlyphTraitMark,
    SFGlyphTraitComponent,
    SFGlyphTraitPlaceholder,
};

static const SFGlyphTraits TRAIT_LIST_14[] = {
    SFGlyphTraitNone,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitBase,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitLigature,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitMark,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitComponent,
    SFGlyphTraitPlaceholder,
};

static const SFGlyphTraits TRAIT_LIST_15[] = {
    SFGlyphTraitNone,
    SFGlyphTraitBase,
    SFGlyphTraitBase,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitLigature,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitMark,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitComponent,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
    SFGlyphTraitPlaceholder,
};

static const SFLookupFlag LOOKUP_FLAG_LIST[] = {
    0,
    SFLookupFlagIgnoreBaseGlyphs,
    SFLookupFlagIgnoreLigatures,
    SFLookupFlagIgnoreMarks,
    SFLookupFlagIgnoreBaseGlyphs | SFLookupFlagIgnoreLigatures,
    SFLookupFlagIgnoreBaseGlyphs | SFLookupFlagIgnoreMarks,
    SFLookupFlagIgnoreLigatures | SFLookupFlagIgnoreMarks,
    SFLookupFlagIgnoreBaseGlyphs | SFLookupFlagIgnoreLigatures | SFLookupFlagIgnoreMarks,
};

static SFAlbumRef SFAlbumCreateWithTraits(const SFGlyphTraits *traits, SFUInteger count)
{
    SFAlbumRef album = SFAlbumCreate();
    SFAlbumReset(album, &CODEPOINT_HANDLER, 1);

    SFAlbumBeginFilling(album);
    SFAlbumReserveGlyphs(album, 0, count);

    for (SFUInteger i = 0; i < count; i++) {
        SFAlbumSetGlyph(album, i, (SFGlyphID)i);
        SFAlbumSetFeatureMask(album, i, 0);
        SFAlbumSetTraits(album, i, traits[i]);
        SFAlbumSetAssociation(album, i, 0);
    }

    SFAlbumEndFilling(album);

    return album;
}

static SFBoolean isIgnored(SFGlyphTraits traits, SFLookupFlag lookupFlag)
{
    if (traits & SFGlyphTraitPlaceholder) {
        return SFTrue;
    }

    if (lookupFlag & SFLookupFlagIgnoreBaseGlyphs && traits & SFGlyphTraitBase) {
        return SFTrue;
    }

    if (lookupFlag & SFLookupFlagIgnoreLigatures && traits & SFGlyphTraitLigature) {
        return SFTrue;
    }

    if (lookupFlag & SFLookupFlagIgnoreMarks && traits & SFGlyphTraitMark) {
        return SFTrue;
    }

    return SFFalse;
}

static SFInteger getNext(const SFGlyphTraits *traits, SFInteger count, SFInteger start, SFLookupFlag lookupFlag)
{
    for (SFInteger i = start + 1; i < count; i++) {
        SFGlyphTraits current = traits[i];
        if (!isIgnored(current, lookupFlag)) {
            return i;
        }
    }

    return -1;
}

static SFInteger getPrevious(const SFGlyphTraits *traits, SFInteger count, SFInteger start, SFLookupFlag lookupFlag)
{
    for (SFInteger i = start - 1; i >= 0; i--) {
        SFGlyphTraits current = traits[i];
        if (!isIgnored(current, lookupFlag)) {
            return i;
        }
    }

    return -1;
}

static void testMoveNext(const SFGlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, NULL);

    const SFLookupFlag *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFLookupFlag);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFLookupFlag lookupFlag = lookupFlagArray[i];
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        SFLocatorReset(&locator, 0, (SFUInteger)count);
        SFLocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            start = getNext(traits, count, start, lookupFlag);
            hasNext = SFLocatorMoveNext(&locator);

            if (start > -1) {
                SFAssert(hasNext == SFTrue);
                SFAssert(locator.index == (SFUInteger)start);
            } else {
                SFAssert(hasNext == SFFalse);
                SFAssert(locator.index == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testSkip(const SFGlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, NULL);

    const SFLookupFlag *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFLookupFlag);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFLookupFlag lookupFlag = lookupFlagArray[i];
        SFUInteger skipCount = 1;
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        SFLocatorReset(&locator, 0, (SFUInteger)count);
        SFLocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            for (SFUInteger j = 0; j < skipCount; j++) {
                start = getNext(traits, count, start, lookupFlag);
                if (start == -1) {
                    break;
                }
            }
            hasNext = SFLocatorSkip(&locator, skipCount);

            if (start > -1) {
                SFAssert(hasNext == SFTrue);
                SFAssert(locator.index == (SFUInteger)start);
            } else {
                SFAssert(hasNext == SFFalse);
                SFAssert(locator.index == SFInvalidIndex);
                break;
            }

            skipCount++;
        }
    }

    SFAlbumRelease(album);
}

static void testJumpTo(const SFGlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, NULL);

    const SFLookupFlag *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFLookupFlag);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFLookupFlag lookupFlag = lookupFlagArray[i];
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        SFLocatorReset(&locator, 0, (SFUInteger)count);
        SFLocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            SFLocatorJumpTo(&locator, (SFUInteger)(start + 1));

            start = getNext(traits, count, start, lookupFlag);
            hasNext = SFLocatorMoveNext(&locator);

            if (start > -1) {
                SFAssert(hasNext == SFTrue);
                SFAssert(locator.index == (SFUInteger)start);
            } else {
                SFAssert(hasNext == SFFalse);
                SFAssert(locator.index == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testGetAfter(const SFGlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, NULL);

    const SFLookupFlag *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFLookupFlag);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFLookupFlag lookupFlag = lookupFlagArray[i];
        SFUInteger afterIndex = SFInvalidIndex;
        SFInteger start = 0;

        SFLocatorReset(&locator, 0, (SFUInteger)count);
        SFLocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            afterIndex = SFLocatorGetAfter(&locator, (SFUInteger)start);
            start = getNext(traits, count, start, lookupFlag);

            if (start > -1) {
                SFAssert(afterIndex == (SFUInteger)start);
            } else {
                SFAssert(afterIndex == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testGetBefore(const SFGlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, NULL);

    const SFLookupFlag *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFLookupFlag);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFLookupFlag lookupFlag = lookupFlagArray[i];
        SFUInteger beforeIndex = SFInvalidIndex;
        SFInteger start = count - 1;

        SFLocatorReset(&locator, 0, (SFUInteger)count);
        SFLocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            beforeIndex = SFLocatorGetBefore(&locator, (SFUInteger)start);
            start = getPrevious(traits, count, start, lookupFlag);

            if (start > -1) {
                SFAssert(beforeIndex == (SFUInteger)start);
            } else {
                SFAssert(beforeIndex == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

LocatorTester::LocatorTester()
{
    UInt16 classValueArray[10];
    for (int i = 0; i < 10; i++) {
        classValueArray[i] = (i % 2);
    }

    ClassDefTable markAttachClass;
    markAttachClass.classFormat = 1;
    markAttachClass.format1.startGlyph = 0;
    markAttachClass.format1.glyphCount = 10;
    markAttachClass.format1.classValueArray = classValueArray;

    Glyph glyphs[5];
    for (int i = 0; i < 5; i++) {
        glyphs[i] = (Glyph)(i * 2);
    }

    CoverageTable markGlyphCoverage;
    markGlyphCoverage.coverageFormat = 1;
    markGlyphCoverage.format1.glyphArray = glyphs;
    markGlyphCoverage.format1.glyphCount = 5;

    MarkGlyphSetsDefTable markGlyphSets;
    markGlyphSets.markSetTableFormat = 1;
    markGlyphSets.markSetCount = 1;
    markGlyphSets.coverage = &markGlyphCoverage;

    GDEF gdef;
    gdef.version = 0x00010002;
    gdef.glyphClassDef = NULL;
    gdef.attachList = NULL;
    gdef.ligCaretList = NULL;
    gdef.markAttachClassDef = &markAttachClass;
    gdef.markGlyphSetsDef = &markGlyphSets;

    Writer writer;
    writer.write(&gdef);

    m_gdef = new uint8_t[writer.size()];
    memcpy(m_gdef, writer.data(), (size_t)writer.size());
}

LocatorTester::~LocatorTester()
{
    delete [] m_gdef;
}

void LocatorTester::testMoveNext()
{
    ::testMoveNext(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(SFGlyphTraits));
    ::testMoveNext(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(SFGlyphTraits));
}

void LocatorTester::testSkip()
{
    ::testSkip(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(SFGlyphTraits));
    ::testSkip(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(SFGlyphTraits));
}

void LocatorTester::testJumpTo()
{
    ::testJumpTo(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(SFGlyphTraits));
    ::testJumpTo(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(SFGlyphTraits));
}

void LocatorTester::testGetAfter()
{
    ::testGetAfter(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(SFGlyphTraits));
    ::testGetAfter(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(SFGlyphTraits));
}

void LocatorTester::testGetBefore()
{
    ::testGetBefore(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(SFGlyphTraits));
    ::testGetBefore(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(SFGlyphTraits));
}

void LocatorTester::testMarkFilteringSet()
{
    int count = 10;
    SFGlyphTraits traits[count];
    for (int i = 0; i < count; i++) {
        traits[i] = SFGlyphTraitMark;
    }

    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, m_gdef);
    SFLocatorReset(&locator, 0, (SFUInteger)count);
    SFLocatorSetLookupFlag(&locator, SFLookupFlagUseMarkFilteringSet);
    SFLocatorSetMarkFilteringSet(&locator, 0);

    /* Zero mark filtering set contains even glyphs, so we should get only those. */
    while (SFLocatorMoveNext(&locator)) {
        SFGlyphID glyph = SFAlbumGetGlyph(album, locator.index);
        SFAssert((glyph % 2) == 0);
    }

    SFAlbumRelease(album);
}

void LocatorTester::testMarkAttachmentType()
{
    int count = 10;
    SFGlyphTraits traits[count];
    for (int i = 0; i < count; i++) {
        traits[i] = SFGlyphTraitMark;
    }

    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    SFLocator locator;
    SFLocatorInitialize(&locator, album, m_gdef);
    SFLocatorReset(&locator, 0, (SFUInteger)count);
    SFLocatorSetLookupFlag(&locator, 0x0100);

    /* Class 1 contains odd glyphs, so we should get only those. */
    while (SFLocatorMoveNext(&locator)) {
        SFGlyphID glyph = SFAlbumGetGlyph(album, locator.index);
        SFAssert((glyph % 2) == 1);
    }

    SFAlbumRelease(album);
}

void LocatorTester::test()
{
    testMoveNext();
    testSkip();
    testJumpTo();
    testGetAfter();
    testGetBefore();
    testMarkFilteringSet();
    testMarkAttachmentType();
}
