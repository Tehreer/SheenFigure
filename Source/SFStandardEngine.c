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
#include "SFStandardEngine.h"

static const SFFeatureTag _SFStandardFeatureArray[] = {
    /* Language based forms. */
    SFFeatureTagCCMP,
    /* Typographical forms */
    SFFeatureTagLIGA,
    SFFeatureTagCLIG,
    /* Positioning features. */
    SFFeatureTagDIST,
    SFFeatureTagKERN,
    SFFeatureTagMARK,
    SFFeatureTagMKMK
};
#define _SF_STANDARD_FEATURE_COUNT (sizeof(_SFStandardFeatureArray) / sizeof(SFFeatureTag))

static const SFScriptKnowledge _SFStandardScriptKnowledge = {
    NULL,
    _SFStandardFeatureArray, NULL,
    _SF_STANDARD_FEATURE_COUNT, 0
};

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *obj, SFScriptTag scriptTag);

const SFShapingKnowledge SFStandardKnowledgeInstance = {
    &_SFStandardKnowledgeSeekScript
};

static SFScriptKnowledgeRef _SFStandardKnowledgeSeekScript(const void *obj, SFScriptTag scriptTag)
{
    switch (scriptTag) {
    case SFScriptTagARMN:
    case SFScriptTagCYRL:
    case SFScriptTagGEOR:
    case SFScriptTagGREK:
    case SFScriptTagLATN:
    case SFScriptTagOGAM:
    case SFScriptTagRUNR:
        return &_SFStandardScriptKnowledge;
    }

    return NULL;
}
