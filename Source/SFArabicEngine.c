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
#include <SFFeatureTag.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include <stddef.h>

#include "SFShapingEngine.h"
#include "SFArabicEngine.h"

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *obj, SFScriptTag scriptTag);

static const SFFeatureTag _SFArabicFeatureArray[] = {
    /* Language based forms */
    SFFeatureTagCCMP,
    SFFeatureTagISOL,
    SFFeatureTagFINA,
    SFFeatureTagMEDI,
    SFFeatureTagINIT,
    SFFeatureTagRLIG,
    SFFeatureTagCALT,
    /* Typographical forms */
    SFFeatureTagLIGA,
    SFFeatureTagDLIG,
    SFFeatureTagCSWH,
    SFFeatureTagMSET,
    /* Positioning features */
    SFFeatureTagCURS,
    SFFeatureTagKERN,
    SFFeatureTagMARK,
    SFFeatureTagMKMK
};
#define _SF_ARABIC_FEATURE_COUNT (sizeof(_SFArabicFeatureArray) / sizeof(SFFeatureTag))

static SFScriptKnowledge _SFArabicScriptKnowledge = {
    NULL,
    _SFArabicFeatureArray, NULL,
    _SF_ARABIC_FEATURE_COUNT, 0
};

const SFShapingKnowledge SFArabicKnowledgeInstance = {
    &_SFArabicKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFArabicKnowledgeSeekScript(const void *obj, SFScriptTag scriptTag)
{
    switch (scriptTag) {
    case SFScriptTagARAB:
        return &_SFArabicScriptKnowledge;
    }

    return NULL;
}
