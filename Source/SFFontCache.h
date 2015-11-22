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

#ifndef SF_FONT_CACHE_INTERNAL_H
#define SF_FONT_CACHE_INTERNAL_H

#include <SFConfig.h>

#include "SFScriptCache.h"
#include "SFTableCache.h"

struct _SFFontCache;
typedef struct _SFFontCache SFFontCache;

struct _SFFontCache {
    SFTableCacheRef tables;
    SFScriptCacheRef scripts;
};

#endif
