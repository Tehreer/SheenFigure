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

#include <SBBase.h>
#include <SBCodepointSequence.h>
#include <SFConfig.h>

#include "SFAssert.h"
#include "SFBase.h"
#include "SFCodepoints.h"

SF_INTERNAL SFCodepoint SFCodepointsGetMirror(SFCodepoint codepoint)
{
    SFCodepoint mirror = SBCodepointGetMirror(codepoint);
    if (!mirror) {
        return codepoint;
    }

    return mirror;
}

SF_INTERNAL void SFCodepointsInitialize(SFCodepointsRef codepoints, const SBCodepointSequence *referral, SFBoolean backward)
{
    codepoints->_referral = referral;
    codepoints->_token = SFInvalidIndex;
    codepoints->index = SFInvalidIndex;
    codepoints->backward = backward;
}

SF_INTERNAL void SFCodepointsReset(SFCodepointsRef codepoints)
{
    codepoints->_token = (!codepoints->backward ? 0 : codepoints->_referral->stringLength);
}

SF_INTERNAL SFCodepoint SFCodepointsNext(SFCodepointsRef codepoints)
{
    SFCodepoint current;

    if (!codepoints->backward) {
        codepoints->index = codepoints->_token;
        current = SBCodepointSequenceGetCodepointAt(codepoints->_referral, &codepoints->_token);
    } else {
        current = SBCodepointSequenceGetCodepointBefore(codepoints->_referral, &codepoints->_token);
        codepoints->index = codepoints->_token;
    }

    return current;
}
