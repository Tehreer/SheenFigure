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

#ifndef _SF_INTERNAL_PATTERN_H
#define _SF_INTERNAL_PATTERN_H

#include <SFConfig.h>
#include <SFPattern.h>

#include "SFArtist.h"
#include "SFBase.h"
#include "SFFont.h"

enum {
    SFFeatureKindSubstitution = 0x01, /**< A value indicating that the feature belongs to 'GSUB' table. */
    SFFeatureKindPositioning = 0x02   /**< A value indicating that the feature belongs to 'GPOS' table. */
};
typedef SFUInt8 SFFeatureKind;

typedef struct _SFLookupInfo {
    SFUInt16 index;
    SFUInt16 value;
} SFLookupInfo, *SFLookupInfoRef;

/**
 * Keeps details of a group having multiple features which must be applied simultaneously.
 */
typedef struct _SFFeatureUnit {
    /**
     * Unique lookup indexes of all features belonging to the unit in ascending order.
     */
    struct {
        SFLookupInfo *items;
        SFUInteger count;
    } lookups;
    SFRange range;
    SFUInt16 mask;
} SFFeatureUnit, *SFFeatureUnitRef;

/**
 * Keeps details of a script from both 'GSUB' and 'GPOS' tables.
 */
typedef struct _SFPattern {
    SFFontRef font;
    struct {
        SFTag *items;
        SFUInteger count;               /**< Total number of features. */
    } featureTags;
    struct {
        SFFeatureUnit *items;           /**< Items of feature units, gsub, followed by gspos. */
        SFUInteger gsub;                /**< Total number of gsub feature units. */
        SFUInteger gpos;                /**< Total number of gpos feature units. */
    } featureUnits;
    SFTag scriptTag;                    /**< Tag of the script. */
    SFTag languageTag;                  /**< Tag of the language. */
    SFTextDirection defaultDirection;   /**< Default direction of the script. */
    SFUInteger _retainCount;
} SFPattern;

SF_INTERNAL SFPatternRef SFPatternCreate(void);

#endif
