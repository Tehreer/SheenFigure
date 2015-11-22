/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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
#include <SFFeature.h>
#include <SFScript.h>
#include <SFTypes.h>

#include <stddef.h>

#include "SFShapingEngine.h"
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *obj, SFScript script);

static const SFFeature _SFArabicFeatureArray[] = {
    /* Language based forms */
    SFFeatureCcmp,
    SFFeatureIsol,
    SFFeatureFina,
    SFFeatureMedi,
    SFFeatureInit,
    SFFeatureRlig,
    SFFeatureCalt,
    /* Typographical forms */
    SFFeatureLiga,
    SFFeatureDlig,
    SFFeatureCswh,
    SFFeatureMset,
    /* Positioning features */
    SFFeatureCurs,
    SFFeatureKern,
    SFFeatureMark,
    SFFeatureMkmk
};
#define _SF_ARABIC_FEATURE_COUNT (sizeof(_SFArabicFeatureArray) / sizeof(SFFeature))

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    NULL,
    _SFArabicFeatureArray, NULL,
    _SF_ARABIC_FEATURE_COUNT, 0
};

const SFShapingKnowledge SFArabicKnowledgeInstance = {
    &_SFArabicKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *obj, SFScript script)
{
    switch (script) {
    case SFScriptARAB:
        return &_SFArabicScriptKnowledge;
    }

    return NULL;
}
