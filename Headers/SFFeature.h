/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SF_FEATURE_H
#define SF_FEATURE_H

#include "SFTypes.h"

/**
 * Referance: https://www.microsoft.com/typography/otspec/scripttags.htm
 */

enum {
    SFFeatureCalt = SFTagMake('c', 'a', 'l', 't'),
    SFFeatureCcmp = SFTagMake('c', 'c', 'm', 'p'),
    SFFeatureClig = SFTagMake('c', 'l', 'i', 'g'),
    SFFeatureCswh = SFTagMake('c', 's', 'w', 'h'),
    SFFeatureCurs = SFTagMake('c', 'u', 'r', 's'),
    SFFeatureDist = SFTagMake('d', 'i', 's', 't'),
    SFFeatureDlig = SFTagMake('d', 'l', 'i', 'g'),
    SFFeatureFina = SFTagMake('f', 'i', 'n', 'a'),
    SFFeatureInit = SFTagMake('i', 'n', 'i', 't'),
    SFFeatureIsol = SFTagMake('i', 's', 'o', 'l'),
    SFFeatureKern = SFTagMake('k', 'e', 'r', 'n'),
    SFFeatureLiga = SFTagMake('l', 'i', 'g', 'a'),
    SFFeatureMark = SFTagMake('m', 'a', 'r', 'k'),
    SFFeatureMedi = SFTagMake('m', 'e', 'd', 'i'),
    SFFeatureMkmk = SFTagMake('m', 'k', 'm', 'k'),
    SFFeatureMset = SFTagMake('m', 's', 'e', 't'),
    SFFeatureRlig = SFTagMake('r', 'l', 'i', 'g')
};
typedef SF_TAG SFFeature;

#endif
