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

SF_PRIVATE SFGlyphTraits _SFGetGlyphTraits(SFTextProcessorRef processor, SFGlyphID glyph)
{
    SFData glyphClassDef = processor->_glyphClassDef;

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

SF_INTERNAL void _SFDiscoverGlyphs(SFTextProcessorRef processor)
{
    SFPatternRef pattern = processor->_pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = processor->_album;
    SFCodepointsRef codepoints = album->codepoints;
    SFBoolean isRTL = processor->_textDirection == SFTextDirectionRightToLeft;

    SFCodepointsReset(album->codepoints);

    switch (processor->_textMode) {
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
                traits = _SFGetGlyphTraits(processor, glyph);
                SFAlbumAddGlyph(album, glyph, traits, codepoints->index);
            }
            break;
        }
    }
}
