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

#ifndef SF_GLYPH_MANIPULATION_INTERNAL_H
#define SF_GLYPH_MANIPULATION_INTERNAL_H

/*
 * Note:
 *  This file is part of SFShapingEngine class. It handles common aspects of
 *  both substitution and positioning of glyphs.
 */

#include <SFConfig.h>

#include "SFCollection.h"
#include "SFData.h"
#include "SFShapingEngine.h"

SF_PRIVATE SFUInteger _SFSearchCoverageIndex(SFData coverage, SFGlyph glyph);
SF_PRIVATE SFUInt16 _SFSearchGlyphClass(SFData classDef, SFGlyph glyph);

SF_PRIVATE SFGlyphTrait _SFGetGlyphTrait(SFShapingEngineRef engine, SFGlyph glyph);

#endif
