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

#include "SFCollection.h"
#include "SFFont.h"

#include "SFGlyphDiscovery.h"
#include "SFTextProcessor.h"

SF_INTERNAL void _SFDiscoverGlyphs(SFTextProcessorRef processor)
{
    SFFontRef font = processor->_font;
    SFCollectionRef collection = processor->_collection;
    SFUInteger length = collection->codePointCount;
    SFUInteger index;

    for (index = 0; index < length; index++) {
        SFCodePoint codePoint = collection->codePointArray[index];
        SFGlyphID glyph = SFFontGetGlyphForCodePoint(font, codePoint);
        SFCollectionAddGlyph(collection, glyph, index);
    }
}
