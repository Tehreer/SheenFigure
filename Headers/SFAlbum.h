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

#ifndef SF_ALBUM_H
#define SF_ALBUM_H

#include "SFTypes.h"

struct __SFAlbum;
typedef struct __SFAlbum SFAlbum;
/**
 * The type used to represent a glyph set.
 */
typedef SFAlbum *SFAlbumRef;

/**
 * Justifies a range of text within the given space.
 * @discussion
 *      This function adjusts glyph positions or inserts new glyphs such as
 *      Kashida for text justification.
 * @param glyphSet
 *      The glyph set whose text you want to justify.
 * @param textStart
 *      The real starting location of the text from where you want to start
 *      justification.
 * @param textLength
 *      The length of the text which you want to justify.
 * @param space
 *      The space in which you want to justify the text. It must be in units per
 *      em of the font.
 */
void SFAlbumJustifyTextRange(SFAlbumRef glyphSet, SFUInteger textStart, SFUInteger textLength, SFUInteger space);

/**
 * Replaces the glyph at the specified character index with the glyph of given
 * character.
 * @discussion
 *      This function is designed for implementing L4 rule of Unicode
 *      Bidirectional Algorithm. The given character is asumed mirror and it's
 *      glyph is replaced, if exists. The advances or positions corresponding to
 *      the original glyph are not changed.
 * @param glyphSet
 *      The glyph set whose character you want to replace.
 * @param index
 *      The index of the character to be replaced.
 * @param character
 *      The replacement character.
 */
void SFAlbumReplaceCharacter(SFAlbumRef album, SFUInteger index, SFUnichar character);

/**
 * Replaces normal glyphs with mirrored ones using the same font as selected for
 * shaping.
 * @note
 *      This function does not perform character level mirroring. For that
 *      purpose, you must call SFAlbumReplaceCharacter for each mirroring
 *      character.
 */
void SFAlbumDoMirroring(SFAlbumRef album);

/**
 * Provides the range of analysed text.
 * @param glyphSet
 *      The glyph set whose text range you want to obtain.
 * @return
 *      The range of input text analysed by the shaping process.
 */
SFRange SFAlbumGetTextRange(SFAlbumRef album);

/**
 * Provides the number of produced glyphs.
 * @param glyphSet
 *      The glyph set whose glyph count you want to obtain.
 * @return
 *      The number of glyphs contained in the glyph set.
 */
SFUInteger SFAlbumGetGlyphCount(SFAlbumRef album);

/**
 * Provides an array of glyphs corresponding to the input text.
 * @param glyphSet
 *      The glyph set whose glyphs you want to obtain.
 * @return
 *      An array of glyphs produced as part of shaping process.
 */
SFGlyph *SFAlbumGetGlyphs(SFAlbumRef album);

/**
 * Provides an array of glyph frames in units per em of the font. Every frame is
 * positioned with respect to zero origin.
 * @param glyphSet
 *      The glyph set whose frames you want to obtain.
 * @return
 *      An array of glyph frames produced as part of shaping process.
 */
SFFrame *SFAlbumGetGlyphFrames(SFAlbumRef album);

/**
 * Provides an array mapping each character to corresponding range of glyph/s.
 * @param glyphSet
 *      The glyph set whose charater to glyph map you want to obtain.
 * @return
 *      An array of ranges mapping characters to glyphs.
 */
SFRange *SFAlbumGetCharacterToGlyphMap(SFAlbumRef album);

#endif
