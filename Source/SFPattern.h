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

#ifndef SF_PATTERN_INTERNAL_H
#define SF_PATTERN_INTERNAL_H

#include <SFConfig.h>
#include <SFFeatureTag.h>
#include <SFFont.h>
#include <SFLanguageTag.h>
#include <SFPattern.h>
#include <SFScriptTag.h>
#include <SFTypes.h>

struct _SFPattern;
struct _SFFeatureGroup;

typedef struct _SFPattern SFPattern;
typedef struct _SFFeatureGroup SFFeatureGroup;

typedef SFPattern *SFPatternRef;
typedef SFFeatureGroup *SFFeatureGroupRef;

/**
 * A type to represent the kind of an OpenType feature.
 */
typedef enum {
    SFHeaderKindGSUB = 0x01, /**< A value indicating that the feature belongs to 'GSUB' table. */
    SFHeaderKindGPOS = 0x02  /**< A value indicating that the feature belongs to 'GPOS' table. */
} SFHeaderKind;

/**
 * Keeps details of a group having multiple features which must be applied simultaneously.
 */
struct _SFFeatureGroup {
    SFUInt16 *lookupIndexes;    /**< Unique lookup indexes of all features of the group in ascending order. */
    SFUInteger lookupCount;     /**< Total number of lookups in the group. */
    SFUInteger featureIndex;    /**< Starting index of feature covered in the group. */
    SFUInteger featureCount;    /**< Total number of features belonging to the group. */
    SFHeaderKind headerKind;    /**< Kind of feature's header table. */
};

/**
 * Keeps details of a script from both 'GSUB' and 'GPOS' tables.
 */
struct _SFPattern {
    SFFontRef font;
    SFFeatureTag *featureTagArray;     /**< Tags of features. */
    SFFeatureGroup *featureGroupArray; /**<  Array of feature groups in implementation order. Substitution features will be followed by positioning features. */
    struct {
        SFUInteger gsub;           /**< Total number of gsub feature groups.*/
        SFUInteger gpos;	       /**< Total number of gpos feature groups.*/
    } groupCount;
    SFUInteger featureCount;       /**< Total number of features. */
    SFScriptTag scriptTag;         /**< Tag of the script. */
    SFLanguageTag languageTag;     /**< Tag of the language. */

    SFUInteger _retainCount;
};

SF_INTERNAL SFPatternRef SFPatternCreate(void);

#endif
