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

#ifndef _SF_SHAPING_ENGINE_INTERNAL_H
#define _SF_SHAPING_ENGINE_INTERNAL_H

#include <SFConfig.h>
#include <SFDirection.h>

#include "SFAlbum.h"
#include "SFShapingEngine.h"

/**
 * A common interface of all shaping engines.
 */
typedef struct _SFShapingEngine {
    void (*_processAlbum)(const void *, SFAlbumRef, SFDirection);
} SFShapingEngine, *SFShapingEngineRef;

SF_INTERNAL void SFShapingEngineProcessAlbum(SFShapingEngineRef shapingEngine, SFAlbumRef album, SFDirection direction);

#endif
