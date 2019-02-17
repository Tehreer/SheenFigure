/*
 * Copyright (C) 2016-2019 Muhammad Tayyab Akram
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
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" {
#include <SBCodepointSequence.h>
#include <Source/SFAlbum.h>
#include <Source/SFCodepoints.h>
}

#include "AlbumTester.h"

using namespace std;
using namespace SheenFigure::Tester;

class Codepoints {
public:
    Codepoints(size_t count, bool backward = false)
    {
        array.reserve(count);
        sequence = {
            SBStringEncodingUTF32,
            array.data(),
            count
        };

        SFCodepointsInitialize(&codepoints, &sequence, backward);
    }

    SFCodepointsRef ptr() { return &codepoints; }

private:
    vector<uint32_t> array;
    SBCodepointSequence sequence;
    SFCodepoints codepoints;
};

static void SFAlbumReserveGlyphsInitialized(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    SFUInteger maxIndex = index + count;
    SFUInteger i;

    SFAlbumReserveGlyphs(album, index, count);

    for (i = index; i < maxIndex; i++) {
        SFAlbumSetGlyph(album, i, 0);
        SFAlbumSetFeatureMask(album, i, 0);
        SFAlbumSetAllTraits(album, i, GlyphTraitNone);
        SFAlbumSetAssociation(album, i, 0);
    }
}

AlbumTester::AlbumTester()
{
}

void AlbumTester::testInitialize()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    assert(album.codepoints == NULL);
    assert(album.codeunitCount == 0);
    assert(album.glyphCount == 0);

    SFAlbumFinalize(&album);
}

void AlbumTester::testReset()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test reset just after initialization. */
    {
        Codepoints codepoints(1024);
        SFAlbumReset(&album, codepoints.ptr());

        assert(album.codepoints == codepoints.ptr());
        assert(album.codeunitCount == 1024);
        assert(album.glyphCount == 0);
    }

    /* Test reset after some manipulation. */
    {
        SFAlbumBeginFilling(&album);
        SFAlbumReserveGlyphsInitialized(&album, 0, 5);
        SFAlbumEndFilling(&album);

        Codepoints codepoints(1024);
        SFAlbumReset(&album, codepoints.ptr());

        assert(album.codepoints == codepoints.ptr());
        assert(album.codeunitCount == 1024);
        assert(album.glyphCount == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testAddGlyph()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test with forward associations. */
    {
        Codepoints codepoints(5);
        SFAlbumReset(&album, codepoints.ptr());

        SFAlbumBeginFilling(&album);
        SFAlbumAddGlyph(&album, 100, GlyphTraitNone, 0);
        SFAlbumAddGlyph(&album, 200, GlyphTraitNone, 1);
        SFAlbumAddGlyph(&album, 300, GlyphTraitNone, 2);
        SFAlbumAddGlyph(&album, 400, GlyphTraitNone, 3);
        SFAlbumAddGlyph(&album, 500, GlyphTraitNone, 4);
        SFAlbumEndFilling(&album);

        SFAlbumBeginArranging(&album);
        SFAlbumEndArranging(&album);

        SFAlbumWrapUp(&album);

        /* Test the glyph count. */
        assert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *actualGlyphs = SFAlbumGetGlyphIDsPtr(&album);
        const SFGlyphID expectedGlyphs[] = { 100, 200, 300, 400, 500 };
        assert(memcmp(actualGlyphs, expectedGlyphs, sizeof(expectedGlyphs)) == 0);

        /* Test the output map. */
        const SFUInteger *actualMap = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFInteger expectedMap[] = { 0, 1, 2, 3, 4 };
        assert(memcmp(actualMap, expectedMap, sizeof(expectedMap)) == 0);
    }

    /* Test with backward associations. */
    {
        Codepoints codepoints(5, true);
        SFAlbumReset(&album, codepoints.ptr());

        SFAlbumBeginFilling(&album);
        SFAlbumAddGlyph(&album, 100, GlyphTraitNone, 4);
        SFAlbumAddGlyph(&album, 200, GlyphTraitNone, 3);
        SFAlbumAddGlyph(&album, 300, GlyphTraitNone, 2);
        SFAlbumAddGlyph(&album, 400, GlyphTraitNone, 1);
        SFAlbumAddGlyph(&album, 500, GlyphTraitNone, 0);
        SFAlbumEndFilling(&album);

        SFAlbumBeginArranging(&album);
        SFAlbumEndArranging(&album);

        SFAlbumWrapUp(&album);

        /* Test the glyph count. */
        assert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *actualGlyphs = SFAlbumGetGlyphIDsPtr(&album);
        const SFGlyphID expectedGlyphs[] = { 100, 200, 300, 400, 500 };
        assert(memcmp(actualGlyphs, expectedGlyphs, sizeof(expectedGlyphs)) == 0);

        /* Test the output map. */
        const SFUInteger *actualMap = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFInteger expectedMap[] = { 4, 3, 2, 1, 0 };
        assert(memcmp(actualMap, expectedMap, sizeof(expectedMap)) == 0);
    }

    /* Test by mimicing multiple code units per codepoint. */
    {
        Codepoints codepoints(15);
        SFAlbumReset(&album, codepoints.ptr());

        SFAlbumBeginFilling(&album);
        SFAlbumAddGlyph(&album, 100, GlyphTraitNone, 0);
        SFAlbumAddGlyph(&album, 200, GlyphTraitNone, 1);
        SFAlbumAddGlyph(&album, 300, GlyphTraitNone, 3);
        SFAlbumAddGlyph(&album, 400, GlyphTraitNone, 6);
        SFAlbumAddGlyph(&album, 500, GlyphTraitNone, 10);
        SFAlbumEndFilling(&album);

        SFAlbumBeginArranging(&album);
        SFAlbumEndArranging(&album);

        SFAlbumWrapUp(&album);

        /* Test the glyph count. */
        assert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *actualGlyphs = SFAlbumGetGlyphIDsPtr(&album);
        const SFGlyphID expectedGlyphs[] = { 100, 200, 300, 400, 500 };
        assert(memcmp(actualGlyphs, expectedGlyphs, sizeof(expectedGlyphs)) == 0);

        /* Test the output map. */
        const SFUInteger *actualMap = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFInteger expectedMap[] = { 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4 };
        assert(memcmp(actualMap, expectedMap, sizeof(expectedMap)) == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testReserveGlyphs()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());
    SFAlbumBeginFilling(&album);

    /* Reserve some initial glyphs and place marking. */
    SFAlbumReserveGlyphs(&album, 0, 10);
    SFAlbumSetGlyph(&album, 0, 100);
    SFAlbumSetGlyph(&album, 9, 200);

    /* Reserve some glyphs at the end and place marking. */
    SFAlbumReserveGlyphs(&album, 10, 5);
    SFAlbumSetGlyph(&album, 4, 300);
    SFAlbumSetGlyph(&album, 5, 400);

    /* Reserve some glyphs at the center and place marking. */
    SFAlbumReserveGlyphs(&album, 5, 5);
    SFAlbumSetGlyph(&album, 19, 500);

    /* Reserve some glyphs at the start. */
    SFAlbumReserveGlyphs(&album, 0, 5);

    /* Wrap up the album. */
    for (SFUInteger i = 0; i < 25; i++) {
        SFAlbumReplaceBasicTraits(&album, i, GlyphTraitNone);
        SFAlbumSetAssociation(&album, i, 0);
    }
    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);

    /* Test the glyph count. */
    assert(SFAlbumGetGlyphCount(&album) == 25);

    /* Test the expected locations of output glyphs. */
    const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
    assert(glyphs[5] == 100);
    assert(glyphs[9] == 300);
    assert(glyphs[15] == 400);
    assert(glyphs[19] == 200);
    assert(glyphs[24] == 500);

    SFAlbumFinalize(&album);
}

void AlbumTester::testSetGlyph()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());
    SFAlbumBeginFilling(&album);

    SFAlbumReserveGlyphsInitialized(&album, 0, 5);

    /* Set the reserved glyphs. */
    SFAlbumSetGlyph(&album, 0, 100);
    SFAlbumSetGlyph(&album, 1, 200);
    SFAlbumSetGlyph(&album, 2, 300);
    SFAlbumSetGlyph(&album, 3, 400);
    SFAlbumSetGlyph(&album, 4, 500);

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);

    /* Test the glyph count. */
    assert(SFAlbumGetGlyphCount(&album) == 5);

    /* Test the output glyphs. */
    const SFGlyphID *actual = SFAlbumGetGlyphIDsPtr(&album);
    const SFGlyphID expected[] = { 100, 200, 300, 400, 500 };
    assert(memcmp(actual, expected, sizeof(expected)) == 0);

    SFAlbumFinalize(&album);
}

void AlbumTester::testGetGlyph()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());
    SFAlbumBeginFilling(&album);

    /* Test by adding some glyphs. */
    {
        SFAlbumAddGlyph(&album, 100, GlyphTraitNone, 0);
        SFAlbumAddGlyph(&album, 200, GlyphTraitNone, 0);

        assert(SFAlbumGetGlyph(&album, 0) == 100);
        assert(SFAlbumGetGlyph(&album, 1) == 200);
    }

    /* Test by reserving and setting some glyphs. */
    {
        SFAlbumReserveGlyphsInitialized(&album, 1, 3);
        SFAlbumSetGlyph(&album, 1, 300);
        SFAlbumSetGlyph(&album, 2, 400);
        SFAlbumSetGlyph(&album, 3, 500);

        assert(SFAlbumGetGlyph(&album, 1) == 300);
        assert(SFAlbumGetGlyph(&album, 2) == 400);
        assert(SFAlbumGetGlyph(&album, 3) == 500);
    }

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testSetAssociation()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test with forward associations. */
    {
        Codepoints codepoints(5);

        SFAlbumReset(&album, codepoints.ptr());
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 5);

        SFAlbumSetAssociation(&album, 0, 0);
        SFAlbumSetAssociation(&album, 1, 1);
        SFAlbumSetAssociation(&album, 2, 2);
        SFAlbumSetAssociation(&album, 3, 3);
        SFAlbumSetAssociation(&album, 4, 4);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *actual = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFUInteger expected[] = { 0, 1, 2, 3, 4 };
        assert(memcmp(actual, expected, sizeof(expected)) == 0);
    }

    /* Test with backward associations. */
    {
        Codepoints codepoints(5, true);

        SFAlbumReset(&album, codepoints.ptr());
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 5);

        SFAlbumSetAssociation(&album, 0, 4);
        SFAlbumSetAssociation(&album, 1, 3);
        SFAlbumSetAssociation(&album, 2, 2);
        SFAlbumSetAssociation(&album, 3, 1);
        SFAlbumSetAssociation(&album, 4, 0);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *actual = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFUInteger expected[] = { 4, 3, 2, 1, 0 };
        assert(memcmp(actual, expected, sizeof(expected)) == 0);
    }

    /* Test with complex associations. */
    {
        Codepoints codepoints(5);

        SFAlbumReset(&album, codepoints.ptr());
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 8);

        SFAlbumSetAssociation(&album, 0, 4);
        SFAlbumSetAssociation(&album, 1, 4);
        SFAlbumSetAssociation(&album, 2, 4);
        SFAlbumSetAssociation(&album, 3, 3);
        SFAlbumSetAssociation(&album, 4, 3);
        SFAlbumSetAssociation(&album, 5, 2);
        SFAlbumSetAssociation(&album, 6, 1);
        SFAlbumSetAssociation(&album, 7, 0);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *actual = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        const SFUInteger expected[] = { 7, 6, 5, 3, 0 };
        assert(memcmp(actual, expected, sizeof(expected)) == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testGetAssociation()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());
    SFAlbumBeginFilling(&album);

    /* Test by adding some glyphs. */
    {
        SFAlbumAddGlyph(&album, 0, GlyphTraitNone, 0);
        SFAlbumAddGlyph(&album, 0, GlyphTraitNone, 1);

        /* Test with get single association. */
        assert(SFAlbumGetAssociation(&album, 0) == 0);
        assert(SFAlbumGetAssociation(&album, 1) == 1);
    }

    /* Test by reserving some glyphs. */
    {
        SFAlbumReserveGlyphsInitialized(&album, 1, 3);
        SFAlbumSetAssociation(&album, 1, 2);
        SFAlbumSetAssociation(&album, 2, 3);
        SFAlbumSetAssociation(&album, 3, 4);

        /* Test with get single association. */
        assert(SFAlbumGetAssociation(&album, 1) == 2);
        assert(SFAlbumGetAssociation(&album, 2) == 3);
        assert(SFAlbumGetAssociation(&album, 3) == 4);
    }

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testFeatureMask()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);

    SFUInt16 mask1 = (SFUInt16)rand();
    SFUInt16 mask2 = (SFUInt16)rand();
    SFUInt16 mask3 = (SFUInt16)rand();
    SFUInt16 mask4 = (SFUInt16)rand();
    SFUInt16 mask5 = (SFUInt16)rand();

    SFAlbumSetFeatureMask(&album, 0, mask1);
    SFAlbumSetFeatureMask(&album, 1, mask2);
    SFAlbumSetFeatureMask(&album, 2, mask3);
    SFAlbumSetFeatureMask(&album, 3, mask4);
    SFAlbumSetFeatureMask(&album, 4, mask5);

    /* Test with get feature mask. */
    assert(SFAlbumGetFeatureMask(&album, 0) == mask1);
    assert(SFAlbumGetFeatureMask(&album, 1) == mask2);
    assert(SFAlbumGetFeatureMask(&album, 2) == mask3);
    assert(SFAlbumGetFeatureMask(&album, 3) == mask4);
    assert(SFAlbumGetFeatureMask(&album, 4) == mask5);

    SFAlbumEndFilling(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testTraits()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);

    /* Test by setting traits. */
    {
        SFAlbumReplaceBasicTraits(&album, 0, GlyphTraitNone);
        SFAlbumReplaceBasicTraits(&album, 1, GlyphTraitBase);
        SFAlbumReplaceBasicTraits(&album, 2, GlyphTraitLigature);
        SFAlbumReplaceBasicTraits(&album, 3, GlyphTraitMark);
        SFAlbumReplaceBasicTraits(&album, 4, GlyphTraitComponent);

        assert(SFAlbumGetAllTraits(&album, 0) == GlyphTraitNone);
        assert(SFAlbumGetAllTraits(&album, 1) == GlyphTraitBase);
        assert(SFAlbumGetAllTraits(&album, 2) == GlyphTraitLigature);
        assert(SFAlbumGetAllTraits(&album, 3) == GlyphTraitMark);
        assert(SFAlbumGetAllTraits(&album, 4) == GlyphTraitComponent);
    }

    SFAlbumEndFilling(&album);
    SFAlbumBeginArranging(&album);

    /* Test by inserting traits. */
    {
        SFAlbumInsertHelperTraits(&album, 0, GlyphTraitRightToLeft);
        SFAlbumInsertHelperTraits(&album, 1, GlyphTraitAttached | GlyphTraitResolved);
        SFAlbumInsertHelperTraits(&album, 2, GlyphTraitCursive);
        SFAlbumInsertHelperTraits(&album, 3, GlyphTraitCursive | GlyphTraitRightToLeft);
        SFAlbumInsertHelperTraits(&album, 4, GlyphTraitResolved);

        assert(SFAlbumGetAllTraits(&album, 0) == (GlyphTraitRightToLeft));
        assert(SFAlbumGetAllTraits(&album, 1) == (GlyphTraitBase | GlyphTraitAttached | GlyphTraitResolved));
        assert(SFAlbumGetAllTraits(&album, 2) == (GlyphTraitLigature | GlyphTraitCursive));
        assert(SFAlbumGetAllTraits(&album, 3) == (GlyphTraitMark | GlyphTraitCursive | GlyphTraitRightToLeft));
        assert(SFAlbumGetAllTraits(&album, 4) == (GlyphTraitComponent | GlyphTraitResolved));
    }

    /* Test by removing traits. */
    {
        SFAlbumRemoveHelperTraits(&album, 0, GlyphTraitRightToLeft);
        SFAlbumRemoveHelperTraits(&album, 1, GlyphTraitCursive | GlyphTraitResolved);
        SFAlbumRemoveHelperTraits(&album, 2, GlyphTraitAttached | GlyphTraitCursive);
        SFAlbumRemoveHelperTraits(&album, 3, GlyphTraitRightToLeft);
        SFAlbumRemoveHelperTraits(&album, 4, GlyphTraitAttached | GlyphTraitRightToLeft);

        assert(SFAlbumGetAllTraits(&album, 0) == (GlyphTraitNone));
        assert(SFAlbumGetAllTraits(&album, 1) == (GlyphTraitBase | GlyphTraitAttached));
        assert(SFAlbumGetAllTraits(&album, 2) == (GlyphTraitLigature));
        assert(SFAlbumGetAllTraits(&album, 3) == (GlyphTraitMark | GlyphTraitCursive));
        assert(SFAlbumGetAllTraits(&album, 4) == (GlyphTraitComponent | GlyphTraitResolved));
    }

    SFAlbumEndArranging(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testOffset()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);
    SFAlbumEndFilling(&album);

    SFAlbumBeginArranging(&album);
    SFAlbumSetX(&album, 0, 100), SFAlbumSetY(&album, 0, 150);
    SFAlbumSetX(&album, 1, 200), SFAlbumSetY(&album, 1, 250);
    SFAlbumSetX(&album, 2, 300), SFAlbumSetY(&album, 2, 350);
    SFAlbumSetX(&album, 3, 400), SFAlbumSetY(&album, 3, 450);
    SFAlbumSetX(&album, 4, 500), SFAlbumSetY(&album, 4, 550);

    /* Test with getters. */
    assert(SFAlbumGetX(&album, 0) == 100 && SFAlbumGetY(&album, 0) == 150);
    assert(SFAlbumGetX(&album, 1) == 200 && SFAlbumGetY(&album, 1) == 250);
    assert(SFAlbumGetX(&album, 2) == 300 && SFAlbumGetY(&album, 2) == 350);
    assert(SFAlbumGetX(&album, 3) == 400 && SFAlbumGetY(&album, 3) == 450);
    assert(SFAlbumGetX(&album, 4) == 500 && SFAlbumGetY(&album, 4) == 550);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);

    /* Test the overall output. */
    const SFPoint *actual = SFAlbumGetGlyphOffsetsPtr(&album);
    const SFPoint expected[] = {
        { 100, 150 }, { 200, 250 }, { 300, 350 }, { 400, 450 }, { 500, 550 }
    };
    assert(memcmp(actual, expected, sizeof(expected)) == 0);

    SFAlbumFinalize(&album);
}

void AlbumTester::testAdvance()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);
    SFAlbumEndFilling(&album);

    SFAlbumBeginArranging(&album);
    SFAlbumSetAdvance(&album, 0, 100);
    SFAlbumSetAdvance(&album, 1, 200);
    SFAlbumSetAdvance(&album, 2, 300);
    SFAlbumSetAdvance(&album, 3, 400);
    SFAlbumSetAdvance(&album, 4, 500);

    /* Test with getters. */
    assert(SFAlbumGetAdvance(&album, 0) == 100);
    assert(SFAlbumGetAdvance(&album, 1) == 200);
    assert(SFAlbumGetAdvance(&album, 2) == 300);
    assert(SFAlbumGetAdvance(&album, 3) == 400);
    assert(SFAlbumGetAdvance(&album, 4) == 500);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);

    /* Test the overall output. */
    const SFAdvance *actual = SFAlbumGetGlyphAdvancesPtr(&album);
    const SFAdvance expected[] = { 100, 200, 300, 400, 500 };
    assert(memcmp(actual, expected, sizeof(expected)) == 0);

    SFAlbumFinalize(&album);
}

void AlbumTester::testCursiveOffset()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);
    SFAlbumEndFilling(&album);

    SFAlbumBeginArranging(&album);
    SFAlbumSetCursiveOffset(&album, 0, 1);
    SFAlbumSetCursiveOffset(&album, 1, 2);
    SFAlbumSetCursiveOffset(&album, 2, 3);
    SFAlbumSetCursiveOffset(&album, 3, 4);
    SFAlbumSetCursiveOffset(&album, 4, 5);

    /* Test with getters. */
    assert(SFAlbumGetCursiveOffset(&album, 0) == 1);
    assert(SFAlbumGetCursiveOffset(&album, 1) == 2);
    assert(SFAlbumGetCursiveOffset(&album, 2) == 3);
    assert(SFAlbumGetCursiveOffset(&album, 3) == 4);
    assert(SFAlbumGetCursiveOffset(&album, 4) == 5);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testAttachmentOffset()
{
    Codepoints codepoints(5);

    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, codepoints.ptr());

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);
    SFAlbumEndFilling(&album);

    SFAlbumBeginArranging(&album);
    SFAlbumSetAttachmentOffset(&album, 0, 1);
    SFAlbumSetAttachmentOffset(&album, 1, 2);
    SFAlbumSetAttachmentOffset(&album, 2, 3);
    SFAlbumSetAttachmentOffset(&album, 3, 4);
    SFAlbumSetAttachmentOffset(&album, 4, 5);

    /* Test with getters. */
    assert(SFAlbumGetAttachmentOffset(&album, 0) == 1);
    assert(SFAlbumGetAttachmentOffset(&album, 1) == 2);
    assert(SFAlbumGetAttachmentOffset(&album, 2) == 3);
    assert(SFAlbumGetAttachmentOffset(&album, 3) == 4);
    assert(SFAlbumGetAttachmentOffset(&album, 4) == 5);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

/*
SF_INTERNAL SFFloat LoadCaretEdges(SFUInteger *clusterMap, SFUInteger codeunitCount,
    SFBoolean isBackward, SFBoolean isRTL, SFInt32 *glyphAdvances, SFUInteger glyphCount,
    SFFloat advanceScale, SFBoolean *caretStops, SFFloat *caretEdges)
*/

void AlbumTester::testCaretEdges()
{
    /* Test with single breakable code units and glyph mapping in forward LTR mode. */
    {
        vector<SFUInteger> clusterMap = { 0, 1, 2, 3, 4 };
        vector<SFInt32> glyphAdvances = { 10, 20, 30, 40, 50 };
        vector<SFBoolean> caretStops = { SFTrue, SFTrue, SFTrue, SFTrue, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 0, 10, 30, 60, 100, 150 };

        /* Test without caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(), 1.0, NULL, caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);

        /* Test with caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with single breakable code units and glyph mapping in forward RTL mode. */
    {
        vector<SFUInteger> clusterMap = { 0, 1, 2, 3, 4 };
        vector<SFInt32> glyphAdvances = { 10, 20, 30, 40, 50 };
        vector<SFBoolean> caretStops = { SFTrue, SFTrue, SFTrue, SFTrue, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 150, 140, 120, 90, 50, 0 };

        /* Test without caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFTrue,
                       glyphAdvances.data(), glyphAdvances.size(), 1.0, NULL, caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);

        /* Test with caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFTrue,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with single breakable code units and glyph mapping in backward LTR mode. */
    {
        vector<SFUInteger> clusterMap = { 4, 3, 2, 1, 0 };
        vector<SFInt32> glyphAdvances = { 50, 40, 30, 20, 10 };
        vector<SFBoolean> caretStops = { SFTrue, SFTrue, SFTrue, SFTrue, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 0, 10, 30, 60, 100, 150 };

        /* Test without caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFTrue, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(), 1.0, NULL, caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);

        /* Test with caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFTrue, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with single breakable code units and glyph mapping in backward RTL mode. */
    {
        vector<SFUInteger> clusterMap = { 4, 3, 2, 1, 0 };
        vector<SFInt32> glyphAdvances = { 50, 40, 30, 20, 10 };
        vector<SFBoolean> caretStops = { SFTrue, SFTrue, SFTrue, SFTrue, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 150, 140, 120, 90, 50, 0 };

        /* Test without caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFTrue, SFTrue,
                       glyphAdvances.data(), glyphAdvances.size(), 1.0, NULL, caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);

        /* Test with caret stops. */
        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFTrue, SFTrue,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with each glyph break but fewer code unit breaks. */
    {
        vector<SFUInteger> clusterMap = { 0, 1, 2, 3, 4 };
        vector<SFInt32> glyphAdvances = { 10, 20, 30, 40, 50 };
        vector<SFBoolean> caretStops = { SFTrue, SFFalse, SFTrue, SFFalse, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 0, 10, 10, 60, 60, 150 };

        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with each code unit break but having fewer glyphs. */
    {
        vector<SFUInteger> clusterMap = { 0, 0, 1, 1, 2 };
        vector<SFInt32> glyphAdvances = { 10, 20, 30 };
        vector<SFBoolean> caretStops = { SFTrue, SFTrue, SFTrue, SFTrue, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 0, 5, 10, 20, 30, 60 };

        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }

    /* Test with unadjusted code unit and glyph breaks. */
    {
        vector<SFUInteger> clusterMap = { 0, 0, 1, 1, 2 };
        vector<SFInt32> glyphAdvances = { 10, 20, 30 };
        vector<SFBoolean> caretStops = { SFTrue, SFFalse, SFTrue, SFFalse, SFTrue };
        SFFloat caretEdges[clusterMap.size() + 1];

        const SFFloat expected[] = { 0, 20, 20, 40, 40, 60 };

        LoadCaretEdges(clusterMap.data(), clusterMap.size(), SFFalse, SFFalse,
                       glyphAdvances.data(), glyphAdvances.size(),
                       1.0, caretStops.data(), caretEdges);
        assert(memcmp(caretEdges, expected, sizeof(expected)) == 0);
    }
}

void AlbumTester::test()
{
    testInitialize();
    testReset();
    testAddGlyph();
    testReserveGlyphs();
    testSetGlyph();
    testGetGlyph();
    testSetAssociation();
    testGetAssociation();
    testFeatureMask();
    testTraits();
    testOffset();
    testAdvance();
    testCursiveOffset();
    testAttachmentOffset();
    testCaretEdges();
}
