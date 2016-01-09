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
#include <SFTypes.h>

#include <stddef.h>
#include <stdlib.h>

#include "SFPattern.h"

SF_INTERNAL SFPatternRef SFPatternCreate(void)
{
    SFPatternRef pattern = malloc(sizeof(SFPattern));
    pattern->font = NULL;
    pattern->featureTags.items = NULL;
    pattern->featureTags.count = 0;
    pattern->featureUnits.items = NULL;
    pattern->featureUnits.gsub = 0;
    pattern->featureUnits.gpos = 0;
    pattern->scriptTag = 0;
    pattern->languageTag = 0;
    pattern->_retainCount = 1;

    return pattern;
}

static void _SFFinalizeFeatureInfo(SFFeatureUnitRef featureUnit)
{
    free(featureUnit->lookupIndexes.items);
}

static void _SFPatternFinalize(SFPatternRef pattern)
{
    SFUInteger featureCount = pattern->featureUnits.gsub + pattern->featureUnits.gpos;
    SFUInteger index;

    /* Finalize all groups. */
    for (index = 0; index < featureCount; index++) {
        _SFFinalizeFeatureInfo((SFFeatureUnitRef)&pattern->featureUnits.items[index]);
    }

    free(pattern->featureUnits.items);
}

SFPatternRef SFPatternRetain(SFPatternRef pattern)
{
    if (pattern) {
        pattern->_retainCount++;
    }

    return pattern;
}

void SFPatternRelease(SFPatternRef pattern)
{
    if (pattern && --pattern->_retainCount == 0) {
        _SFPatternFinalize(pattern);
    }
}
