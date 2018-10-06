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

#include <SFConfig.h>

#include "SFAlbum.h"
#include "SFBase.h"
#include "SFCodepoints.h"
#include "SFFont.h"
#include "SFGDEF.h"
#include "SFOpenType.h"
#include "SFPattern.h"

#include "SFGlyphDiscovery.h"
#include "SFTextProcessor.h"

static SFBoolean IsZeroWidthCodepoint(SFCodepoint codepoint)
{
    /* TODO: Handle other control characters as well. */
    return SFCodepointInRange(codepoint, 0x200B, 0x200F);
}

SF_PRIVATE SFGlyphTraits GetGlyphTraits(SFTextProcessorRef textProcessor, SFGlyphID glyph)
{
    SFData glyphClassDef = textProcessor->_glyphClassDef;

    if (glyphClassDef) {
        SFUInt16 glyphClass = SFOpenTypeSearchGlyphClass(glyphClassDef, glyph);

        /* Convert glyph class to traits options. */
        switch (glyphClass) {
            case SFGlyphClassValueBase:
                return SFGlyphTraitBase;

            case SFGlyphClassValueLigature:
                return SFGlyphTraitLigature;

            case SFGlyphClassValueMark:
                return SFGlyphTraitMark;

            case SFGlyphClassValueComponent:
                return SFGlyphTraitComponent;
        }
    }

    return SFGlyphTraitNone;
}

SF_PRIVATE void DiscoverGlyphs(SFTextProcessorRef textProcessor)
{
    SFPatternRef pattern = textProcessor->_pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = textProcessor->_album;
    SFCodepointsRef codepoints = album->codepoints;
    SFBoolean isRTL = textProcessor->_textDirection == SFTextDirectionRightToLeft;

    SFCodepointsReset(album->codepoints);

    switch (textProcessor->_textMode) {
        case SFTextModeForward:
        case SFTextModeBackward: {
            SFCodepoint current;

            while ((current = SFCodepointsNext(codepoints)) != SFCodepointInvalid) {
                SFGlyphID glyph;
                SFGlyphTraits traits;

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
                    traits |= SFGlyphTraitZeroWidth;
                }

                SFAlbumAddGlyph(album, glyph, traits, codepoints->index);
            }
            break;
        }
    }
}
