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

#ifndef _SF_INTERNAL_SCHEME_H
#define _SF_INTERNAL_SCHEME_H

#include <SFScheme.h>

#include "SFBase.h"
#include "SFFont.h"
#include "SFShapingKnowledge.h"

typedef struct _SFScheme {
    SFFontRef _font;                /**< Font, whose scheme is being built. */
    SFTag _scriptTag;               /**< Tag of the script. */
    SFTag _languageTag;             /**< Tag of the language. */
    SFTag *_featureTags;            /**< Tags of features to override. */
    SFUInt16 *_featureValues;       /**< Values of features to override. */
    SFUInteger _featureCount;       /**< The number of features to override. */

    SFInteger _retainCount;
} SFScheme;

#endif
