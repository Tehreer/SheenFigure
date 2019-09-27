/*
 * Copyright (C) 2019 Muhammad Tayyab Akram
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
#include "SFPattern.h"

#include "Tables/Morx.h"

#include "AATProcessor.h"

SF_INTERNAL void AATProcessorInitialize(AATProcessorRef aatProcessor,
    SFPatternRef pattern, SFAlbumRef album, SFTextDirection textDirection)
{
    aatProcessor->pattern = pattern;
    aatProcessor->album = album;
    aatProcessor->textDirection = textDirection;
}

SF_INTERNAL void AATProcessorDiscoverGlyphs(AATProcessorRef aatProcessor)
{
    SFPatternRef pattern = aatProcessor->pattern;
    SFFontRef font = pattern->font;
    SFAlbumRef album = aatProcessor->album;
    SFCodepointsRef codepoints = album->codepoints;
    SFBoolean isRTL = aatProcessor->textDirection == SFTextDirectionRightToLeft;
    SFCodepoint current;

    SFCodepointsReset(codepoints);

    while ((current = SFCodepointsNext(codepoints)) != SFCodepointInvalid) {
        SFGlyphID glyph;

        if (isRTL) {
            SFCodepoint mirror = SFCodepointsGetMirror(current);

            if (mirror) {
                current = mirror;
            }
        }

        glyph = SFFontGetGlyphIDForCodepoint(font, current);

        SFAlbumAddGlyph(album, glyph, GlyphTraitNone, codepoints->index);
    }
}

SF_INTERNAL void AATProcessorTransformGlyphs(AATProcessorRef aatProcessor)
{
    SFPatternRef pattern = aatProcessor->pattern;
    Data morxTable = pattern->font->resource->morx;

    if (morxTable) {
        SFUInt32 chainCount = Morx_NChains(morxTable);
        Data chain = Morx_FirstChain(morxTable);
        SFUInt32 chainIndex;

        for (chainIndex = 0; chainIndex < chainCount; chainIndex++) {
            SFUInt32 featureCount = MorxChain_NFeatureEntries(chain);
            SFUInt32 subtableCount = MorxChain_NSubtables(chain);
            Data morxSubtable = MorxChain_FirstSubtable(chain, featureCount);
            SFUInt32 subtableIndex;

            /* TODO: Handle Features. */

            for (subtableIndex = 0; subtableIndex < subtableCount; subtableIndex++) {
                /* TODO: Check Coverage. */

                morxSubtable = MorxSubtable_NextSubtable(morxSubtable);
            }

            chain = MorxChain_NextChain(chain);
        }
    }
}
