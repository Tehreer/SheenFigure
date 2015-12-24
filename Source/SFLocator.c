/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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
#include <SFTypes.h>

#include <stddef.h>

#include "SFAssert.h"
#include "SFAlbum.h"
#include "SFGDEF.h"
#include "SFOpenType.h"
#include "SFLocator.h"

static SFBoolean _SFIsIgnoredGlyph(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag);

SF_INTERNAL void SFLocatorInitialize(SFLocatorRef locator, SFAlbumRef album, SFData gdef)
{
    /* Collection must NOT be null. */
    SFAssert(album != NULL);

    locator->_album = album;
    locator->_markAttachClassDef = NULL;
    locator->_markGlyphSetsDef = NULL;
    locator->index = SFInvalidIndex;
    locator->_state = 0;
    locator->lookupFlag = 0;

    if (gdef) {
        SFOffset offset = SF_GDEF__MARK_ATTACH_CLASS_DEF_OFFSET(gdef);
        locator->_markAttachClassDef = SF_DATA__SUBDATA(gdef, offset);

        if (SF_GDEF__VERSION(gdef) == 0x00010002) {
            offset = SF_GDEF__MARK_GLYPH_SETS_DEF_OFFSET(gdef);
            locator->_markGlyphSetsDef = SF_DATA__SUBDATA(gdef, offset);
        }
    }
}

SF_INTERNAL void SFLocatorReset(SFLocatorRef locator)
{
    locator->index = SFInvalidIndex;
    locator->_state = 0;
}

SF_INTERNAL void SFLocatorSetLookupFlag(SFLocatorRef locator, SFLookupFlag lookupFlag)
{
    locator->lookupFlag = lookupFlag;
}

static SFBoolean _SFIsIgnoredGlyph(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag) {
    SFAlbumRef album = locator->_album;
    SFGlyphTrait traits = SFAlbumGetTraits(album, index);
    SFBoolean isMark;

    if (traits & SFGlyphTraitRemoved) {
        return SFTrue;
    }

    isMark = (traits & SFGlyphTraitMark);
    if ((lookupFlag & SFLookupFlagIgnoreMarks) && isMark) {
        return SFTrue;
    }

    if ((lookupFlag & SFLookupFlagIgnoreLigatures) && (traits & SFGlyphTraitLigature)) {
        return SFTrue;
    }

    if ((lookupFlag & SFLookupFlagIgnoreBaseGlyphs) && (traits & SFGlyphTraitBase)) {
        return SFTrue;
    }

    if (lookupFlag & SFLookupFlagMarkAttachmentType) {
        SFGlyphID glyph = SFAlbumGetGlyph(album, index);
        SFUInt16 glyphClass;

        if (locator->_markAttachClassDef && isMark
            && !(SFOpenTypeSearchGlyphClass(locator->_markAttachClassDef, glyph, &glyphClass)
                 && glyphClass == (lookupFlag >> 8))) {
            return SFTrue;
        }
    }

    return SFFalse;
}

SF_INTERNAL SFBoolean SFLocatorMoveNext(SFLocatorRef locator)
{
    SFAlbumRef album = locator->_album;

    /* The state of locator must be valid. */
    SFAssert(locator->_state < album->elementCount);

    do {
        SFUInteger index = locator->_state++;

        if (!_SFIsIgnoredGlyph(locator, index, locator->lookupFlag)) {
            locator->index = index;
            return SFTrue;
        }
    } while (locator->_state < album->elementCount);

    return SFFalse;
}

SF_INTERNAL void SFLocatorJumpTo(SFLocatorRef locator, SFUInteger index)
{
    locator->_state = index;
}

SF_INTERNAL SFUInteger SFLocatorGetAfter(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag)
{
    SFAlbumRef album = locator->_album;

    /* The index must be valid. */
    SFAssert(index < album->elementCount);

    for (index += 1; index < album->elementCount; index++) {
        if (!_SFIsIgnoredGlyph(locator, index, lookupFlag)) {
            return index;
        }
    }

    return SFInvalidIndex;
}

SF_INTERNAL SFUInteger SFLocatorGetBefore(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag)
{
    SFAlbumRef album = locator->_album;

    /* The index must be valid. */
    SFAssert(index < album->elementCount);

    while (index-- > 0) {
        if (!_SFIsIgnoredGlyph(locator, index, lookupFlag)) {
            return index;
        }
    }

    return SFInvalidIndex;
}
