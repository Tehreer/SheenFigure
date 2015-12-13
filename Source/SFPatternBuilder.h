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

#ifndef SF_PATTERN_BUILDER_INTERNAL_H
#define SF_PATTERN_BUILDER_INTERNAL_H

#include <SFConfig.h>
#include <SFFeature.h>
#include <SFLanguage.h>
#include <SFScript.h>
#include <SFTypes.h>

#include "SFList.h"
#include "SFPattern.h"

/**
 * ScriptBuilder is an inner class of ScriptCache, therefore it can access private members of
 * ScriptCache.
 */

struct _SFPatternBuilder;
typedef struct _SFPatternBuilder SFPatternBuilder;
typedef SFPatternBuilder *SFPatternBuilderRef;

/**
 * Provides a way to cache scripts, languages, features and lookups.
 */
struct _SFPatternBuilder {
    SFFeatureGroupRef _currentGroup;    /**< Reference to the feature group being built. */
    SFHeaderKind _currentHeader;        /**< The kind of table whose features are being added. */
    SFUInteger _gsubGroupCount;
    SFUInteger _gposGroupCount;
    SFUInteger _featureIndex;
    SFScript _script;                   /**< Tag of the script. */
    SFLanguage _language;               /**< Tag of the language. */

    SF_LIST(SFFeature) _featureTags;
    SF_LIST(SFFeatureGroup) _featureGroups;
    SF_LIST(SFUInt16) _lookupIndexes;
};

/**
 * Initializes builder for a script cache.
 * @param scriptCache
 *      The script cache to be built. It must be an uninitialized object as it
 *		will be manipulated by the builder.
 */
SF_INTERNAL void SFPatternBuilderInitialize(SFPatternBuilderRef builder);

SF_INTERNAL void SFPatternBuilderSetScript(SFPatternBuilderRef builder, SFScript script);
SF_INTERNAL void SFPatternBuilderSetLanguage(SFPatternBuilderRef builder, SFLanguage language);

SF_INTERNAL void SFPatternBuilderBeginHeader(SFPatternBuilderRef builder, SFHeaderKind kind);

/**
 * Adds a new feature for specified header.
 */
SF_INTERNAL void SFPatternBuilderAddFeature(SFPatternBuilderRef builder, SFFeature feature, SFHeaderKind kind);

/**
 * Adds a new feature group for specified header.
 */
SF_INTERNAL void SFPatternBuilderAddGroup(SFPatternBuilderRef builder, SFUInteger featureCount);

/**
 * Adds a new lookup in recently added feature group.
 */
SF_INTERNAL void SFPatternBuilderAddLookup(SFPatternBuilderRef builder, SFUInt16 lookupIndex);

/**
 * Closes recently added feature group.
 */
SF_INTERNAL void SFPatternBuilderCloseGroup(SFPatternBuilderRef builder);

SF_INTERNAL void SFPatternBuilderBuild(SFPatternBuilderRef builder, SFPatternRef pattern);

SF_INTERNAL void SFPatternBuilderFinalize(SFPatternBuilderRef builder);

#endif
