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

#ifndef SF_INTERNAL_TABLE_CACHE_H
#define SF_INTERNAL_TABLE_CACHE_H

#include <SFConfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "SFData.h"

struct _SFTableCache;
typedef struct _SFTableCache SFTableCache;
typedef SFTableCache *SFTableCacheRef;

struct _SFTableCache {
    SFData gdef;
    SFData gsub;
    SFData gpos;
};

SF_INTERNAL void SFTableCacheInitialize(SFTableCacheRef tableCache, FT_Face ftFace);
SF_INTERNAL void SFTableCacheInvalidate(SFTableCacheRef tableCache);

#endif
