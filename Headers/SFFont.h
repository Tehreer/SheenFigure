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

#ifndef SF_FONT_H
#define SF_FONT_H

#include "SFConfig.h"
#include "SFScriptTag.h"
#include "SFTypes.h"

#include <ft2build.h>
#include FT_FREETYPE_H

struct _SFFont;
typedef struct _SFFont SFFont;
/**
 * The type used to represent a font.
 */
typedef SFFont *SFFontRef;

SFFontRef SFFontCreateWithFTFace(FT_Face ftFace);

SFInteger SFFontGetUnitsPerEm(SFFontRef font);
SFInteger SFFontGetAscender(SFFontRef font);
SFInteger SFFontGetDescender(SFFontRef font);
SFInteger SFFontGetLeading(SFFontRef font);

SFGlyphID SFFontGetGlyphForCodePoint(SFFontRef font, SFCodePoint codePoint);
SFInteger SFFontGetGlyphAdvance(SFFontRef font, SFGlyphID glyph);

SFFontRef SFFontRetain(SFFontRef font);
void SFFontRelease(SFFontRef font);

#endif
