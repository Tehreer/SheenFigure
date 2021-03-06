/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#ifndef _SF_PUBLIC_SCHEME_H
#define _SF_PUBLIC_SCHEME_H

#include "SFBase.h"
#include "SFFont.h"
#include "SFPattern.h"

/**
 * The type used to represent an open type scheme.
 */
typedef struct _SFScheme *SFSchemeRef;

SFSchemeRef SFSchemeCreate(void);

/**
 * Sets the font in a scheme.
 *
 * @param scheme
 *      The scheme for which to set the font.
 * @param font
 *      A font.
 */
void SFSchemeSetFont(SFSchemeRef scheme, SFFontRef font);

/**
 * Sets the script tag in a scheme.
 *
 * @param scheme
 *      The scheme for which to set the script tag.
 * @param scriptTag
 *      A script tag.
 */
void SFSchemeSetScriptTag(SFSchemeRef scheme, SFTag scriptTag);

/**
 * Sets the language tag in a scheme.
 *
 * @param scheme
 *      The scheme for which to set the language tag.
 * @param languageTag
 *      A language tag.
 */
void SFSchemeSetLanguageTag(SFSchemeRef scheme, SFTag languageTag);

/**
 * Sets the OpenType feature values in a scheme.
 *
 * If the value of a feature is set to zero, it would be disabled provided that it is not a required
 * feature of the script specific shaping engine. If the value of a feature is greater than zero, it
 * would be enabled. In case of an alternate feature, this value would be used to pick the alternate
 * glyph at this position.
 *
 * @param scheme
 *      The scheme for which to set the feature values.
 * @param featureTags
 *      An array of feature tags whose values should be set.
 * @param featureValues
 *      An array of OpenType feature values.
 * @param featureCount
 *      The number of elements in feature tags and feature values array.
 */
void SFSchemeSetFeatureValues(SFSchemeRef scheme,
    SFTag *featureTags, SFUInt16 *featureValues, SFUInteger featureCount);

/**
 * Builds a pattern for the scheme.
 *
 * @param scheme
 *      The scheme for which to build the pattern.
 * @return
 *      A reference to a pattern object if the call was successful, NULL otherwise.
 */
SFPatternRef SFSchemeBuildPattern(SFSchemeRef scheme);

SFSchemeRef SFSchemeRetain(SFSchemeRef scheme);
void SFSchemeRelease(SFSchemeRef scheme);

#endif
