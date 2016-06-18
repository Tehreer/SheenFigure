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

#ifndef _SF_FONT_H
#define _SF_FONT_H

#include "SFBase.h"

enum {
    SFFontLayoutHorizontal = 0,
    SFFontLayoutVertical = 1
};
typedef SFUInt32 SFFontLayout;

/**
 * The type used to represent a font.
 */
typedef struct _SFFont *SFFontRef;

typedef void (*SFFontProtocolFinalizeFunc)(void *object);
typedef void (*SFFontProtocolLoadTableFunc)(void *object, SFTag tableTag, SFUInt8 *buffer, SFUInteger *length);
typedef SFGlyphID (*SFFontProtocolGetGlyphIDForCodepointFunc)(void *object, SFCodepoint codepoint);
typedef SFAdvance (*SFFontProtocolGetAdvanceForGlyphFunc)(void *object, SFFontLayout fontLayout, SFGlyphID glyphID);

typedef struct _SFFontProtocol {
    SFFontProtocolFinalizeFunc finalize;
    SFFontProtocolLoadTableFunc loadTable;
    SFFontProtocolGetGlyphIDForCodepointFunc getGlyphIDForCodepoint;
    SFFontProtocolGetAdvanceForGlyphFunc getAdvanceForGlyph;
} SFFontProtocol;

SFFontRef SFFontCreateWithProtocol(const SFFontProtocol *protocol, void *object);
SFFontRef SFFontRetain(SFFontRef font);
void SFFontRelease(SFFontRef font);

#endif
