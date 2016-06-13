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

void SFCodepointsInitialize(SFCodepointsRef codepoints, SBCodepointSequenceRef referral, SFRange range, SFBoolean backward)
{
    /* Range MUST be valid. */
    SFAssert(SFRangeFallsInLength(range, SBCodepointSequenceGetStringLength(referral)));

    codepoints->referral = referral;
    codepoints->index = SFInvalidIndex;
    codepoints->_start = range.start;
    codepoints->_limit = range.start + range.count;
    codepoints->backward = backward;
}

void SFCodepointsReset(SFCodepointsRef codepoints)
{
    codepoints->index = (!codepoints->backward ? codepoints->_start : codepoints->_limit);
}

SFCodepoint SFCodepointsNext(SFCodepointsRef codepoints)
{
    if (!codepoints->backward) {
        if (codepoints->index < codepoints->_limit) {
            return SBCodepointSequenceGetCodepointAt(codepoints->referral, &codepoints->index);
        }
    } else {
        if (codepoints->index > codepoints->_start) {
            return SBCodepointSequenceGetCodepointBefore(codepoints->referral, &codepoints->index);
        }
    }

    return SFCodepointInvalid;
}
