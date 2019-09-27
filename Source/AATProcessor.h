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

#ifndef _SF_INTERNAL_AAT_PROCESSOR_H
#define _SF_INTERNAL_AAT_PROCESSOR_H

#include <SFConfig.h>

#include "SFAlbum.h"
#include "SFBase.h"
#include "SFPattern.h"

typedef struct _AATProcessor {
    SFPatternRef pattern;
    SFAlbumRef album;
    SFTextDirection textDirection;
} AATProcessor, *AATProcessorRef;

SF_INTERNAL void AATProcessorInitialize(AATProcessorRef aatProcessor,
   SFPatternRef pattern, SFAlbumRef album, SFTextDirection textDirection);

SF_INTERNAL void AATProcessorDiscoverGlyphs(AATProcessorRef aatProcessor);
SF_INTERNAL void AATProcessorTransformGlyphs(AATProcessorRef aatProcessor);

#endif
