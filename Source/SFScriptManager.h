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

#ifndef SF_SCRIPT_MANAGER_INTERNAL_H
#define SF_SCRIPT_MANAGER_INTERNAL_H

#include <SFConfig.h>
#include <SFTypes.h>

#include "SFData.h"
#include "SFKnowledge.h"
#include "SFScriptBuilder.h"

struct _SFScriptManager;
typedef struct _SFScriptManager SFScriptManager;
typedef SFScriptManager *SFScriptManagerRef;

typedef struct {
    SFData _records;                /**< OpenType tag record array. */
    SFData current;                 /**< Current record in the array. */
    SFUInteger _count;              /**< Number of items in the array. */
    SFUInteger _index;              /**< Current index of iterator. */
    SFTag tag;                      /**< Tag of current record. */
} _SFRecordIter;
typedef _SFRecordIter *_SFRecordIterRef;

typedef struct {
    _SFRecordIter gsubIter;         /**< Record iterator related to OpenType gsub table. */
    _SFRecordIter gposIter;         /**< Record iterator related to OpenType gpos table. */
    SFData gsubCurrent;             /**< Current record of gsub iterator or NULL */
    SFData gposCurrent;             /**< Current record of gpos iterator or NULL */
    SFTag tag;                      /**< Tag of current record. */
} _SFParallelIter;
typedef _SFParallelIter *_SFParallelIterRef;

typedef struct {
    SFData scriptList;              /**< OpenType script list table. */
    SFData featureList;             /**< OpenType feature list table. */
    SFData lookupList;              /**< OpenType lookup list table. */
} _SFHeaderDetail;
typedef _SFHeaderDetail *_SFHeaderDetailRef;

struct _SFScriptManager {
    SFScriptBuilderRef builder;
    SFKnowledge knowledge;
    _SFHeaderDetail gsub;
    _SFHeaderDetail gpos;
};

SF_INTERNAL void SFScriptManagerInitialize(SFScriptManagerRef scriptManager, SFScriptBuilderRef builder, SFData gsub, SFData gpos);

SF_INTERNAL void SFScriptManagerBuildCache(SFScriptManagerRef scriptManager);

SF_INTERNAL void SFScriptManagerFinalize(SFScriptManagerRef scriptManager);

#endif
