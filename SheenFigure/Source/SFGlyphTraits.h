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

#ifndef _SF_GLYPH_TRAITS_INTERNAL_H
#define _SF_GLYPH_TRAITS_INTERNAL_H

#include <SFTypes.h>

enum {
    SFGlyphTraitNone        = 0 << 0,
    SFGlyphTraitRightToLeft = 1 << 0,
    SFGlyphTraitBase        = 1 << 1,
    SFGlyphTraitLigature    = 1 << 2,
    SFGlyphTraitMark        = 1 << 3,
    SFGlyphTraitComponent   = 1 << 4,
    SFGlyphTraitRemoved     = 1 << 5,

    SFGlyphTraitRegular     = SFGlyphTraitRightToLeft
                            | SFGlyphTraitBase
                            | SFGlyphTraitLigature
                            | SFGlyphTraitMark
                            | SFGlyphTraitComponent
                            | SFGlyphTraitRemoved,
    SFGlyphTraitSpecial     = ~SFGlyphTraitRegular
};
typedef SFUInt32 SFGlyphTraits;

#define SFGlyphTraitMakeSpecial(number)     \
(                                           \
   (SFGlyphTraitRegular << ((number) + 1))  \
 & (SFGlyphTraitSpecial << ((number) + 0))  \
)

#endif
