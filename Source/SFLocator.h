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

#ifndef SF_INTERNAL__LOCATOR_H
#define SF_INTERNAL__LOCATOR_H

#include <SFConfig.h>

#include "SFCommon.h"
#include "SFData.h"
#include "SFCollection.h"

typedef struct {
    SFCollectionRef _collection;
    SFData _markAttachClassDef;
    SFData _markGlyphSetsDef;
    SFUInteger index;
    SFUInteger _iterIndex;
    SFLookupFlag _lookupFlag;
} SFLocator;
typedef SFLocator *SFLocatorRef;

SF_INTERNAL void SFLocatorInitialize(SFLocatorRef locator, SFCollectionRef collection, SFData gdef);

SF_INTERNAL void SFLocatorReset(SFLocatorRef locator, SFLookupFlag lookupFlag);

/**
 * Advances the locator to next glyph.
 */
SF_INTERNAL SFBoolean SFLocatorMoveNext(SFLocatorRef locator);

/**
 * Skips the given number of glyphs.
 */
SF_INTERNAL void SFLocatorSkip(SFLocatorRef locator, SFUInteger glyphCount);

/**
 * Gets the index of appropriate glyph after the given index.
 * @return
 *      The index of next glyph, or SFInvalidIndex if there is no appropriate
 *      glyph after the given index.
 */
SF_INTERNAL SFUInteger SFLocatorGetAfter(SFLocatorRef locator, SFUInteger index);

/**
 * Gets the index of appropriate glyph before the given index.
 * @return
 *      The index of previous glyph, or SFInvalidIndex if there is no
 *      appropriate glyph after the given index.
 */
SF_INTERNAL SFUInteger SFLocatorGetBefore(SFLocatorRef locator, SFUInteger index, SFLookupFlag lookupFlag);

#endif
