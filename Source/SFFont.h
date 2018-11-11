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

#ifndef _SF_INTERNAL_FONT_H
#define _SF_INTERNAL_FONT_H

#include <SFConfig.h>
#include <SFFont.h>

#include "SFBase.h"
#include "SFData.h"

typedef struct _SFFontTables {
    Data gdef;
    Data gsub;
    Data gpos;
} SFFontTables;

typedef struct _SFFont {
    SFFontProtocol _protocol;
    void *_object;
    SFFontTables tables;
    SFUInteger _retainCount;
} SFFont;

SF_INTERNAL void SFFontLoadTable(SFFontRef font, SFTag tableTag, SFUInt8 *buffer, SFUInteger *length);
SF_INTERNAL SFGlyphID SFFontGetGlyphIDForCodepoint(SFFontRef font, SFCodepoint codepoint);
SF_INTERNAL SFAdvance SFFontGetAdvanceForGlyph(SFFontRef font, SFFontLayout fontLayout, SFGlyphID glyphID);

#endif
