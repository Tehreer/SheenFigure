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

#ifndef _SF_GLYPH_MANIPULATION_INTERNAL_H
#define _SF_GLYPH_MANIPULATION_INTERNAL_H

/*
 * NOTE:
 *      This file is part of SFTextProcessor class. It handles common aspects of substitution and
 *      positioning of glyphs.
 */

#include <SFConfig.h>

#include "SFAlbum.h"
#include "SFData.h"
#include "SFLocator.h"
#include "SFTextProcessor.h"

SF_PRIVATE SFUInteger _SFSearchCoverageIndex(SFData coverage, SFGlyphID glyph);
SF_PRIVATE SFUInt16 _SFSearchGlyphClass(SFData classDef, SFGlyphID glyph);

SF_PRIVATE SFGlyphTraits _SFGetGlyphTraits(SFTextProcessorRef processor, SFGlyphID glyph);

SF_PRIVATE SFBoolean _SFApplyExtensionSubtable(SFTextProcessorRef processor, SFData extensionSubtable);
SF_PRIVATE SFBoolean _SFApplyChainContextSubtable(SFTextProcessorRef processor, SFData chainContext);

#endif
