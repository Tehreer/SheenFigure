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

#ifndef _SF_INTERNAL_CODEPOINTS_H
#define _SF_INTERNAL_CODEPOINTS_H

#include <SBCodepointSequence.h>
#include <SFConfig.h>

#include "SFBase.h"

typedef struct _SFCodepoints {
    const SBCodepointSequence *_referral;
    SFUInteger _token;
    SFUInteger index;
    SFBoolean backward;
} SFCodepoints, *SFCodepointsRef;

SF_INTERNAL SFCodepoint SFCodepointsGetMirror(SFCodepoint codepoint);

SF_INTERNAL void SFCodepointsInitialize(SFCodepointsRef codepoints, const SBCodepointSequence *referral, SFBoolean backward);
SF_INTERNAL void SFCodepointsReset(SFCodepointsRef codepoints);
SF_INTERNAL SFCodepoint SFCodepointsNext(SFCodepointsRef codepoints);

#endif
