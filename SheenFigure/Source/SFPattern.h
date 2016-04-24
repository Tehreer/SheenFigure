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

#ifndef _SF_PATTERN_INTERNAL_H
#define _SF_PATTERN_INTERNAL_H

#include <SFConfig.h>
#include <SFFeatureTag.h>
#include <SFLanguageTag.h>
#include <SFPattern.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

#include "SFArtist.h"
#include "SFFont.h"

enum {
    SFFeatureKindSubstitution = 0x01, /**< A value indicating that the feature belongs to 'GSUB' table. */
    SFFeatureKindPositioning = 0x02   /**< A value indicating that the feature belongs to 'GPOS' table. */
};
typedef SFUInt8 SFFeatureKind;

/**
 * Keeps details of a group having multiple features which must be applied simultaneously.
 */
typedef struct _SFFeatureUnit {
    /**
     * Unique lookup indexes of all features belonging to the unit in ascending order.
     */
    struct {
        SFUInt16 *items;
        SFUInteger count;
    } lookupIndexes;
    SFRange coveredRange;
    SFUInt16 featureMask;
} SFFeatureUnit, *SFFeatureUnitRef;

/**
 * Keeps details of a script from both 'GSUB' and 'GPOS' tables.
 */
struct _SFPattern {
    SFFontRef font;
    struct {
        SFFeatureTag *items;
        SFUInteger count;           /**< Total number of features.*/
    } featureTags;
    struct {
        SFFeatureUnit *items;
        SFUInteger gsub;            /**< Total number of gsub feature groups.*/
        SFUInteger gpos;            /**< Total number of gpos feature groups.*/
    } featureUnits;
    SFScriptTag scriptTag;          /**< Tag of the script. */
    SFTextFlow textFlow;            /**< Natural text flow of the script. */
    SFLanguageTag languageTag;      /**< Tag of the language. */
    SFUInteger _retainCount;
};

SF_INTERNAL SFPatternRef SFPatternCreate(void);

#endif
