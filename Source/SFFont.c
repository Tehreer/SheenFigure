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

#include <ft2build.h>
#include FT_ADVANCES_H
#include FT_FREETYPE_H
#include FT_TYPES_H

#include <stddef.h>
#include <stdlib.h>

#include "SFData.h"
#include "SFFont.h"

SFFontRef SFFontCreateWithFTFace(FT_Face ftFace)
{
    if (ftFace) {
        SFFontRef font = malloc(sizeof(SFFont));

        FT_Reference_Face(ftFace);

        font->_ftFace = ftFace;
        font->unitsPerEm = ftFace->units_per_EM;
        font->ascender = ftFace->ascender;
        font->descender = ftFace->descender;
        font->leading = font->ascender - font->descender;
        font->_retainCount = 1;

        return font;
    }

    return NULL;
}

SFInteger SFFontGetUnitsPerEm(SFFontRef font)
{
    return font->unitsPerEm;
}

SFInteger SFFontGetAscender(SFFontRef font)
{
    return font->ascender;
}

SFInteger SFFontGetDescender(SFFontRef font)
{
    return font->descender;
}

SFInteger SFFontGetLeading(SFFontRef font)
{
    return font->leading;
}

SFGlyphID SFFontGetCodePointGlyph(SFFontRef font, SFCodePoint codePoint) {
    /*
     * OpenType recommendation for 'cmap' table:
     *      "The number of glyphs that may be included in one font is limited to
     *       64k."
     * Reference:
     *      https://www.microsoft.com/typography/otspec/recom.htm
     * Conclusion:
     *      It is safe to assume that a font will not contain more than 64k
     *      glyphs. So, if the glyph id returned by FreeType is greater than
     *      64k, it is considered invalid.
     */
    FT_UInt glyphIndex =  FT_Get_Char_Index(font->_ftFace, codePoint);
    if (glyphIndex <= 64000) {
        return (SFGlyphID)glyphIndex;
    }

    return 0;
}

SFInteger SFFontGetGlyphAdvance(SFFontRef font, SFGlyphID glyph)
{
    FT_Fixed advance;
    FT_Get_Advance(font->_ftFace, glyph, FT_LOAD_NO_SCALE, &advance);

    return advance;
}

SFFontRef SFFontRetain(SFFontRef font)
{
    if (font) {
        font->_retainCount++;
    }

    return font;
}

void SFFontRelease(SFFontRef font)
{
    if (font && --font->_retainCount == 0) {
        FT_Done_Face(font->_ftFace);
        SFTableCacheFinalize(&font->tables);
        free(font);
    }
}
