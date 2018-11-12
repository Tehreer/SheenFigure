/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#include <cassert>
#include <cstdlib>
#include <cstring>

extern "C" {
#include <Source/SFAlbum.h>
#include <Source/Locator.h>
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

static const GlyphTraits TRAIT_LIST_1[] = {
    GlyphTraitNone,
};

static const GlyphTraits TRAIT_LIST_2[] = {
    GlyphTraitBase,
};

static const GlyphTraits TRAIT_LIST_3[] = {
    GlyphTraitLigature,
};

static const GlyphTraits TRAIT_LIST_4[] = {
    GlyphTraitMark,
};

static const GlyphTraits TRAIT_LIST_5[] = {
    GlyphTraitComponent,
};

static const GlyphTraits TRAIT_LIST_6[] = {
    GlyphTraitPlaceholder,
};

static const GlyphTraits TRAIT_LIST_7[] = {
    GlyphTraitNone,
    GlyphTraitNone,
    GlyphTraitNone,
    GlyphTraitNone,
    GlyphTraitNone,
    GlyphTraitNone,
};

static const GlyphTraits TRAIT_LIST_8[] = {
    GlyphTraitBase,
    GlyphTraitBase,
    GlyphTraitBase,
    GlyphTraitBase,
    GlyphTraitBase,
    GlyphTraitBase,
};

static const GlyphTraits TRAIT_LIST_9[] = {
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitLigature,
};

static const GlyphTraits TRAIT_LIST_10[] = {
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
};

static const GlyphTraits TRAIT_LIST_11[] = {
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
};

static const GlyphTraits TRAIT_LIST_12[] = {
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
};

static const GlyphTraits TRAIT_LIST_13[] = {
    GlyphTraitNone,
    GlyphTraitBase,
    GlyphTraitLigature,
    GlyphTraitMark,
    GlyphTraitComponent,
    GlyphTraitPlaceholder,
};

static const GlyphTraits TRAIT_LIST_14[] = {
    GlyphTraitNone,
    GlyphTraitPlaceholder,
    GlyphTraitBase,
    GlyphTraitPlaceholder,
    GlyphTraitLigature,
    GlyphTraitPlaceholder,
    GlyphTraitMark,
    GlyphTraitPlaceholder,
    GlyphTraitComponent,
    GlyphTraitPlaceholder,
};

static const GlyphTraits TRAIT_LIST_15[] = {
    GlyphTraitNone,
    GlyphTraitBase,
    GlyphTraitBase,
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitLigature,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitMark,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitComponent,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
    GlyphTraitPlaceholder,
};

static const SFUInt16 LOOKUP_FLAG_LIST[] = {
    0,
    LookupFlagIgnoreBaseGlyphs,
    LookupFlagIgnoreLigatures,
    LookupFlagIgnoreMarks,
    LookupFlagIgnoreBaseGlyphs | LookupFlagIgnoreLigatures,
    LookupFlagIgnoreBaseGlyphs | LookupFlagIgnoreMarks,
    LookupFlagIgnoreLigatures | LookupFlagIgnoreMarks,
    LookupFlagIgnoreBaseGlyphs | LookupFlagIgnoreLigatures | LookupFlagIgnoreMarks,
};

static SFAlbumRef SFAlbumCreateWithTraits(const GlyphTraits *traits, SFUInteger count)
{
    SFAlbumRef album = SFAlbumCreate();
    SFAlbumReset(album, &CODEPOINT_HANDLER);

    SFAlbumBeginFilling(album);
    SFAlbumReserveGlyphs(album, 0, count);

    for (SFUInteger i = 0; i < count; i++) {
        SFAlbumSetGlyph(album, i, (SFGlyphID)i);
        SFAlbumSetFeatureMask(album, i, 0);
        SFAlbumReplaceBasicTraits(album, i, traits[i]);
        SFAlbumSetAssociation(album, i, 0);
    }

    SFAlbumEndFilling(album);

    return album;
}

static SFBoolean isIgnored(GlyphTraits traits, SFUInt16 lookupFlag)
{
    if (traits & GlyphTraitPlaceholder) {
        return SFTrue;
    }

    if (lookupFlag & LookupFlagIgnoreBaseGlyphs && traits & GlyphTraitBase) {
        return SFTrue;
    }

    if (lookupFlag & LookupFlagIgnoreLigatures && traits & GlyphTraitLigature) {
        return SFTrue;
    }

    if (lookupFlag & LookupFlagIgnoreMarks && traits & GlyphTraitMark) {
        return SFTrue;
    }

    return SFFalse;
}

static SFInteger getNext(const GlyphTraits *traits, SFInteger count, SFInteger start, SFUInt16 lookupFlag)
{
    for (SFInteger i = start + 1; i < count; i++) {
        GlyphTraits current = traits[i];
        if (!isIgnored(current, lookupFlag)) {
            return i;
        }
    }

    return -1;
}

static SFInteger getPrevious(const GlyphTraits *traits, SFInteger count, SFInteger start, SFUInt16 lookupFlag)
{
    for (SFInteger i = start - 1; i >= 0; i--) {
        GlyphTraits current = traits[i];
        if (!isIgnored(current, lookupFlag)) {
            return i;
        }
    }

    return -1;
}

static void testMoveNext(const GlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, NULL);

    const SFUInt16 *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFUInt16);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFUInt16 lookupFlag = lookupFlagArray[i];
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        LocatorReset(&locator, 0, (SFUInteger)count);
        LocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            start = getNext(traits, count, start, lookupFlag);
            hasNext = LocatorMoveNext(&locator);

            if (start > -1) {
                assert(hasNext == SFTrue);
                assert(locator.index == (SFUInteger)start);
            } else {
                assert(hasNext == SFFalse);
                assert(locator.index == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testSkip(const GlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, NULL);

    const SFUInt16 *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFUInt16);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFUInt16 lookupFlag = lookupFlagArray[i];
        SFUInteger skipCount = 1;
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        LocatorReset(&locator, 0, (SFUInteger)count);
        LocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            for (SFUInteger j = 0; j < skipCount; j++) {
                start = getNext(traits, count, start, lookupFlag);
                if (start == -1) {
                    break;
                }
            }
            hasNext = LocatorSkip(&locator, skipCount);

            if (start > -1) {
                assert(hasNext == SFTrue);
                assert(locator.index == (SFUInteger)start);
            } else {
                assert(hasNext == SFFalse);
                assert(locator.index == SFInvalidIndex);
                break;
            }

            skipCount++;
        }
    }

    SFAlbumRelease(album);
}

static void testJumpTo(const GlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, NULL);

    const SFUInt16 *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFUInt16);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFUInt16 lookupFlag = lookupFlagArray[i];
        SFBoolean hasNext = SFFalse;
        SFInteger start = -1;

        LocatorReset(&locator, 0, (SFUInteger)count);
        LocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            LocatorJumpTo(&locator, (SFUInteger)(start + 1));

            start = getNext(traits, count, start, lookupFlag);
            hasNext = LocatorMoveNext(&locator);

            if (start > -1) {
                assert(hasNext == SFTrue);
                assert(locator.index == (SFUInteger)start);
            } else {
                assert(hasNext == SFFalse);
                assert(locator.index == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testGetAfter(const GlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, NULL);

    const SFUInt16 *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFUInt16);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFUInt16 lookupFlag = lookupFlagArray[i];
        SFUInteger afterIndex = SFInvalidIndex;
        SFInteger start = 0;

        LocatorReset(&locator, 0, (SFUInteger)count);
        LocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            afterIndex = LocatorGetAfter(&locator, (SFUInteger)start, SFTrue);
            start = getNext(traits, count, start, lookupFlag);

            if (start > -1) {
                assert(afterIndex == (SFUInteger)start);
            } else {
                assert(afterIndex == SFInvalidIndex);
                break;
            }
        }
    }

    SFAlbumRelease(album);
}

static void testGetBefore(const GlyphTraits *traits, SFInteger count)
{
    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, NULL);

    const SFUInt16 *lookupFlagArray = LOOKUP_FLAG_LIST;
    SFInteger lookupFlagCount = sizeof(LOOKUP_FLAG_LIST) / sizeof(SFUInt16);

    for (SFInteger i = 0; i < lookupFlagCount; i++) {
        SFUInt16 lookupFlag = lookupFlagArray[i];
        SFUInteger beforeIndex = SFInvalidIndex;
        SFInteger start = count - 1;

        LocatorReset(&locator, 0, (SFUInteger)count);
        LocatorSetLookupFlag(&locator, lookupFlag);

        while (true) {
            beforeIndex = LocatorGetBefore(&locator, (SFUInteger)start, SFTrue);
            start = getPrevious(traits, count, start, lookupFlag);

            if (start > -1) {
                assert(beforeIndex == (SFUInteger)start);
            } else {
                assert(beforeIndex == SFInvalidIndex);
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
    ::testMoveNext(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(GlyphTraits));
    ::testMoveNext(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(GlyphTraits));
}

void LocatorTester::testSkip()
{
    ::testSkip(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(GlyphTraits));
    ::testSkip(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(GlyphTraits));
}

void LocatorTester::testJumpTo()
{
    ::testJumpTo(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(GlyphTraits));
    ::testJumpTo(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(GlyphTraits));
}

void LocatorTester::testGetAfter()
{
    ::testGetAfter(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(GlyphTraits));
    ::testGetAfter(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(GlyphTraits));
}

void LocatorTester::testGetBefore()
{
    ::testGetBefore(TRAIT_LIST_1, sizeof(TRAIT_LIST_1) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_2, sizeof(TRAIT_LIST_2) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_3, sizeof(TRAIT_LIST_3) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_4, sizeof(TRAIT_LIST_4) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_5, sizeof(TRAIT_LIST_5) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_6, sizeof(TRAIT_LIST_6) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_7, sizeof(TRAIT_LIST_7) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_8, sizeof(TRAIT_LIST_8) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_9, sizeof(TRAIT_LIST_9) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_10, sizeof(TRAIT_LIST_10) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_11, sizeof(TRAIT_LIST_11) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_12, sizeof(TRAIT_LIST_12) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_13, sizeof(TRAIT_LIST_13) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_14, sizeof(TRAIT_LIST_14) / sizeof(GlyphTraits));
    ::testGetBefore(TRAIT_LIST_15, sizeof(TRAIT_LIST_15) / sizeof(GlyphTraits));
}

void LocatorTester::testMarkFilteringSet()
{
    const int count = 10;
    GlyphTraits traits[count];
    for (int i = 0; i < count; i++) {
        traits[i] = GlyphTraitMark;
    }

    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, m_gdef);
    LocatorReset(&locator, 0, (SFUInteger)count);
    LocatorSetLookupFlag(&locator, LookupFlagUseMarkFilteringSet);
    LocatorSetMarkFilteringSet(&locator, 0);

    /* Zero mark filtering set contains even glyphs, so we should get only those. */
    while (LocatorMoveNext(&locator)) {
        SFGlyphID glyph = SFAlbumGetGlyph(album, locator.index);
        assert((glyph % 2) == 0);
    }

    SFAlbumRelease(album);
}

void LocatorTester::testMarkAttachmentType()
{
    const int count = 10;
    GlyphTraits traits[count];
    for (int i = 0; i < count; i++) {
        traits[i] = GlyphTraitMark;
    }

    SFAlbumRef album = SFAlbumCreateWithTraits(traits, (SFUInteger)count);

    Locator locator;
    LocatorInitialize(&locator, album, m_gdef);
    LocatorReset(&locator, 0, (SFUInteger)count);
    LocatorSetLookupFlag(&locator, 0x0100);

    /* Class 1 contains odd glyphs, so we should get only those. */
    while (LocatorMoveNext(&locator)) {
        SFGlyphID glyph = SFAlbumGetGlyph(album, locator.index);
        assert((glyph % 2) == 1);
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
