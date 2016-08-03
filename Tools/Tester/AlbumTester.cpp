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

#include <cstddef>
#include <cstdlib>

#include "AlbumTester.h"

extern "C" {
#include <SheenBidi/SBCodepointSequence.h>
#include <SheenFigure/Source/SFAlbum.h>
#include <SheenFigure/Source/SFAssert.h>
}

using namespace SheenFigure::Tester;

static const SFCodepoint CODEPOINT_ARRAY[] = { 'S', 'H', 'E', 'E', 'N' };
static const SFUInteger CODEPOINT_COUNT = sizeof(CODEPOINT_ARRAY) / sizeof(SFCodepoint);

static const SBCodepointSequence CODEPOINT_SEQUENCE = {
    SBStringEncodingUTF32,
    (void *)CODEPOINT_ARRAY,
    CODEPOINT_COUNT
};

static void SFAlbumReserveGlyphsInitialized(SFAlbumRef album, SFUInteger index, SFUInteger count)
{
    SFUInteger maxIndex = index + count;
    SFUInteger i;

    SFAlbumReserveGlyphs(album, index, count);

    for (i = index; i < maxIndex; i++) {
        SFAlbumSetGlyph(album, i, 0);
        SFAlbumSetFeatureMask(album, i, 0);
        SFAlbumSetTraits(album, i, SFGlyphTraitNone);
        SFAlbumSetSingleAssociation(album, i, 0);
    }
}

AlbumTester::AlbumTester()
{
    SFCodepointsInitialize(&m_codepoints, &CODEPOINT_SEQUENCE, SFFalse);
}

void AlbumTester::testInitialize()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    SFAssert(album.codepoints == NULL);
    SFAssert(album.stringLength == 0);
    SFAssert(album.glyphCount == 0);

    SFAlbumFinalize(&album);
}

void AlbumTester::testReset()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test reset just after initialization. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

        SFAssert(album.codepoints == &m_codepoints);
        SFAssert(album.stringLength == CODEPOINT_COUNT);
        SFAssert(album.glyphCount == 0);
    }

    /* Test reset after some manipulation. */
    {
        SFAlbumBeginFilling(&album);
        SFAlbumReserveGlyphsInitialized(&album, 0, 5);
        SFAlbumEndFilling(&album);

        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

        SFAssert(album.codepoints == &m_codepoints);
        SFAssert(album.stringLength == CODEPOINT_COUNT);
        SFAssert(album.glyphCount == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testAddGlyph()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test with forward associations. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
        SFAlbumBeginFilling(&album);

        SFAlbumAddGlyph(&album, 100, 0, 1);
        SFAlbumAddGlyph(&album, 200, 1, 1);
        SFAlbumAddGlyph(&album, 300, 2, 1);
        SFAlbumAddGlyph(&album, 400, 3, 1);
        SFAlbumAddGlyph(&album, 500, 4, 1);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(glyphs[0] == 100);
        SFAssert(glyphs[1] == 200);
        SFAssert(glyphs[2] == 300);
        SFAssert(glyphs[3] == 400);
        SFAssert(glyphs[4] == 500);

        /* Test the output map. */
        const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        SFAssert(map[0] == 0);
        SFAssert(map[1] == 1);
        SFAssert(map[2] == 2);
        SFAssert(map[3] == 3);
        SFAssert(map[4] == 4);
    }

    /* Test with backward associations. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
        SFAlbumBeginFilling(&album);

        SFAlbumAddGlyph(&album, 100, 4, 1);
        SFAlbumAddGlyph(&album, 200, 3, 1);
        SFAlbumAddGlyph(&album, 300, 2, 1);
        SFAlbumAddGlyph(&album, 400, 1, 1);
        SFAlbumAddGlyph(&album, 500, 0, 1);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the glyph count. */
        SFAssert(SFAlbumGetGlyphCount(&album) == 5);

        /* Test the output glyphs. */
        const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
        SFAssert(glyphs[0] == 100);
        SFAssert(glyphs[1] == 200);
        SFAssert(glyphs[2] == 300);
        SFAssert(glyphs[3] == 400);
        SFAssert(glyphs[4] == 500);

        /* Test the output map. */
        const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        SFAssert(map[0] == 4);
        SFAssert(map[1] == 3);
        SFAssert(map[2] == 2);
        SFAssert(map[3] == 1);
        SFAssert(map[4] == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testReserveGlyphs()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
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
        SFAlbumSetTraits(&album, i, SFGlyphTraitNone);
        SFAlbumSetSingleAssociation(&album, i, 0);
    }
    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);

    /* Test the glyph count. */
    SFAssert(SFAlbumGetGlyphCount(&album) == 25);

    /* Test the expected locations of output glyphs. */
    const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
    SFAssert(glyphs[5] == 100);
    SFAssert(glyphs[9] == 300);
    SFAssert(glyphs[15] == 400);
    SFAssert(glyphs[19] == 200);
    SFAssert(glyphs[24] == 500);

    SFAlbumFinalize(&album);
}

void AlbumTester::testSetGlyph()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
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
    SFAssert(SFAlbumGetGlyphCount(&album) == 5);

    /* Test the output glyphs. */
    const SFGlyphID *glyphs = SFAlbumGetGlyphIDsPtr(&album);
    SFAssert(glyphs[0] == 100);
    SFAssert(glyphs[1] == 200);
    SFAssert(glyphs[2] == 300);
    SFAssert(glyphs[3] == 400);
    SFAssert(glyphs[4] == 500);

    SFAlbumFinalize(&album);
}

void AlbumTester::testGetGlyph()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
    SFAlbumBeginFilling(&album);

    /* Test by adding some glyphs. */
    {
        SFAlbumAddGlyph(&album, 100, 0, 1);
        SFAlbumAddGlyph(&album, 200, 0, 1);

        /* Test with get glyph. */
        SFAssert(SFAlbumGetGlyph(&album, 0) == 100);
        SFAssert(SFAlbumGetGlyph(&album, 1) == 200);
    }

    /* Test by reserving and setting some glyphs. */
    {
        SFAlbumReserveGlyphsInitialized(&album, 1, 3);
        SFAlbumSetGlyph(&album, 1, 300);
        SFAlbumSetGlyph(&album, 2, 400);
        SFAlbumSetGlyph(&album, 3, 500);

        /* Test with get glyph. */
        SFAssert(SFAlbumGetGlyph(&album, 1) == 300);
        SFAssert(SFAlbumGetGlyph(&album, 2) == 400);
        SFAssert(SFAlbumGetGlyph(&album, 3) == 500);
    }

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testSetSingleAssociation()
{
    SFAlbum album;
    SFAlbumInitialize(&album);

    /* Test with forward associations. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 5);

        SFAlbumSetSingleAssociation(&album, 0, 0);
        SFAlbumSetSingleAssociation(&album, 1, 1);
        SFAlbumSetSingleAssociation(&album, 2, 2);
        SFAlbumSetSingleAssociation(&album, 3, 3);
        SFAlbumSetSingleAssociation(&album, 4, 4);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        SFAssert(map[0] == 0);
        SFAssert(map[1] == 1);
        SFAssert(map[2] == 2);
        SFAssert(map[3] == 3);
        SFAssert(map[4] == 4);
    }

    /* Test with backward associations. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 5);

        SFAlbumSetSingleAssociation(&album, 0, 4);
        SFAlbumSetSingleAssociation(&album, 1, 3);
        SFAlbumSetSingleAssociation(&album, 2, 2);
        SFAlbumSetSingleAssociation(&album, 3, 1);
        SFAlbumSetSingleAssociation(&album, 4, 0);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        SFAssert(map[0] == 4);
        SFAssert(map[1] == 3);
        SFAssert(map[2] == 2);
        SFAssert(map[3] == 1);
        SFAssert(map[4] == 0);
    }

    /* Test with complex associations. */
    {
        SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
        SFAlbumBeginFilling(&album);

        SFAlbumReserveGlyphsInitialized(&album, 0, 8);

        SFAlbumSetSingleAssociation(&album, 0, 4);
        SFAlbumSetSingleAssociation(&album, 1, 4);
        SFAlbumSetSingleAssociation(&album, 2, 4);
        SFAlbumSetSingleAssociation(&album, 3, 3);
        SFAlbumSetSingleAssociation(&album, 4, 3);
        SFAlbumSetSingleAssociation(&album, 5, 2);
        SFAlbumSetSingleAssociation(&album, 6, 1);
        SFAlbumSetSingleAssociation(&album, 7, 0);

        SFAlbumEndFilling(&album);
        SFAlbumWrapUp(&album);

        /* Test the output map. */
        const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
        SFAssert(map[0] == 7);
        SFAssert(map[1] == 6);
        SFAssert(map[2] == 5);
        SFAssert(map[3] == 3);
        SFAssert(map[4] == 0);
    }

    SFAlbumFinalize(&album);
}

void AlbumTester::testGetSingleAssociation()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
    SFAlbumBeginFilling(&album);

    /* Test by adding some glyphs. */
    {
        SFAlbumAddGlyph(&album, 0, 0, 1);
        SFAlbumAddGlyph(&album, 0, 1, 1);

        /* Test with get single association. */
        SFAssert(SFAlbumGetSingleAssociation(&album, 0) == 0);
        SFAssert(SFAlbumGetSingleAssociation(&album, 1) == 1);
    }

    /* Test by reserving some glyphs. */
    {
        SFAlbumReserveGlyphsInitialized(&album, 1, 3);
        SFAlbumSetSingleAssociation(&album, 1, 2);
        SFAlbumSetSingleAssociation(&album, 2, 3);
        SFAlbumSetSingleAssociation(&album, 3, 4);

        /* Test with get single association. */
        SFAssert(SFAlbumGetSingleAssociation(&album, 1) == 2);
        SFAssert(SFAlbumGetSingleAssociation(&album, 2) == 3);
        SFAssert(SFAlbumGetSingleAssociation(&album, 3) == 4);
    }

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testMakeCompositeAssociations()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
    SFAlbumBeginFilling(&album);

    /* Add some glyphs with associations. */
    SFAlbumAddGlyph(&album, 0, 1, 1);
    SFAlbumAddGlyph(&album, 0, 2, 1);
    SFAlbumAddGlyph(&album, 0, 3, 1);

    /* Make the second glyph composite. */
    SFAlbumSetTraits(&album, 1, SFGlyphTraitComposite);

    /* Set the composite associations. */
    SFUInteger *associations = SFAlbumMakeCompositeAssociations(&album, 1, 3);
    associations[0] = 0;
    associations[1] = 2;
    associations[2] = 4;

    SFAlbumEndFilling(&album);
    SFAlbumWrapUp(&album);

    /* Test the output map. */
    const SFUInteger *map = SFAlbumGetCodeunitToGlyphMapPtr(&album);
    SFAssert(map[0] == 1);
    SFAssert(map[1] == 0);
    SFAssert(map[2] == 1);
    SFAssert(map[3] == 2);
    SFAssert(map[4] == 1);

    SFAlbumFinalize(&album);
}

void AlbumTester::testGetCompositeAssociations()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);
    SFAlbumBeginFilling(&album);

    /* Add some composite glyphs. */
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);
    for (SFUInteger i = 0; i < 5; i++) {
        SFAlbumSetTraits(&album, i, SFGlyphTraitComposite);
    }

    /* Make some composite associations. */
    {
        SFUInteger *ca1 = SFAlbumMakeCompositeAssociations(&album, 1, 2);
        for (SFUInteger i = 0; i < 2; i++) {
            ca1[i] = i;
        }

        SFUInteger *ca2 = SFAlbumMakeCompositeAssociations(&album, 2, 3);
        for (SFUInteger i = 0; i < 3; i++) {
            ca2[i] = i;
        }

        SFUInteger *ca3 = SFAlbumMakeCompositeAssociations(&album, 3, 4);
        for (SFUInteger i = 0; i < 4; i++) {
            ca3[i] = i;
        }

        SFUInteger *ca4 = SFAlbumMakeCompositeAssociations(&album, 4, 5);
        for (SFUInteger i = 0; i < 5; i++) {
            ca4[i] = i;
        }
    }

    /* Test the associations. */
    {
        SFUInteger count = 0;

        SFUInteger *ca4 = SFAlbumGetCompositeAssociations(&album, 4, &count);
        SFAssert(count == 5);
        for (SFUInteger i = 0; i < count; i++) {
            SFAssert(ca4[i] == i);
        }

        SFUInteger *ca3 = SFAlbumGetCompositeAssociations(&album, 3, &count);
        SFAssert(count == 4);
        for (SFUInteger i = 0; i < count; i++) {
            SFAssert(ca3[i] == i);
        }

        SFUInteger *ca2 = SFAlbumGetCompositeAssociations(&album, 2, &count);
        SFAssert(count == 3);
        for (SFUInteger i = 0; i < count; i++) {
            SFAssert(ca2[i] == i);
        }

        SFUInteger *ca1 = SFAlbumGetCompositeAssociations(&album, 1, &count);
        SFAssert(count == 2);
        for (SFUInteger i = 0; i < count; i++) {
            SFAssert(ca1[i] == i);
        }
    }

    SFAlbumEndFilling(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testFeatureMask()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

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
    SFAssert(SFAlbumGetFeatureMask(&album, 0) == mask1);
    SFAssert(SFAlbumGetFeatureMask(&album, 1) == mask2);
    SFAssert(SFAlbumGetFeatureMask(&album, 2) == mask3);
    SFAssert(SFAlbumGetFeatureMask(&album, 3) == mask4);
    SFAssert(SFAlbumGetFeatureMask(&album, 4) == mask5);

    SFAlbumEndFilling(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testTraits()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

    SFAlbumBeginFilling(&album);
    SFAlbumReserveGlyphsInitialized(&album, 0, 5);

    /* Test by setting traits. */
    {
        SFAlbumSetTraits(&album, 0, SFGlyphTraitNone);
        SFAlbumSetTraits(&album, 1, SFGlyphTraitBase);
        SFAlbumSetTraits(&album, 2, SFGlyphTraitLigature);
        SFAlbumSetTraits(&album, 3, SFGlyphTraitMark);
        SFAlbumSetTraits(&album, 4, SFGlyphTraitComponent);

        SFAssert(SFAlbumGetTraits(&album, 0) == SFGlyphTraitNone);
        SFAssert(SFAlbumGetTraits(&album, 1) == SFGlyphTraitBase);
        SFAssert(SFAlbumGetTraits(&album, 2) == SFGlyphTraitLigature);
        SFAssert(SFAlbumGetTraits(&album, 3) == SFGlyphTraitMark);
        SFAssert(SFAlbumGetTraits(&album, 4) == SFGlyphTraitComponent);
    }

    /* Test by inserting traits. */
    {
        SFAlbumInsertTraits(&album, 0, SFGlyphTraitComposite | SFGlyphTraitPlaceholder);
        SFAlbumInsertTraits(&album, 1, SFGlyphTraitAttached | SFGlyphTraitResolved);
        SFAlbumInsertTraits(&album, 2, SFGlyphTraitCursive);
        SFAlbumInsertTraits(&album, 3, SFGlyphTraitCursive | SFGlyphTraitRightToLeft);
        SFAlbumInsertTraits(&album, 4, SFGlyphTraitResolved);

        SFAssert(SFAlbumGetTraits(&album, 0) == (SFGlyphTraitComposite | SFGlyphTraitPlaceholder));
        SFAssert(SFAlbumGetTraits(&album, 1) == (SFGlyphTraitBase | SFGlyphTraitAttached | SFGlyphTraitResolved));
        SFAssert(SFAlbumGetTraits(&album, 2) == (SFGlyphTraitLigature | SFGlyphTraitCursive));
        SFAssert(SFAlbumGetTraits(&album, 3) == (SFGlyphTraitMark | SFGlyphTraitCursive | SFGlyphTraitRightToLeft));
        SFAssert(SFAlbumGetTraits(&album, 4) == (SFGlyphTraitComponent | SFGlyphTraitResolved));
    }

    /* Test by removing traits. */
    {
        SFAlbumRemoveTraits(&album, 0, SFGlyphTraitPlaceholder);
        SFAlbumRemoveTraits(&album, 1, SFGlyphTraitBase | SFGlyphTraitResolved);
        SFAlbumRemoveTraits(&album, 2, SFGlyphTraitLigature | SFGlyphTraitCursive);
        SFAlbumRemoveTraits(&album, 3, SFGlyphTraitRightToLeft);
        SFAlbumRemoveTraits(&album, 4, SFGlyphTraitComponent);

        SFAssert(SFAlbumGetTraits(&album, 0) == (SFGlyphTraitComposite));
        SFAssert(SFAlbumGetTraits(&album, 1) == (SFGlyphTraitAttached));
        SFAssert(SFAlbumGetTraits(&album, 2) == (SFGlyphTraitNone));
        SFAssert(SFAlbumGetTraits(&album, 3) == (SFGlyphTraitMark | SFGlyphTraitCursive));
        SFAssert(SFAlbumGetTraits(&album, 4) == (SFGlyphTraitResolved));
    }

    SFAlbumEndFilling(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testPosition()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

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
    SFAssert(SFAlbumGetX(&album, 0) == 100 && SFAlbumGetY(&album, 0) == 150);
    SFAssert(SFAlbumGetX(&album, 1) == 200 && SFAlbumGetY(&album, 1) == 250);
    SFAssert(SFAlbumGetX(&album, 2) == 300 && SFAlbumGetY(&album, 2) == 350);
    SFAssert(SFAlbumGetX(&album, 3) == 400 && SFAlbumGetY(&album, 3) == 450);
    SFAssert(SFAlbumGetX(&album, 4) == 500 && SFAlbumGetY(&album, 4) == 550);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);

    /* Test the overall output. */
    const SFPoint *offsets = SFAlbumGetGlyphOffsetsPtr(&album);
    SFAssert(offsets[0].x == 100 && offsets[0].y == 150);
    SFAssert(offsets[1].x == 200 && offsets[1].y == 250);
    SFAssert(offsets[2].x == 300 && offsets[2].y == 350);
    SFAssert(offsets[3].x == 400 && offsets[3].y == 450);
    SFAssert(offsets[4].x == 500 && offsets[4].y == 550);

    SFAlbumFinalize(&album);
}

void AlbumTester::testAdvance()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

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
    SFAssert(SFAlbumGetAdvance(&album, 0) == 100);
    SFAssert(SFAlbumGetAdvance(&album, 1) == 200);
    SFAssert(SFAlbumGetAdvance(&album, 2) == 300);
    SFAssert(SFAlbumGetAdvance(&album, 3) == 400);
    SFAssert(SFAlbumGetAdvance(&album, 4) == 500);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);

    /* Test the overall output. */
    const SFAdvance *advances = SFAlbumGetGlyphAdvancesPtr(&album);
    SFAssert(advances[0] == 100);
    SFAssert(advances[1] == 200);
    SFAssert(advances[2] == 300);
    SFAssert(advances[3] == 400);
    SFAssert(advances[4] == 500);

    SFAlbumFinalize(&album);
}

void AlbumTester::testCursiveOffset()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

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
    SFAssert(SFAlbumGetCursiveOffset(&album, 0) == 1);
    SFAssert(SFAlbumGetCursiveOffset(&album, 1) == 2);
    SFAssert(SFAlbumGetCursiveOffset(&album, 2) == 3);
    SFAssert(SFAlbumGetCursiveOffset(&album, 3) == 4);
    SFAssert(SFAlbumGetCursiveOffset(&album, 4) == 5);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::testAttachmentOffset()
{
    SFAlbum album;
    SFAlbumInitialize(&album);
    SFAlbumReset(&album, &m_codepoints, CODEPOINT_COUNT);

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
    SFAssert(SFAlbumGetAttachmentOffset(&album, 0) == 1);
    SFAssert(SFAlbumGetAttachmentOffset(&album, 1) == 2);
    SFAssert(SFAlbumGetAttachmentOffset(&album, 2) == 3);
    SFAssert(SFAlbumGetAttachmentOffset(&album, 3) == 4);
    SFAssert(SFAlbumGetAttachmentOffset(&album, 4) == 5);

    SFAlbumEndArranging(&album);
    SFAlbumWrapUp(&album);
    SFAlbumFinalize(&album);
}

void AlbumTester::test()
{
    testInitialize();
    testReset();
    testAddGlyph();
    testReserveGlyphs();
    testSetGlyph();
    testGetGlyph();
    testSetSingleAssociation();
    testGetSingleAssociation();
    testMakeCompositeAssociations();
    testGetCompositeAssociations();
    testFeatureMask();
    testTraits();
    testPosition();
    testAdvance();
    testCursiveOffset();
    testAttachmentOffset();
}
