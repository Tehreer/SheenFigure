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
#include <SFLanguage.h>
#include <SFScript.h>
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SFScriptCache.h"

/*
 * Finds only non-default language.
 */
static SFLanguageDetailRef _SFFindLanguageInScript(SFScriptDetailRef scriptDetail, SFLanguage language) {
    SFLanguageDetail *langArray = scriptDetail->languageArray;
    SFUInteger langCount = scriptDetail->languageCount;
    SFUInteger langIndex;

    for (langIndex = 0; langIndex < langCount; langIndex++) {
        SFLanguageDetailRef langDetail = &langArray[langIndex];

        if (langDetail->language == language) {
            return langDetail;
        }
    }

    return NULL;
}

SF_INTERNAL SFLanguageDetailRef SFScriptCacheFindLanguage(SFScriptCacheRef scriptCache, SFScript script, SFLanguage language) {
    SFScriptDetail *scriptArray = scriptCache->scriptArray;
    SFUInteger scriptCount = scriptCache->scriptCount;
    SFUInteger scriptIndex;

    for (scriptIndex = 0; scriptIndex < scriptCount; scriptIndex++) {
        SFScriptDetailRef scriptDetail = &scriptArray[scriptIndex];

        if (scriptDetail->script == script) {
            if (language == SFLanguageDflt) {
                return scriptDetail->defaultLanguage;
            }

            return _SFFindLanguageInScript(scriptDetail, language);
        }
    }

    return NULL;
}

static void _SFInvalidateGroup(SFGroupDetailRef group)
{
    free(group->lookupArray);
}

static void _SFInvalidateFeature(SFFeatureDetailRef feature)
{
    free(feature->lookupArray);
}

static void _SFInvalidateLanguage(SFLanguageDetailRef language)
{
    SFUInteger index;

    /* Invalidate all features. */
    for (index = 0; index < language->featureCount; index++) {
        _SFInvalidateFeature(&language->featureArray[index]);
    }

    /* Invalidate all groups. */
    for (index = 0; index < language->groupCount; index++) {
        _SFInvalidateGroup(&language->groupArray[index]);
    }
}

static void _SFInvalidateScript(SFScriptDetailRef script)
{
    SFUInteger index;

    /* Invalidate all languages. */
    for (index = 0; index < script->languageCount; index++) {
        _SFInvalidateLanguage(&script->languageArray[index]);
    }
}

SF_INTERNAL void SFScriptCacheInvalidate(SFScriptCacheRef scriptCache)
{
    SFUInteger index;

    /* Invalidate scripts. */
    for (index = 0; index < scriptCache->scriptCount; index++) {
        _SFInvalidateScript(&scriptCache->scriptArray[index]);
    }
}
