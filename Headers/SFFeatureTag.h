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

#ifndef SF_FEATURE_TAG_H
#define SF_FEATURE_TAG_H

#include "SFTypes.h"

/**
 * Referance: https://www.microsoft.com/typography/otspec/scripttags.htm
 */

enum {
    SFFeatureTagCALT = SFTagMake('c', 'a', 'l', 't'),
    SFFeatureTagCCMP = SFTagMake('c', 'c', 'm', 'p'),
    SFFeatureTagCLIG = SFTagMake('c', 'l', 'i', 'g'),
    SFFeatureTagCSWH = SFTagMake('c', 's', 'w', 'h'),
    SFFeatureTagCURS = SFTagMake('c', 'u', 'r', 's'),
    SFFeatureTagDIST = SFTagMake('d', 'i', 's', 't'),
    SFFeatureTagDLIG = SFTagMake('d', 'l', 'i', 'g'),
    SFFeatureTagFINA = SFTagMake('f', 'i', 'n', 'a'),
    SFFeatureTagINIT = SFTagMake('i', 'n', 'i', 't'),
    SFFeatureTagISOL = SFTagMake('i', 's', 'o', 'l'),
    SFFeatureTagKERN = SFTagMake('k', 'e', 'r', 'n'),
    SFFeatureTagLIGA = SFTagMake('l', 'i', 'g', 'a'),
    SFFeatureTagMARK = SFTagMake('m', 'a', 'r', 'k'),
    SFFeatureTagMEDI = SFTagMake('m', 'e', 'd', 'i'),
    SFFeatureTagMKMK = SFTagMake('m', 'k', 'm', 'k'),
    SFFeatureTagMSET = SFTagMake('m', 's', 'e', 't'),
    SFFeatureTagRLIG = SFTagMake('r', 'l', 'i', 'g')
};
typedef SF_TAG SFFeatureTag;

#endif
