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

#ifndef _SF_INTERNAL_TEXT_PROCESSOR_H
#define _SF_INTERNAL_TEXT_PROCESSOR_H

#include <SFConfig.h>

#include "SFAlbum.h"
#include "SFBase.h"
#include "SFFont.h"
#include "SFLocator.h"
#include "SFPattern.h"

typedef struct _TextProcessor {
    SFPatternRef _pattern;
    SFAlbumRef _album;
    SFData _glyphClassDef;
    SFData _lookupList;
    SFUInt16 _lookupValue;
    SFBoolean (*_lookupOperation)(struct _SFTextProcessor *, SFLookupType, SFData);
    SFTextDirection _textDirection;
    SFUInt16 _ppemWidth;
    SFUInt16 _ppemHeight;
    SFBoolean _zeroWidthMarks;
    SFBoolean _containsZeroWidthCodepoints;
    SFLocator _locator;
} TextProcessor, *TextProcessorRef;

SF_INTERNAL void TextProcessorInitialize(TextProcessorRef textProcessor,
   SFPatternRef pattern, SFAlbumRef album, SFTextDirection textDirection,
   SFUInt16 ppemWidth, SFUInt16 ppemHeight, SFBoolean zeroWidthMarks);

SF_INTERNAL void TextProcessorDiscoverGlyphs(TextProcessorRef textProcessor);
SF_INTERNAL void TextProcessorSubstituteGlyphs(TextProcessorRef textProcessor);
SF_INTERNAL void TextProcessorPositionGlyphs(TextProcessorRef textProcessor);
SF_INTERNAL void TextProcessorWrapUp(TextProcessorRef textProcessor);

SF_PRIVATE void ApplyLookup(TextProcessorRef textProcessor, SFUInt16 lookupIndex);

#endif
