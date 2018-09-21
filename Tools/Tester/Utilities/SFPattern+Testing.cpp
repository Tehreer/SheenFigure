/*
 * Copyright (C) 2016-2018 Muhammad Tayyab Akram
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

#include <cstring>

#include "SFPattern+Testing.h"

static SFBoolean SFFeatureUnitListEqualToList(SFFeatureUnit *list1, SFFeatureUnit *list2, SFUInteger count)
{
    for (SFUInteger i = 0; i < count; i++) {
        SFFeatureUnitRef unit1 = &list1[i];
        SFFeatureUnitRef unit2 = &list2[i];

        if (!(unit1->mask == unit2->mask
              && unit1->range.start == unit2->range.start
              && unit1->range.count == unit2->range.count
              && unit1->lookups.count == unit2->lookups.count
              && memcmp(unit1->lookups.items,
                        unit2->lookups.items,
                        unit1->lookups.count * sizeof(SFLookupInfo)) == 0)) {
                  return SFFalse;
              }
    }

    return SFTrue;
}

SFBoolean SFPatternEqualToPattern(SFPatternRef pattern1, SFPatternRef pattern2)
{
    return (pattern1->font == pattern2->font
            && pattern1->scriptTag == pattern2->scriptTag
            && pattern1->languageTag == pattern2->languageTag
            && pattern1->defaultDirection == pattern2->defaultDirection
            && pattern1->featureTags.count == pattern2->featureTags.count
            && memcmp(pattern1->featureTags.items,
                      pattern2->featureTags.items,
                      pattern1->featureTags.count * sizeof(SFTag)) == 0
            && pattern1->featureUnits.gsub == pattern2->featureUnits.gsub
            && pattern1->featureUnits.gpos == pattern2->featureUnits.gpos
            && SFFeatureUnitListEqualToList(pattern1->featureUnits.items,
                                            pattern2->featureUnits.items,
                                            pattern1->featureUnits.gsub + pattern1->featureUnits.gpos));
}
