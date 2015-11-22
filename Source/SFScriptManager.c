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
#include <SFTypes.h>

#include <stddef.h>

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFData.h"
#include "SFGSUB.h"
#include "SFGPOS.h"
#include "SFKnowledge.h"
#include "SFScriptBuilder.h"
#include "SFScriptManager.h"

static void _SFRecordIterInitialize(_SFRecordIterRef recordIter, SFData recordArray, SFUInt16 recordCount);
static SFBoolean _SFRecordIterMoveNext(_SFRecordIterRef recordIter);

static void _SFParallelIterInitialize(_SFParallelIterRef parallelIter);
static SFBoolean _SFParallelIterMoveNext(_SFParallelIterRef parallelIter);

static void _SFInitializeHeaderDetail(_SFHeaderDetailRef state, SFData header);
static SFUInt16 _SFInitializeScriptIter(_SFRecordIterRef iter, SFData scriptList);
static SFUInt16 _SFInitializeLanguageIter(_SFRecordIterRef iter, SFData script, SFData *defaultLangSys);

static void _SFInsertAllLanguages(SFScriptManagerRef manager, SFData gsubScript, SFData gposScript);
static void _SFInsertLanguage(SFScriptManagerRef manager, SFData gsubLangSys, SFData gposLangSys, SFTag tag);
static void _SFInsertAllFeatures(SFScriptManagerRef manager, _SFHeaderDetailRef detail, SFData langSys, SFHeaderKind kind);
static void _SFInsertAllLookups(SFScriptManagerRef manager, _SFHeaderDetailRef detail, SFData feature);

static void _SFRecordIterInitialize(_SFRecordIterRef recordIter, SFData recordArray, SFUInt16 recordCount)
{
    SFAssert((recordArray && recordCount) || (!recordArray && !recordCount));

    recordIter->_records = recordArray;
    recordIter->current = NULL;
    recordIter->_count = recordCount;
    recordIter->_index = 0;
    recordIter->tag = SFTagInvalid;
}

static SFBoolean _SFRecordIterMoveNext(_SFRecordIterRef recordIter)
{
    if (recordIter->_index < recordIter->_count) {
        SFData record = SF_TAG_RECORD_ARRAY__VALUE(recordIter->_records, recordIter->_index);
        SFTag tag = SF_TAG_RECORD__TAG(record);

        recordIter->current = record;
        recordIter->tag = tag;
        recordIter->_index++;

        return SFTrue;
    }

    recordIter->current = NULL;
    recordIter->tag = SFTagInvalid;
    recordIter->_index = SFInvalidIndex;

    return SFFalse;
}

/**
 * @note
 *      Inner iterators must be initialized manually before calling this method.
 */
static void _SFParallelIterInitialize(_SFParallelIterRef parallelIter)
{
    /*
     * Inner iterators must be one record ahead. This way parallel iterator will be able to compare
     * tags of both iterators and determine which record/s should come next.
     */
    _SFRecordIterMoveNext(&parallelIter->gsubIter);
    _SFRecordIterMoveNext(&parallelIter->gposIter);

    /* Set empty state. */
    parallelIter->gsubCurrent = NULL;
    parallelIter->gposCurrent = NULL;
    parallelIter->tag = SFTagInvalid;
}

static SFBoolean _SFParallelIterMoveNext(_SFParallelIterRef parallelIter)
{
    SFTag gsubRecordTag = parallelIter->gsubIter.tag;
    SFTag gposRecordTag = parallelIter->gposIter.tag;

    if (gsubRecordTag != SFTagInvalid || gposRecordTag != SFTagInvalid) {
        if (gsubRecordTag == gposRecordTag) {
            /* Save current records of both iterators and advance these to next record. */
            parallelIter->gsubCurrent = parallelIter->gsubIter.current;
            parallelIter->gposCurrent = parallelIter->gposIter.current;
            parallelIter->tag = gsubRecordTag;
            _SFRecordIterMoveNext(&parallelIter->gsubIter);
            _SFRecordIterMoveNext(&parallelIter->gposIter);
        } else if (gsubRecordTag < gposRecordTag) {
            /* Save current record of gsub iterator and advance it to next record. */
            parallelIter->gsubCurrent = parallelIter->gsubIter.current;
            parallelIter->gposCurrent = NULL;
            parallelIter->tag = gsubRecordTag;
            _SFRecordIterMoveNext(&parallelIter->gsubIter);
        } else {
            /* Save current record of gpos iterator and advance it to next record. */
            parallelIter->gsubCurrent = NULL;
            parallelIter->gposCurrent = parallelIter->gposIter.current;
            parallelIter->tag = gposRecordTag;
            _SFRecordIterMoveNext(&parallelIter->gposIter);
        }

        return SFTrue;
    }

    return SFFalse;
}

static void _SFInitializeHeaderDetail(_SFHeaderDetailRef state, SFData header)
{
    if (header) {
        SFOffset offset;

        /* Get script list table. */
        offset = SF_HEADER__SCRIPT_LIST(header);
        state->scriptList = SF_DATA__SUBDATA(header, offset);

        /* Get feature list table. */
        offset = SF_HEADER__FEATURE_LIST(header);
        state->featureList = SF_DATA__SUBDATA(header, offset);

        /* Get lookup list table. */
        offset = SF_HEADER__LOOKUP_LIST(header);
        state->lookupList = SF_DATA__SUBDATA(header, offset);
    } else {
        state->scriptList = NULL;
        state->featureList = NULL;
        state->lookupList = NULL;
    }
}

static SFUInt16 _SFInitializeScriptIter(_SFRecordIterRef iter, SFData scriptList)
{
    if (scriptList) {
        SFUInt16 scriptCount = SF_SCRIPT_LIST__SCRIPT_COUNT(scriptList);

        if (scriptCount) {
            SFData recordArray = SF_SCRIPT_LIST__TAG_RECORD_ARRAY(scriptList);
            _SFRecordIterInitialize(iter, recordArray, scriptCount);

            return scriptCount;
        }
    }

    /* Script list table is not available, so make the iter empty. */
    _SFRecordIterInitialize(iter, NULL, 0);

    return 0;
}

static SFUInt16 _SFInitializeLanguageIter(_SFRecordIterRef iter, SFData script, SFData *defaultLangSys)
{
    *defaultLangSys = NULL;

    if (script) {
        SFOffset offset = SF_SCRIPT__DEFAULT_LANG_SYS(script);
        SFUInt16 langSysCount = SF_SCRIPT__LANG_SYS_COUNT(script);

        if (offset) {
            *defaultLangSys = SF_DATA__SUBDATA(script, offset);
        }

        if (langSysCount) {
            SFData recordArray = SF_SCRIPT__TAG_RECORD_ARRAY(script);
            _SFRecordIterInitialize(iter, recordArray, langSysCount);

            return langSysCount;
        }
    }

    /* Script table is not available, so make the iter empty. */
    _SFRecordIterInitialize(iter, NULL, 0);

    return 0;
}

SF_INTERNAL void SFScriptManagerInitialize(SFScriptManagerRef scriptManager, SFScriptBuilderRef builder, SFData gsub, SFData gpos)
{
    /* Save builder. */
    scriptManager->builder = builder;

    /* Initialize headers details. */
    _SFInitializeHeaderDetail(&scriptManager->gsub, gsub);
    _SFInitializeHeaderDetail(&scriptManager->gpos, gpos);

    /* Initialize knowledge manager. */
    SFKnowledgeInitialize(&scriptManager->knowledge);
}

SF_INTERNAL void SFScriptManagerBuildCache(SFScriptManagerRef manager)
{
    _SFParallelIter scriptIter;
    SFUInteger scriptCount;

    /* Initialize inner script iterators. */
    scriptCount = _SFInitializeScriptIter(&scriptIter.gsubIter, manager->gsub.scriptList);
    scriptCount += _SFInitializeScriptIter(&scriptIter.gposIter, manager->gpos.scriptList);

    if (scriptCount) {
        /* Initialize parallel script iterator. */
        _SFParallelIterInitialize(&scriptIter);

        /* Reserve estimated number of scripts in builder. */
        SFScriptBuilderReserveScripts(manager->builder, scriptCount);

        while (_SFParallelIterMoveNext(&scriptIter)) {
            /* Check whether Sheen Figure has knowledge about this script. */
            if (SFKnowledgeSeekScript(&manager->knowledge, scriptIter.tag)) {
                SFData gsubScript = NULL;
                SFData gposScript = NULL;

                if (scriptIter.gsubCurrent) {
                    SFOffset offset = SF_SCRIPT_RECORD__SCRIPT(scriptIter.gsubCurrent);
                    gsubScript = SF_DATA__SUBDATA(manager->gsub.scriptList, offset);
                }

                if (scriptIter.gposCurrent) {
                    SFOffset offset = SF_SCRIPT_RECORD__SCRIPT(scriptIter.gposCurrent);
                    gposScript = SF_DATA__SUBDATA(manager->gpos.scriptList, offset);
                }

                /* Add current script in builder. */
                SFScriptBuilderAddScript(manager->builder, scriptIter.tag);
                /* Insert all languages of current script. */
                _SFInsertAllLanguages(manager, gsubScript, gposScript);
            }
        }
    }
}

static void _SFInsertAllLanguages(SFScriptManagerRef manager, SFData gsubScript, SFData gposScript)
{
    SFData gsubLangSys;
    SFData gposLangSys;
    SFUInteger languageCount;
    _SFParallelIter languageIter;

    languageCount = _SFInitializeLanguageIter(&languageIter.gsubIter, gsubScript, &gsubLangSys);
    languageCount += _SFInitializeLanguageIter(&languageIter.gposIter, gposScript, &gposLangSys);

    /* Insert default language. */
    _SFInsertLanguage(manager, gsubLangSys, gposLangSys, SFLanguageDflt);

    if (languageCount) {
        /* Initialize language iterator. */
        _SFParallelIterInitialize(&languageIter);

        /* Reserve estimated number of languages in builder. */
        SFScriptBuilderReserveLanguages(manager->builder, languageCount);

        while (_SFParallelIterMoveNext(&languageIter)) {
            gsubLangSys = NULL;
            gposLangSys = NULL;

            if (languageIter.gsubCurrent) {
                SFOffset offset = SF_LANG_SYS_RECORD__LANG_SYS(languageIter.gsubCurrent);
                gsubLangSys = SF_DATA__SUBDATA(gsubScript, offset);
            }

            if (languageIter.gposCurrent) {
                SFOffset offset = SF_LANG_SYS_RECORD__LANG_SYS(languageIter.gposCurrent);
                gposLangSys = SF_DATA__SUBDATA(gposScript, offset);
            }

            /* Insert current language. */
            _SFInsertLanguage(manager, gsubLangSys, gposLangSys, languageIter.tag);
        }
    }
}

static void _SFInsertLanguage(SFScriptManagerRef manager, SFData gsubLangSys, SFData gposLangSys, SFTag tag)
{
    SFUInteger featureCount = SFKnowledgeCountFeatures(&manager->knowledge);
    SFUInteger groupCount = SFKnowledgeCountGroups(&manager->knowledge);
    SFUInteger index;

    /* Add the language in the builder. */
    SFScriptBuilderAddLanguage(manager->builder, tag);
    /* Reserve features for the language. */
    SFScriptBuilderReserveFeatures(manager->builder, featureCount);

    if (groupCount) {
        /* Reserve groups for the language. */
        SFScriptBuilderReserveGroups(manager->builder, groupCount);

        /* Add all groups in the builder. */
        for (index = 0; index < groupCount; index++) {
            SFRange featureRange = SFKnowledgeSeekGroup(&manager->knowledge, index);
            SFScriptBiulderAddGroup(manager->builder, featureRange.start, featureRange.length);
        }
    }

    if (gsubLangSys) {
        _SFInsertAllFeatures(manager, &manager->gsub, gsubLangSys, SFHeaderKindGSUB);
    }
    if (gposLangSys) {
        _SFInsertAllFeatures(manager, &manager->gpos, gposLangSys, SFHeaderKindGPOS);
    }
}

static void _SFInsertAllFeatures(SFScriptManagerRef manager, _SFHeaderDetailRef detail, SFData langSys, SFHeaderKind kind)
{
    SFUInteger featureCount = SF_LANG_SYS__FEATURE_COUNT(langSys);
    SFUInteger index;

    for (index = 0; index < featureCount; index++) {
        SFUInt16 featureIndex = SF_LANG_SYS__FEATURE_INDEX(langSys, index);
        SFData featureRecord = SF_FEATURE_LIST__FEATURE_RECORD(detail->featureList, featureIndex);
        SFTag featureTag = SF_FEATURE_RECORD__FEATURE_TAG(featureRecord);
        SFUInteger orderIndex;

        /* Check whether Sheen Figure has knowledge of current feature. */
        orderIndex = SFKnowledgeSeekFeature(&manager->knowledge, featureTag);
        if (orderIndex != SFInvalidIndex) {
            SFOffset offset = SF_FEATURE_RECORD__FEATURE(featureRecord);
            SFData feature = SF_DATA__SUBDATA(detail->featureList, offset);

            /* Add current feature in the builder. */
            SFScriptBuilderAddFeature(manager->builder, featureTag, kind, orderIndex);
            /* Insert all lookups of current feature. */
            _SFInsertAllLookups(manager, detail, feature);
        }
    }
}

static void _SFInsertAllLookups(SFScriptManagerRef manager, _SFHeaderDetailRef detail, SFData feature)
{
    SFUInteger totalLookups = SF_LOOKUP_LIST__LOOKUP_COUNT(detail->lookupList);
    SFUInteger lookupCount = SF_FEATURE__LOOKUP_COUNT(feature);
    SFUInteger index;

    /* Reserve lookups for the feature. */
    SFScriptBuilderReserveLookups(manager->builder, totalLookups);

    /* Add all lookups in the builder. */
    for (index = 0; index < lookupCount; index++) {
        SFUInt16 lookupIndex = SF_FEATURE__LOOKUP_LIST_INDEX(feature, index);
        SFOffset offset = SF_LOOKUP_LIST__LOOKUP(detail->lookupList, index);
        SFData lookup = SF_DATA__SUBDATA(detail->lookupList, offset);
        SFScriptBuilderAddLookup(manager->builder, lookupIndex, lookup);
    }
}

SF_INTERNAL void SFScriptManagerInvalidate(SFScriptManagerRef scriptManager)
{
}
