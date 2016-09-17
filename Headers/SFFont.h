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

#ifndef _SF_PUBLIC_FONT_H
#define _SF_PUBLIC_FONT_H

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

/**
 * The function invoked when a font is about to be deallocated.
 *
 * @param object
 *      The object associated with the font.
 */
typedef void (*SFFontProtocolFinalizeFunc)(void *object);

/**
 * The function used to load a font table into a buffer.
 *
 * @param object
 *      The object associated with the font.
 * @param tableTag
 *      The tag of the table to load.
 * @param buffer
 *      The target buffer that is large enough to hold the data of the table. This parameter can be
 *      NULL if not needed.
 * @param length
 *      The pointer that takes the length of the table. This parameter can be NULL if not needed.
 */
typedef void (*SFFontProtocolLoadTableFunc)(void *object, SFTag tableTag, SFUInt8 *buffer, SFUInteger *length);

/**
 * The function used to get the glyph ID of a code point.
 *
 * @param object
 *      The object associated with the font.
 * @param codepoint
 *      The codepoint for which to get the glyph ID.
 * @return
 *      The glyph ID of the passed-in code point.
 */
typedef SFGlyphID (*SFFontProtocolGetGlyphIDForCodepointFunc)(void *object, SFCodepoint codepoint);

/**
 * The function used to get the advance of a glyph.
 *
 * @param object
 *      The object associated with the font.
 * @param fontLayout
 *      The drawing layout of the glyph.
 * @param glyphID
 *      The glyph ID for which to get the advance.
 * @return
 *      The advance of the passed-in glyph ID.
 */
typedef SFAdvance (*SFFontProtocolGetAdvanceForGlyphFunc)(void *object, SFFontLayout fontLayout, SFGlyphID glyphID);

/**
 * Structure containing the functions of a SFFont.
 */
typedef struct _SFFontProtocol {
    /**
     * The function used when a font is about to be deallocated. This function may be NULL.
     */
    SFFontProtocolFinalizeFunc finalize;
    /**
     * The function used to load the table of a font into a buffer.
     */
    SFFontProtocolLoadTableFunc loadTable;
    /**
     * The function used to get the glyph ID of a code point.
     */
    SFFontProtocolGetGlyphIDForCodepointFunc getGlyphIDForCodepoint;
    /**
     * The function used to get the advance of a glyph. This function may be NULL, which is
     * equivalent to a getAdvanceForGlyph function that always returns 0.
     */
    SFFontProtocolGetAdvanceForGlyphFunc getAdvanceForGlyph;
} SFFontProtocol;

/**
 * Creates a font object with a given protocol.
 *
 * @param protocol
 *      A structure holding pointers to the implemented functions for this font.
 * @param object
 *      An object associated with the font to identify it.
 * @return
 *      A reference to a font object if the call was successful, NULL otherwise.
 */
SFFontRef SFFontCreateWithProtocol(const SFFontProtocol *protocol, void *object);

SFFontRef SFFontRetain(SFFontRef font);
void SFFontRelease(SFFontRef font);

#endif
