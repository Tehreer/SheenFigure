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

#ifndef _SF_INTERNAL_LOCATOR_H
#define _SF_INTERNAL_LOCATOR_H

#include <SFConfig.h>

#include "SFAlbum.h"
#include "Common.h"
#include "Data.h"

typedef struct _LocatorFilter {
    Data markFilteringCoverage;
    GlyphMask ignoreMask;
    LookupFlag lookupFlag;
} LocatorFilter, *LocatorFilterRef;

typedef struct _Locator {
    SFAlbumRef _album;
    Data _markAttachClassDef;
    Data _markGlyphSetsDef;
    LocatorFilter filter;
    SFUInteger version;
    SFRange range;
    SFUInteger comingIndex;
    SFUInteger index;
} Locator, *LocatorRef;

SF_INTERNAL void LocatorInitialize(LocatorRef locator, SFAlbumRef album, Data gdef);

SF_INTERNAL void LocatorSetFeatureMask(LocatorRef locator, SFUInt16 featureMask);

/**
 * Sets the lookup flag describing the criterion for ignoring glyphs.
 */
SF_INTERNAL void LocatorSetLookupFlag(LocatorRef locator, LookupFlag lookupFlag);

/**
 * Sets the mark filtering set to use, if needed.
 */
SF_INTERNAL void LocatorSetMarkFilteringSet(LocatorRef locator, SFUInt16 markFilteringSet);

SF_INTERNAL void LocatorUpdateFilter(LocatorRef locator, LocatorFilterRef filter);

SF_INTERNAL void LocatorAdjustRange(LocatorRef locator, SFUInteger start, SFUInteger count);

SF_INTERNAL void LocatorReset(LocatorRef locator, SFUInteger start, SFUInteger count);

SF_INTERNAL void LocatorReserveGlyphs(LocatorRef locator, SFUInteger glyphCount);

/**
 * Advances the locator to next glyph within the contextual boundary.
 * @return
 *      SFTrue if the locator was successfully advanced to the next glyph; SFFalse if the locator
 *      has passed the end of the album.
 */
SF_INTERNAL SFBoolean LocatorMoveNext(LocatorRef locator);

/**
 * Advances the locator to previous glyph within the contextual boundary.
 * @return
 *      SFTrue if the locator was successfully advanced to the previous glyph; SFFalse if the
 *      locator has passed the start of the album.
 */
SF_INTERNAL SFBoolean LocatorMovePrevious(LocatorRef locator);

/**
 * Skips the given number of legitimate glyphs.
 * @return
 *      SFTrue if the given number of glyphs were successfully skipped; SFFalse if the locator could
 *      not find enough legitimate glyphs to skip.
 */
SF_INTERNAL SFBoolean LocatorSkip(LocatorRef locator, SFUInteger count);

/**
 * Jumps the locator to given index in such a way that next call to MoveNext starts looking
 * legitimate glyphs from this index.
 */
SF_INTERNAL void LocatorJumpTo(LocatorRef locator, SFUInteger index);

/**
 * Determines the index of next legitimate glyph after the specified index.
 *
 * @param bounded
 *      Enables/Disables glyph searching within the contextual boundary.
 * @return
 *      The index of next legitimate glyph if available, or SFInvalidIndex if there was no
 *      legitimate glyph after the specified index.
 */
SF_INTERNAL SFUInteger LocatorGetAfter(LocatorRef locator, SFUInteger index, SFBoolean bounded);

/**
 * Determines the index of previous legitimate glyph before the specified index.
 *
 * @param bounded
 *      Enables/Disables glyph searching within the contextual boundary.
 * @return
 *      The index of previous legitimate glyph if available, or SFInvalidIndex if there was no
 *      legitimate glyph after the specified index.
 */
SF_INTERNAL SFUInteger LocatorGetBefore(LocatorRef locator, SFUInteger index, SFBoolean bounded);

/**
 * Returns the index of appropriate preceding base glyph.
 * @return
 *      The index of preceding base glyph if available, or SFInvalidIndex if there was no
 *      appropriate base glyph.
 */
SF_INTERNAL SFUInteger LocatorGetPrecedingBaseIndex(LocatorRef locator);

/**
 * Returns the index of appropriate preceding ligature glyph.
 * @return
 *      The index of preceding ligature glyph if available, or SFInvalidIndex if there was no
 *      appropriate ligature glyph.
 */
SF_INTERNAL SFUInteger LocatorGetPrecedingLigatureIndex(LocatorRef locator, SFUInteger *outComponent);

/**
 * Returns the index of appropriate preceding mark glyph.
 * @return
 *      The index of preceding mark glyph if available, or SFInvalidIndex if there was no
 *      appropriate mark glyph.
 */
SF_INTERNAL SFUInteger LocatorGetPrecedingMarkIndex(LocatorRef locator);

#endif
