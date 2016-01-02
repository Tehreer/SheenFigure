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

#ifndef _SF_TEXT_PROCESSOR_INTERNAL_H
#define _SF_TEXT_PROCESSOR_INTERNAL_H

#include <SFConfig.h>
#include <SFDirection.h>
#include <SFTypes.h>

#include "SFAlbum.h"
#include "SFFont.h"
#include "SFLocator.h"
#include "SFPattern.h"
#include "SFFontCache.h"

typedef struct _SFTextProcessor {
    SFFontRef _font;
    SFPatternRef _pattern;
    SFAlbumRef _album;
    SFLocator _locator;
    SFData _glyphClassDef;
    SFDirection _direction;
    /* State values. */
    SFFeatureTag _featureTag;
    SFHeaderKind _headerKind;
} SFTextProcessor, *SFTextProcessorRef;

SF_INTERNAL void SFTextProcessorInitialize(SFTextProcessorRef textProcessor, SFPatternRef pattern, SFAlbumRef album);

SF_INTERNAL void SFTextProcessorDiscoverGlyphs(SFTextProcessorRef textProcessor);
SF_INTERNAL void SFTextProcessorSubstituteGlyphs(SFTextProcessorRef textProcessor);
SF_INTERNAL void SFTextProcessorPositionGlyphs(SFTextProcessorRef textProcessor);

#endif
