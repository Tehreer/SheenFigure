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

#include <stddef.h>
#include <stdlib.h>

#include "SFData.h"
#include "SFFont.h"

static SFUInt8 *_SFFontCopyTable(SFFontRef font, SFTag tag) {
    SFUInt8 *data = NULL;
    SFUInteger length = 0;

    SFFontLoadTable(font, tag, NULL, &length);

    if (length) {
        data = malloc(length);
        SFFontLoadTable(font, tag, data, NULL);
    }

    return data;
}

SFFontRef SFFontCreateWithProtocol(const SFFontProtocol *protocol, void *object)
{
    /* Verify that required functions exist in protocol. */
    if (protocol->loadTable && protocol->getGlyphIDForCodepoint) {
        SFFontRef font = malloc(sizeof(SFFont));
        font->_protocol = *protocol;
        font->_object = object;
        font->_retainCount = 1;

        /* Load open type tables. */
        font->tables.gdef = _SFFontCopyTable(font, SFTagMake('G', 'D', 'E', 'F'));
        font->tables.gsub = _SFFontCopyTable(font, SFTagMake('G', 'S', 'U', 'B'));
        font->tables.gpos = _SFFontCopyTable(font, SFTagMake('G', 'P', 'O', 'S'));

        return font;
    }

    return NULL;
}

SF_INTERNAL void SFFontLoadTable(SFFontRef font, SFTag tableTag, SFUInt8 *buffer, SFUInteger *length)
{
    font->_protocol.loadTable(font->_object, tableTag, buffer, length);
}

SF_INTERNAL SFGlyphID SFFontGetGlyphIDForCodepoint(SFFontRef font, SFCodepoint codepoint)
{
    return font->_protocol.getGlyphIDForCodepoint(font->_object, codepoint);
}

SF_INTERNAL SFInteger SFFontGetAdvanceForGlyph(SFFontRef font, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    return font->_protocol.getAdvanceForGlyph(font->_object, fontLayout, glyphID);
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
        if (font->_protocol.finalize) {
            font->_protocol.finalize(font->_object);
        }
        free((void *)font->tables.gdef);
        free((void *)font->tables.gsub);
        free((void *)font->tables.gpos);
        free(font);
    }
}
