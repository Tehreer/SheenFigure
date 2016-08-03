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

#ifndef __SHEENFIGURE_TESTER__ALBUM_TESTER_H
#define __SHEENFIGURE_TESTER__ALBUM_TESTER_H

namespace SheenFigure {
namespace Tester {

class AlbumTester {
public:
    AlbumTester();

    void testInitialize();
    void testReset();
    void testAddGlyph();
    void testReserveGlyphs();
    void testSetGlyph();
    void testGetGlyph();
    void testSetSingleAssociation();
    void testGetSingleAssociation();
    void testMakeCompositeAssociations();
    void testGetCompositeAssociations();
    void testFeatureMask();
    void testTraits();
    void testOffset();
    void testAdvance();
    void testCursiveOffset();
    void testAttachmentOffset();

    void test();
};

}
}

#endif
