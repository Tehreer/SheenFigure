/*
 * Copyright (C) 2015-2019 Muhammad Tayyab Akram
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

#include "SFAlbum.h"
#include "SFBase.h"
#include "SFCodepoints.h"
#include "SFFont.h"
#include "SFPattern.h"

#include "Tables/GDEF.h"
#include "Tables/OpenType.h"

#include "GlyphDiscovery.h"
#include "TextProcessor.h"

static SFBoolean IsZeroWidthCodepoint(SFCodepoint codepoint)
{
    /* TODO: Handle other control characters as well. */
    return SFCodepointInRange(codepoint, 0x200B, 0x200F);
}

SF_PRIVATE GlyphTraits GetGlyphTraits(TextProcessorRef textProcessor, SFGlyphID glyph)
{
    Data glyphClassDef = textProcessor->_glyphClassDef;

    if (glyphClassDef) {
        SFUInt16 glyphClass = SearchGlyphClass(glyphClassDef, glyph);

        /* Convert glyph class to traits options. */
        switch (glyphClass) {
            case GlyphClassValueBase:
                return GlyphTraitBase;

            case GlyphClassValueLigature:
                return GlyphTraitLigature;

            case GlyphClassValueMark:
                return GlyphTraitMark;

            case GlyphClassValueComponent:
                return GlyphTraitComponent;
        }
    }

    return GlyphTraitNone;
}

SF_PRIVATE void DiscoverGlyphs(TextProcessorRef textProcessor)
{
    SFPatternRef pattern = textProcessor->_pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = textProcessor->_album;
    SFCodepointsRef codepoints = album->codepoints;
    SFBoolean isRTL = textProcessor->_textDirection == SFTextDirectionRightToLeft;
    SFCodepoint current;

    SFCodepointsReset(codepoints);

    while ((current = SFCodepointsNext(codepoints)) != SFCodepointInvalid) {
        SFGlyphID glyph;
        GlyphTraits traits;

        if (isRTL) {
            SFCodepoint mirror = SFCodepointsGetMirror(current);

            if (mirror) {
                current = mirror;
            }
        }

        glyph = SFFontGetGlyphIDForCodepoint(font, current);
        traits = GetGlyphTraits(textProcessor, glyph);

        if (IsZeroWidthCodepoint(current)) {
            textProcessor->_containsZeroWidthCodepoints = SFTrue;
            traits |= GlyphTraitZeroWidth;
        }

        SFAlbumAddGlyph(album, glyph, traits, codepoints->index);
    }
}
