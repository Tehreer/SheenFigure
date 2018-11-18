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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SFBase.h"
#include "Data.h"
#include "SFFont.h"

static SFUInt8 *CopySFNTTable(const SFFontProtocol *protocol, void *object, SFTag tableTag) {
    SFUInt8 *data = NULL;
    SFUInteger length = 0;

    protocol->loadTable(object, tableTag, NULL, &length);

    if (length != 0) {
        data = malloc(length);
        protocol->loadTable(object, tableTag, data, NULL);
    }

    return data;
}

static FontResourceRef CreateFontResource(const SFFontProtocol *protocol, void *object)
{
    FontResourceRef fontResource = malloc(sizeof(FontResource));
    fontResource->retainCount = 1;

    /* Load the open type tables. */
    fontResource->gdef = CopySFNTTable(protocol, object, TAG('G', 'D', 'E', 'F'));
    fontResource->gsub = CopySFNTTable(protocol, object, TAG('G', 'S', 'U', 'B'));
    fontResource->gpos = CopySFNTTable(protocol, object, TAG('G', 'P', 'O', 'S'));

    return fontResource;
}

static FontResourceRef RetainFontResource(FontResourceRef fontResource)
{
    if (fontResource) {
        fontResource->retainCount++;
    }

    return fontResource;
}

static void ReleaseFontResource(FontResourceRef fontResource)
{
    if (fontResource && --fontResource->retainCount == 0) {
        free((void *)fontResource->gdef);
        free((void *)fontResource->gsub);
        free((void *)fontResource->gpos);
        free(fontResource);
    }
}

static SFInt32 ZeroGlyphAdvance(void *object, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    return 0;
}

SFFontRef SFFontCreateWithProtocol(const SFFontProtocol *protocol, void *object)
{
    /* Verify that required functions exist in the protocol. */
    if (protocol && protocol->loadTable && protocol->getGlyphIDForCodepoint) {
        SFFontRef font = malloc(sizeof(SFFont));
        font->protocol = *protocol;
        font->object = object;
        font->resource = CreateFontResource(protocol, object);
        font->coordArray = NULL;
        font->coordCount = 0;
        font->retainCount = 1;

        if (!font->protocol.getAdvanceForGlyph) {
            font->protocol.getAdvanceForGlyph = ZeroGlyphAdvance;
        }

        return font;
    }

    return NULL;
}

SF_INTERNAL SFGlyphID SFFontGetGlyphIDForCodepoint(SFFontRef font, SFCodepoint codepoint)
{
    return font->protocol.getGlyphIDForCodepoint(font->object, codepoint);
}

SF_INTERNAL SFAdvance SFFontGetAdvanceForGlyph(SFFontRef font, SFFontLayout fontLayout, SFGlyphID glyphID)
{
    return font->protocol.getAdvanceForGlyph(font->object, fontLayout, glyphID);
}

SFFontRef SFFontRetain(SFFontRef font)
{
    if (font) {
        font->retainCount++;
    }

    return font;
}

void SFFontRelease(SFFontRef font)
{
    if (font && --font->retainCount == 0) {
        if (font->protocol.finalize) {
            font->protocol.finalize(font->object);
        }

        ReleaseFontResource(font->resource);
        free(font);
    }
}
