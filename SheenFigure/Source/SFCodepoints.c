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

#include <SFConfig.h>
#include <stddef.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"

void SFCodepointsInitialize(SFCodepointsRef codepoints, const SBCodepointSequence *referral, SFBoolean backward)
{
    codepoints->referral = referral;
    codepoints->index = SFInvalidIndex;
    codepoints->backward = backward;
}

void SFCodepointsReset(SFCodepointsRef codepoints)
{
    codepoints->index = (!codepoints->backward ? 0 : codepoints->referral->stringLength);
}

SFCodepoint SFCodepointsNext(SFCodepointsRef codepoints)
{
    if (!codepoints->backward) {
        return SBCodepointSequenceGetCodepointAt(codepoints->referral, &codepoints->index);
    }

    return SBCodepointSequenceGetCodepointBefore(codepoints->referral, &codepoints->index);
}
