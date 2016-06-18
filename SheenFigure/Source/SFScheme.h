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

#ifndef _SF_SCHEME_INTERNAL_H
#define _SF_SCHEME_INTERNAL_H

#include <SFScheme.h>

#include "SFBase.h"
#include "SFData.h"
#include "SFFont.h"
#include "SFPatternBuilder.h"
#include "SFShapingKnowledge.h"

typedef struct _SFSchemeState {
    SFSchemeRef scheme;
    SFScriptKnowledgeRef knowledge;
    SFPatternBuilder builder;
    SFData scriptList;              /**< OpenType script list table. */
    SFData featureList;             /**< OpenType feature list table. */
    SFData script;                  /**< OpenType script table. */
    SFData langSys;                 /**< OpenType language system table. */
} _SFSchemeState, *_SFSchemeStateRef;

typedef struct _SFScheme {
    SFFontRef _font;                /**< Font, whose scheme is being built. */
    SFTag _scriptTag;               /**< Tag of the script. */
    SFTag _languageTag;             /**< Tag of the language. */

    SFInteger _retainCount;
} SFScheme;

#endif
