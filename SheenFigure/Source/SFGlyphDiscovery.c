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
#include <SFTypes.h>

#include "SFAlbum.h"
#include "SFFont.h"
#include "SFGDEF.h"
#include "SFOpenType.h"

#include "SFGlyphDiscovery.h"
#include "SFTextProcessor.h"

static SFGlyphTraits _SFGlyphClassToGlyphTraits(SFUInt16 glyphClass)
{
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

    return SFGlyphTraitNone;
}

SF_PRIVATE SFGlyphTraits _SFGetGlyphTraits(SFTextProcessorRef processor, SFGlyphID glyph)
{
    if (processor->_glyphClassDef) {
        SFUInt16 glyphClass = SFOpenTypeSearchGlyphClass(processor->_glyphClassDef, glyph);
        return _SFGlyphClassToGlyphTraits(glyphClass);
    }
    
    return SFGlyphTraitNone;
}

SF_INTERNAL void _SFDiscoverGlyphs(SFTextProcessorRef processor)
{
    SFPatternRef pattern = processor->_pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = processor->_album;
    SBCodepointSequenceRef sequence = album->codepointSequence;
    SFUInteger stringIndex;
    SFUInteger oldIndex;
    SBCodepoint codepoint;

    switch (processor->_textMode) {
    case SFTextModeForward:
        stringIndex = 0;
        oldIndex = stringIndex;

        while ((codepoint = SBCodepointSequenceGetCodepointAt(sequence, &stringIndex)) != SBCodepointInvalid) {
            SFGlyphID glyph = SFFontGetGlyphIDForCodepoint(font, codepoint);
            SFAlbumAddGlyph(album, glyph, oldIndex, stringIndex - oldIndex);

            oldIndex = stringIndex;
        }
        break;

    case SFTextModeBackward:
        stringIndex = SBCodepointSequenceGetStringLength(sequence);
        oldIndex = stringIndex;

        while ((codepoint = SBCodepointSequenceGetCodepointBefore(sequence, &stringIndex)) != SBCodepointInvalid) {
            SFGlyphID glyph = SFFontGetGlyphIDForCodepoint(font, codepoint);
            SFAlbumAddGlyph(album, glyph, stringIndex, oldIndex - stringIndex);

            oldIndex = stringIndex;
        }
        break;

    default:
        /* Unknown text mode. */
        SFAssert(SFFalse);
        break;
    }
}
