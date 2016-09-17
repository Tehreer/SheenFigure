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

#ifndef _SF_PUBLIC_PATTERN_H
#define _SF_PUBLIC_PATTERN_H

#include "SFBase.h"
#include "SFFont.h"

/**
 * The type used to represent an open type scheme's pattern.
 */
typedef struct _SFPattern *SFPatternRef;

/**
 * Returns the font object for the pattern.
 *
 * @param pattern
 *      The pattern for which to return the font object.
 * @return
 *      The font object.
 */
SFFontRef SFPatternGetFont(SFPatternRef pattern);

/**
 * Returns the script tag for the pattern.
 *
 * @param pattern
 *      The pattern for which to return the script tag.
 * @return
 *      The script tag.
 */
SFTag SFPatternGetScriptTag(SFPatternRef pattern);

/**
 * Returns the language tag for the pattern.
 *
 * @param pattern
 *      The pattern for which to return the language tag.
 * @return
 *      The language tag.
 */
SFTag SFPatternGetLanguageTag(SFPatternRef pattern);

/**
 * Returns the total feature count for the pattern.
 *
 * @param pattern
 *      The pattern for which to return the feature count.
 * @return
 *      The number of features that the pattern contains.
 */
SFUInteger SFPatternGetFeatureCount(SFPatternRef pattern);

/**
 * Copies tags of all features into a user-provided buffer.
 *
 * @param pattern
 *      The pattern from which to copy the feature tags.
 * @param buffer
 *      The buffer where the feature tags will be copied to. The buffer should be large enough to
 *      hold the tags of all features.
 */
void SFPatternGetFeatureTags(SFPatternRef pattern, SFTag *buffer);

SFPatternRef SFPatternRetain(SFPatternRef pattern);
void SFPatternRelease(SFPatternRef pattern);

#endif
