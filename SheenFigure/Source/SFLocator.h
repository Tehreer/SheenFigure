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

#ifndef _SF_LOCATOR_INTERNAL_H
#define _SF_LOCATOR_INTERNAL_H

#include <SFConfig.h>

#include "SFAlbum.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGlyphTraits.h"

typedef struct _SFLocator {
    SFAlbumRef _album;
    SFData _markAttachClassDef;
    SFData _markGlyphSetsDef;
#ifdef SF_SAFE_ALBUM
    SFUInteger _version;
#endif
    SFUInteger _startIndex;
    SFUInteger _limitIndex;
    SFUInteger _stateIndex;
    SFUInteger index;
    SFGlyphTraits _requiredTraits;
    SFLookupFlag lookupFlag;
} SFLocator, *SFLocatorRef;

SF_INTERNAL void SFLocatorInitialize(SFLocatorRef locator, SFAlbumRef album, SFData gdef);

SF_INTERNAL void SFLocatorSetRequiredTraits(SFLocatorRef locator, SFGlyphTraits requiredTraits);

SF_INTERNAL void SFLocatorSetLookupFlag(SFLocatorRef locator, SFLookupFlag lookupFlag);

SF_INTERNAL void SFLocatorReset(SFLocatorRef locator, SFUInteger index, SFUInteger count);

SF_INTERNAL void SFLocatorReserveGlyphs(SFLocatorRef locator, SFUInteger glyphCount);

/**
 * Advances the locator to next glyph.
 */
SF_INTERNAL SFBoolean SFLocatorMoveNext(SFLocatorRef locator);

/**
 * Skips the given number of glyphs excluding the ignored ones.
 */
SF_INTERNAL SFBoolean SFLocatorSkip(SFLocatorRef locator, SFUInteger count);

/**
 * Jumps the locator to given index in such a way that next call to MoveNext starts looking from
 * this index.
 */
SF_INTERNAL void SFLocatorJumpTo(SFLocatorRef locator, SFUInteger index);

/**
 * Gets the index of appropriate glyph after the given index.
 * @return
 *      The index of next glyph, or SFInvalidIndex if there is no appropriate glyph after the given
 *      index.
 */
SF_INTERNAL SFUInteger SFLocatorGetAfter(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag);

/**
 * Gets the index of appropriate glyph before the given index.
 * @return
 *      The index of previous glyph, or SFInvalidIndex if there is no appropriate glyph after the
 *      given index.
 */
SF_INTERNAL SFUInteger SFLocatorGetBefore(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag);

#endif
