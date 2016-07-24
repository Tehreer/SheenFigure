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

#ifndef _SF_CODEPOINTS_INTERNAL_H
#define _SF_CODEPOINTS_INTERNAL_H

#include <SFConfig.h>
#include <SBCodepointSequence.h>

#include "SFBase.h"

typedef struct _SFCodepoints {
    SBCodepointSequencePtr referral;
    SFUInteger index;
    SFBoolean backward;
} SFCodepoints, *SFCodepointsRef;

void SFCodepointsInitialize(SFCodepointsRef codepoints, SBCodepointSequencePtr referral, SFBoolean backward);

void SFCodepointsReset(SFCodepointsRef codepoints);
SFCodepoint SFCodepointsNext(SFCodepointsRef codepoints);

#endif
