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

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

#include "Builder.h"

using namespace std;
using namespace SheenFigure::Tester::OpenType;

Builder::Builder()
    : m_pool()
{
}

Builder::~Builder()
{
}

template<class T, class... Args>
T &Builder::createObject(Args&&... args)
{
    shared_ptr<T> object = make_shared<T>(forward<Args>(args)...);
    m_pool.push_back(object);

    return *object;
}

template<class T>
T *Builder::createArray(size_t size)
{
    return (size == 0 ? nullptr : &createObject<vector<T>>(size)[0]);
}

template<class InputIt, class Operation>
Glyph *Builder::createGlyphs(InputIt begin, InputIt end, Operation operation)
{
    vector<Glyph> &glyphs = createObject<vector<Glyph>>();

    while (begin != end) {
        glyphs.push_back(operation(*begin++));
    }

    return (glyphs.size() == 0 ? nullptr : glyphs.data());
}

template<class Collection>
Glyph *Builder::createGlyphs(const Collection &glyphs)
{
    return createGlyphs(glyphs.begin(), glyphs.end(),
                        [](Glyph glyph) { return glyph; });
}

static void fillTagChars(UInt8 *chars, UInt32 tag)
{
    chars[0] = (tag >> 24) & 0xFF;
    chars[1] = (tag >> 16) & 0xFF;
    chars[2] = (tag >> 8) & 0xFF;
    chars[3] = tag & 0xFF;
}

GSUB &Builder::createGSUB(ScriptListTable *scriptList, FeatureListTable *featureList, LookupListTable *lookupList)
{
    GSUB &gsub = createObject<GSUB>();
    gsub.version = 0x00010000;
    gsub.scriptList = scriptList;
    gsub.featureList = featureList;
    gsub.lookupList = lookupList;

    return gsub;
}

GPOS &Builder::createGPOS(ScriptListTable *scriptList, FeatureListTable *featureList, LookupListTable *lookupList)
{
    GPOS &gpos = createObject<GPOS>();
    gpos.version = 0x00010000;
    gpos.scriptList = scriptList;
    gpos.featureList = featureList;
    gpos.lookupList = lookupList;

    return gpos;
}

LangSysTable &Builder::createLangSys(const vector<UInt16> features, UInt16 req)
{
    LangSysTable &langSys = createObject<LangSysTable>();
    langSys.lookupOrder = 0;
    langSys.reqFeatureIndex = req;
    langSys.featureCount = (UInt16)features.size();
    langSys.featureIndex = createArray<UInt16>(features.size());

    for (size_t i = 0; i < features.size(); i++) {
        langSys.featureIndex[i] = features[i];
    }

    return langSys;
}

ScriptTable &Builder::createScript(reference_wrapper<LangSysTable> defaultLangSys,
    const map<UInt32, reference_wrapper<LangSysTable>> records)
{
    ScriptTable &script = createObject<ScriptTable>();
    script.defaultLangSys = &defaultLangSys.get();
    script.langSysCount = (UInt16)records.size();
    script.langSysRecord = createArray<LangSysRecord>(records.size());

    size_t index = 0;

    for (const auto &pair : records) {
        LangSysRecord &langSysRecord = script.langSysRecord[index];
        fillTagChars(langSysRecord.langSysTag, pair.first);
        langSysRecord.langSys = &pair.second.get();

        index++;
    }

    return script;
}

ScriptListTable &Builder::createScriptList(const map<UInt32, reference_wrapper<ScriptTable>> records)
{
    ScriptListTable &scriptList = createObject<ScriptListTable>();
    scriptList.scriptCount = (UInt16)records.size();
    scriptList.scriptRecord = createArray<ScriptRecord>(records.size());

    size_t index = 0;

    for (const auto &pair : records) {
        ScriptRecord &scriptRecord = scriptList.scriptRecord[index];
        fillTagChars(scriptRecord.scriptTag, pair.first);
        scriptRecord.script = &pair.second.get();

        index++;
    }

    return scriptList;
}

FeatureTable &Builder::createFeature(const vector<UInt16> lookups, UInt16 params)
{
    FeatureTable &feature = createObject<FeatureTable>();
    feature.featureParams = params;
    feature.lookupCount = (UInt16)lookups.size();
    feature.lookupListIndex = createArray<UInt16>(lookups.size());

    for (size_t j = 0; j < lookups.size(); j++) {
        feature.lookupListIndex[j] = lookups[j];
    }

    return feature;
}

FeatureListTable &Builder::createFeatureList(const map<UInt32, reference_wrapper<FeatureTable>> records)
{
    FeatureListTable &featureList = createObject<FeatureListTable>();
    featureList.featureCount = (UInt16)records.size();
    featureList.featureRecord = createArray<FeatureRecord>(records.size());

    size_t index = 0;

    for (const auto &pair : records) {
        FeatureRecord &record = featureList.featureRecord[index];
        fillTagChars(record.featureTag, pair.first);
        record.feature = &pair.second.get();

        index++;
    }

    return featureList;
}

LookupTable &Builder::createLookup(const pair<LookupSubtable *, UInt16> subtables,
                                   LookupFlag lookupFlag, UInt16 markFilteringSet)
{
    LookupTable &lookup = createObject<LookupTable>();
    lookup.lookupType = (LookupType)0;
    lookup.lookupFlag = lookupFlag;
    lookup.subTableCount = subtables.second;
    lookup.subtables = subtables.first;
    lookup.markFilteringSet = markFilteringSet;

    if (lookup.subTableCount > 0) {
        lookup.lookupType = lookup.subtables->lookupType();
    }

    return lookup;
}

LookupListTable &Builder::createLookupList(const vector<reference_wrapper<LookupTable>> lookups)
{
    LookupListTable &lookupList = createObject<LookupListTable>();
    lookupList.lookupCount = (UInt16)lookups.size();
    lookupList.lookupTables = createArray<LookupTable>(lookups.size());

    for (size_t i = 0; i < lookups.size(); i++) {
        lookupList.lookupTables[i] = lookups[i].get();
    }

    return lookupList;
}

static void initCoverage(CoverageTable &coverage, Glyph *glyphs, UInt16 count)
{
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = count;
    coverage.format1.glyphArray = glyphs;
}

CoverageTable &Builder::createCoverage(Glyph *glyphs, UInt16 count)
{
    CoverageTable &coverage = createObject<CoverageTable>();
    initCoverage(coverage, glyphs, count);

    return coverage;
}

ClassDefTable &Builder::createClassDef(Glyph startGlyph, UInt16 glyphCount, const vector<UInt16> classValues)
{
    ClassDefTable &classDef = createObject<ClassDefTable>();
    classDef.classFormat = 1;
    classDef.format1.startGlyph = startGlyph;
    classDef.format1.glyphCount = glyphCount;
    classDef.format1.classValueArray = createGlyphs(classValues);

    return classDef;
}

ClassDefTable &Builder::createClassDef(const vector<class_range> classRanges)
{
    ClassDefTable &classDef = createObject<ClassDefTable>();
    classDef.classFormat = 2;
    classDef.format2.classRangeCount = (UInt16)classRanges.size();
    classDef.format2.classRangeRecord = createArray<ClassRangeRecord>(classRanges.size());

    for (size_t i = 0; i < classRanges.size(); i++) {
        const auto &value = classRanges[i];

        ClassRangeRecord &record = classDef.format2.classRangeRecord[i];
        record.start = get<0>(value);
        record.end = get<1>(value);
        record.clazz = get<2>(value);
    }

    return classDef;
}

DeviceTable &Builder::createDevice(const pair<UInt16, UInt16> sizeRange, const vector<Int8> values)
{
    UInt16 format = 1;

    for (Int8 value : values) {
        if (value >= -2 && value <= 1) {
            // Ignore.
        } else if (value >= -8 && value <= 7) {
            format = (format < 2 ? 2 : format);
        } else {
            format = 3;
        }
    }

    const UInt16 perDelta = (UInt16)(1 << (4 - format));
    const UInt16 valueBits = 16 / perDelta;
    const UInt16 valueMask = 0xFFFF >> (16 - valueBits);
    const UInt16 valueCount = (UInt16)values.size();
    const UInt16 deltaCount = valueCount / perDelta + (valueCount % perDelta != 0);

    UInt16 *deltaArray = createArray<UInt16>(deltaCount);
    size_t valueIndex = 0;

    for (size_t i = 0; i < deltaCount; i++) {
        UInt16 deltaShift = 16 - valueBits;
        UInt16 currentDelta = 0;

        for (size_t j = 0; j < perDelta && valueIndex < valueCount; j++) {
            currentDelta |= (values[valueIndex] & valueMask) << deltaShift;
            deltaShift -= valueBits;
            valueIndex++;
        }

        deltaArray[i] = currentDelta;
    }

    DeviceTable &device = createObject<DeviceTable>();
    device.startSize = sizeRange.first;
    device.endSize = sizeRange.second;
    device.deltaFormat = format;
    device.deltaValue = deltaArray;

    return device;
}

ConditionTable &Builder::createCondition(UInt16 axisIndex, const pair<float, float> filterRange)
{
    ConditionTable &condition = createObject<ConditionTable>();
    condition.format = 1;
    condition.format1.axisIndex = axisIndex;
    condition.format1.filterRangeMinValue = toF2DOT14(filterRange.first);
    condition.format1.filterRangeMaxValue = toF2DOT14(filterRange.second);

    return condition;
}

ConditionSetTable &Builder::createConditionSet(const pair<ConditionTable *, UInt16> conditions)
{
    ConditionSetTable &conditionSet = createObject<ConditionSetTable>();
    conditionSet.conditionCount = conditions.second;
    conditionSet.conditions = conditions.first;

    return conditionSet;
}

FeatureTableSubstitutionTable &Builder::createFeatureSubst(const map<UInt16, reference_wrapper<FeatureTable>> records)
{
    FeatureTableSubstitutionTable &featureSubst = createObject<FeatureTableSubstitutionTable>();
    featureSubst.majorVersion = 1;
    featureSubst.minorVersion = 0;
    featureSubst.substitutionCount = (UInt16)records.size();
    featureSubst.substitutions = createArray<FeatureTableSubstitutionRecord>(records.size());

    size_t index = 0;

    for (const auto &pair : records) {
        FeatureTableSubstitutionRecord &subst = featureSubst.substitutions[index];
        subst.featureIndex = pair.first;
        subst.alternateFeature = &pair.second.get();

        index++;
    }

    return featureSubst;
}

FeatureVariationsTable &Builder::createFeatureVariations(
    const vector<pair<reference_wrapper<ConditionSetTable>,
                      reference_wrapper<FeatureTableSubstitutionTable>>> records)
{
    FeatureVariationsTable &featureVariations = createObject<FeatureVariationsTable>();
    featureVariations.majorVersion = 1;
    featureVariations.minorVersion = 0;
    featureVariations.featureVariationRecordCount = (UInt16)records.size();
    featureVariations.featureVariationRecords = createArray<FeatureVariationRecord>(records.size());

    for (size_t i = 0; i < records.size(); i++) {
        FeatureVariationRecord &variation = featureVariations.featureVariationRecords[i];
        variation.conditionSet = &records[i].first.get();
        variation.featureTableSubstitution = &records[i].second.get();
    }

    return featureVariations;
}

UInt16 Builder::findMaxClass(ClassDefTable &classDef)
{
    UInt16 maxClass = 0;

    switch (classDef.classFormat) {
        case 1:
            for (size_t i = 0; i < classDef.format1.glyphCount; i++) {
                maxClass = max(maxClass, classDef.format1.classValueArray[i]);
            }
            break;

        case 2:
            for (size_t i = 0; i < classDef.format2.classRangeCount; i++) {
                maxClass = max(maxClass, classDef.format2.classRangeRecord[i].clazz);
            }
            break;
    }

    return maxClass;
}

SingleSubstSubtable &Builder::createSingleSubst(const set<Glyph> glyphs, Int16 delta)
{
    SingleSubstSubtable &subtable = createObject<SingleSubstSubtable>();
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(createGlyphs(glyphs), (UInt16)glyphs.size());
    subtable.format1.deltaGlyphID = delta;

    return subtable;
}

SingleSubstSubtable &Builder::createSingleSubst(const map<Glyph, Glyph> glyphs)
{
    Glyph *input = createGlyphs(glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });
    Glyph *output = createGlyphs(glyphs.begin(), glyphs.end(),
                                 [](const decltype(glyphs)::value_type &pair) {
                                     return pair.second;
                                 });

    SingleSubstSubtable &subtable = createObject<SingleSubstSubtable>();
    subtable.substFormat = 2;
    subtable.coverage = &createCoverage(input, (UInt16)glyphs.size());
    subtable.format2.glyphCount = (UInt16)glyphs.size();
    subtable.format2.substitute = output;

    return subtable;
}

MultipleSubstSubtable &Builder::createMultipleSubst(const map<Glyph, vector<Glyph>> glyphs)
{
    Glyph *input = createGlyphs(glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });

    SequenceTable *sequences = createArray<SequenceTable>(glyphs.size());
    for (size_t i = 0; i < glyphs.size(); i++) {
        const vector<Glyph> &substitutes = glyphs.at(input[i]);

        SequenceTable &current = sequences[i];
        current.glyphCount = (UInt16)substitutes.size();
        current.substitute = createGlyphs(substitutes);
    }

    MultipleSubstSubtable &subtable = createObject<MultipleSubstSubtable>();
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(input, (UInt16)glyphs.size());
    subtable.sequenceCount = (UInt16)glyphs.size();
    subtable.sequence = sequences;

    return subtable;
}

AlternateSubstSubtable &Builder::createAlternateSubst(const std::map<Glyph, std::vector<Glyph>> glyphs)
{
    Glyph *input = createGlyphs(glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });

    AlternateSetTable *alternateSets = createArray<AlternateSetTable>(glyphs.size());
    for (size_t i = 0; i < glyphs.size(); i++) {
        const vector<Glyph> &substitutes = glyphs.at(input[i]);

        AlternateSetTable &current = alternateSets[i];
        current.glyphCount = (UInt16)substitutes.size();
        current.substitute = createGlyphs(substitutes);
    }

    AlternateSubstSubtable &subtable = createObject<AlternateSubstSubtable>();
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(input, (UInt16)glyphs.size());
    subtable.alternateSetCount = (UInt16)glyphs.size();
    subtable.alternateSet = alternateSets;

    return subtable;
}

LigatureSubstSubtable &Builder::createLigatureSubst(const map<vector<Glyph>, Glyph> glyphs)
{
    set<Glyph> initials;
    vector<size_t> components;

    /* Extract all initial glyphs and their component count. */
    for (auto &pair : glyphs) {
        const vector<Glyph> &sequence = pair.first;

        if (initials.insert(sequence.at(0)).second) {
            components.push_back(1);
        } else {
            components.back()++;
        }
    }

    LigatureSubstSubtable &subtable = createObject<LigatureSubstSubtable>();
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(createGlyphs(initials), (UInt16)initials.size());
    subtable.ligSetCount = (UInt16)components.size();
    subtable.ligatureSet = createArray<LigatureSetTable>(components.size());

    LigatureSetTable *ligatureSet = nullptr;
    LigatureTable *ligature = nullptr;
    int previous = -1;
    size_t index = 0;

    for (auto &pair : glyphs) {
        const vector<Glyph> &sequence = pair.first;

        if (sequence[0] != previous) {
            ligatureSet = &subtable.ligatureSet[index];
            ligatureSet->ligatureCount = (UInt16)components[index];
            ligatureSet->ligature = createArray<LigatureTable>(components[index]);

            ligature = ligatureSet->ligature;
            index++;
        }

        ligature->ligGlyph = pair.second;
        ligature->compCount = (UInt16)sequence.size();
        ligature->component = createGlyphs(sequence.begin() + 1, sequence.end(),
                                           [](Glyph glyph) { return glyph; });

        previous = sequence[0];
        ligature++;
    }

    return subtable;
}

ReverseChainContextSubstSubtable &Builder::createRevChainSubst(
    const map<Glyph, Glyph> glyphs,
    const vector<vector<Glyph>> backtrack,
    const vector<vector<Glyph>> lookahead)
{
    Glyph *input = createGlyphs(glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });
    Glyph *output = createGlyphs(glyphs.begin(), glyphs.end(),
                                 [](const decltype(glyphs)::value_type &pair) {
                                     return pair.second;
                                 });

    ReverseChainContextSubstSubtable &subtable = createObject<ReverseChainContextSubstSubtable>();
    subtable.format = 1;
    subtable.coverage = &createCoverage(input, (UInt16)glyphs.size());
    subtable.backtrackGlyphCount = (UInt16)backtrack.size();
    subtable.backtrackGlyphCoverage = createArray<CoverageTable>(backtrack.size());
    subtable.lookaheadGlyphCount = (UInt16)lookahead.size();
    subtable.lookaheadGlyphCoverage = createArray<CoverageTable>(lookahead.size());
    subtable.glyphCount = (UInt16)glyphs.size();
    subtable.substitute = output;

    for (size_t i = 0; i < backtrack.size(); i++) {
        initCoverage(subtable.backtrackGlyphCoverage[backtrack.size() - i - 1],
                     createGlyphs(backtrack[i]), (UInt16)backtrack[i].size());
    }

    for (size_t i = 0; i < lookahead.size(); i++) {
        initCoverage(subtable.lookaheadGlyphCoverage[i],
                     createGlyphs(lookahead[i]), (UInt16)lookahead[i].size());
    }

    return subtable;
}

ContextSubtable &Builder::createContext(const vector<rule_context> rules)
{
    map<Glyph, vector<size_t>> ruleSets;

    /* Extract all initial glyphs with their rules. */
    for (size_t i = 0; i < rules.size(); i++) {
        Glyph ruleInitial = get<0>(rules[i])[0];
        vector<size_t> *ruleIndexes = nullptr;

        auto entry = ruleSets.find(ruleInitial);
        if (entry != ruleSets.end()) {
            ruleIndexes = &entry->second;
        } else {
            ruleIndexes = &(*ruleSets.insert({ ruleInitial, vector<size_t>() }).first).second;
        }

        ruleIndexes->push_back(i);
    }

    Glyph *initials = createGlyphs(ruleSets.begin(), ruleSets.end(),
                                   [](const decltype(ruleSets)::value_type &pair) {
                                       return pair.first;
                                   });

    ContextSubtable &subtable = createObject<ContextSubtable>();
    subtable.format = 1;
    subtable.format1.coverage = &createCoverage(initials, (UInt16)ruleSets.size());
    subtable.format1.ruleSetCount = (UInt16)ruleSets.size();
    subtable.format1.ruleSet = createArray<RuleSet>(ruleSets.size());

    size_t ruleSetIndex = 0;

    for (const auto &entry : ruleSets) {
        const vector<size_t> &ruleIndexes = entry.second;

        RuleSet &ruleSet = subtable.format1.ruleSet[ruleSetIndex++];
        ruleSet.ruleCount = (UInt16)ruleIndexes.size();
        ruleSet.rule = createArray<Rule>(ruleIndexes.size());

        for (size_t i = 0; i < ruleIndexes.size(); i++) {
            const rule_context &currentRule = rules[ruleIndexes[i]];
            const vector<Glyph> &input = get<0>(currentRule);
            const vector<pair<UInt16, UInt16>> &lookups = get<1>(currentRule);

            Rule &rule = ruleSet.rule[i];
            rule.glyphCount = (UInt16)input.size();
            rule.recordCount = (UInt16)lookups.size();
            rule.input = createGlyphs(input.begin() + 1, input.end(),
                                      [](Glyph glyph) { return glyph; });
            rule.lookupRecord = createArray<LookupRecord>(lookups.size());

            for (size_t j = 0; j < lookups.size(); j++) {
                LookupRecord &lookupRecord = rule.lookupRecord[j];
                lookupRecord.sequenceIndex = lookups[j].first;
                lookupRecord.lookupListIndex = lookups[j].second;
            }
        }
    }

    return subtable;
}

ContextSubtable &Builder::createContext(const vector<Glyph> initialGlyphs,
    const ClassDefTable &classDef,
    const vector<rule_context> rules)
{
    map<UInt16, vector<size_t>> classSets;

    /* Extract all initial classes with their rules. */
    for (size_t i = 0; i < rules.size(); i++) {
        UInt16 initialClass = get<0>(rules[i])[0];
        vector<size_t> *ruleIndexes = nullptr;

        auto classEntry = classSets.find(initialClass);
        if (classEntry != classSets.end()) {
            ruleIndexes = &classEntry->second;
        } else {
            ruleIndexes = &(*classSets.insert({ initialClass, vector<size_t>() }).first).second;
        }

        ruleIndexes->push_back(i);
    }

    /* Add rule sets for empty classes. */
    if (!classSets.empty()) {
        int previousClass = -1;

        for (auto entry = classSets.begin(); entry != classSets.end(); entry++) {
            UInt16 currentClass = entry->first;
            UInt16 emptyClass = (UInt16)(previousClass + 1);

            for (; emptyClass < currentClass; emptyClass++) {
                entry = classSets.insert({ emptyClass, vector<size_t>() }).first;
            }

            previousClass = currentClass;
        }
    }

    ContextSubtable &subtable = createObject<ContextSubtable>();
    subtable.format = 2;
    subtable.format2.coverage = &createCoverage(createGlyphs(initialGlyphs), (UInt16)initialGlyphs.size());
    subtable.format2.classDef = (ClassDefTable *)(&classDef);
    subtable.format2.classSetCnt = (UInt16)classSets.size();
    subtable.format2.classSet = createArray<ClassSet *>(classSets.size());

    size_t classSetIndex = 0;

    for (const auto &entry : classSets) {
        const vector<size_t> &ruleIndexes = entry.second;

        ClassSet *&classSet = subtable.format2.classSet[classSetIndex++];
        if (ruleIndexes.size() > 0) {
            classSet = &createObject<ClassSet>();
            classSet->classRuleCnt = (UInt16)ruleIndexes.size();
            classSet->classRule = createArray<ClassRule>(ruleIndexes.size());

            for (size_t i = 0; i < ruleIndexes.size(); i++) {
                const rule_context &currentRule = rules[ruleIndexes[i]];
                const vector<UInt16> &clazz = get<0>(currentRule);
                const vector<pair<UInt16, UInt16>> &lookups = get<1>(currentRule);

                ClassRule &classRule = classSet->classRule[i];
                classRule.glyphCount = (UInt16)clazz.size();
                classRule.recordCount = (UInt16)lookups.size();
                classRule.clazz = createGlyphs(clazz.begin() + 1, clazz.end(),
                                               [](Glyph glyph) { return glyph; });
                classRule.lookupRecord = createArray<LookupRecord>(lookups.size());

                for (size_t j = 0; j < lookups.size(); j++) {
                    LookupRecord &lookupRecord = classRule.lookupRecord[j];
                    lookupRecord.sequenceIndex = lookups[j].first;
                    lookupRecord.lookupListIndex = lookups[j].second;
                }
            }
        } else {
            classSet = NULL;
        }
    }
    
    return subtable;
}

ContextSubtable &Builder::createContext(const vector<vector<Glyph>> input,
    const vector<pair<UInt16, UInt16>> lookups)
{
    ContextSubtable &subtable = createObject<ContextSubtable>();
    subtable.format = 3;
    subtable.format3.glyphCount = (UInt16)input.size();
    subtable.format3.recordCount = (UInt16)lookups.size();
    subtable.format3.coverage = createArray<CoverageTable>(input.size());
    subtable.format3.lookupRecord = createArray<LookupRecord>(lookups.size());

    for (size_t i = 0; i < input.size(); i++) {
        initCoverage(subtable.format3.coverage[i],
                     createGlyphs(input[i]), (UInt16)input[i].size());
    }

    for (size_t i = 0; i < lookups.size(); i++) {
        LookupRecord &lookupRecord = subtable.format3.lookupRecord[i];
        lookupRecord.sequenceIndex = lookups[i].first;
        lookupRecord.lookupListIndex = lookups[i].second;
    }

    return subtable;
}

ChainContextSubtable &Builder::createChainContext(const vector<rule_chain_context> rules)
{
    map<Glyph, vector<size_t>> ruleSets;

    /* Extract all initial glyphs with their rules. */
    for (size_t i = 0; i < rules.size(); i++) {
        Glyph ruleInitial = get<1>(rules[i])[0];
        vector<size_t> *ruleIndexes = nullptr;

        auto entry = ruleSets.find(ruleInitial);
        if (entry != ruleSets.end()) {
            ruleIndexes = &entry->second;
        } else {
            ruleIndexes = &(*ruleSets.insert({ ruleInitial, vector<size_t>() }).first).second;
        }

        ruleIndexes->push_back(i);
    }

    Glyph *initials = createGlyphs(ruleSets.begin(), ruleSets.end(),
                                   [](const decltype(ruleSets)::value_type &pair) {
                                       return pair.first;
                                   });

    ChainContextSubtable &subtable = createObject<ChainContextSubtable>();
    subtable.format = 1;
    subtable.format1.coverage = &createCoverage(initials, (UInt16)ruleSets.size());
    subtable.format1.chainRuleSetCount = (UInt16)ruleSets.size();
    subtable.format1.chainRuleSet = createArray<ChainRuleSet>(ruleSets.size());

    size_t ruleSetIndex = 0;

    for (const auto &entry : ruleSets) {
        const vector<size_t> &ruleIndexes = entry.second;

        ChainRuleSet &chainRuleSet = subtable.format1.chainRuleSet[ruleSetIndex++];
        chainRuleSet.chainRuleCount = (UInt16)ruleIndexes.size();
        chainRuleSet.chainRule = createArray<ChainRule>(ruleIndexes.size());

        for (size_t i = 0; i < ruleIndexes.size(); i++) {
            const rule_chain_context &currentRule = rules[ruleIndexes[i]];
            const vector<Glyph> &backtrack = get<0>(currentRule);
            const vector<Glyph> &input = get<1>(currentRule);
            const vector<Glyph> &lookahead = get<2>(currentRule);
            const vector<pair<UInt16, UInt16>> &lookups = get<3>(currentRule);

            ChainRule &chainRule = chainRuleSet.chainRule[i];
            chainRule.backtrackGlyphCount = (UInt16)backtrack.size();
            chainRule.backtrack = createGlyphs(backtrack.rbegin(), backtrack.rend(),
                                               [](Glyph glyph) { return glyph; });
            chainRule.inputGlyphCount = (UInt16)input.size();
            chainRule.input = createGlyphs(input.begin() + 1, input.end(),
                                           [](Glyph glyph) { return glyph; });
            chainRule.lookaheadGlyphCount = (UInt16)lookahead.size();
            chainRule.lookAhead = createGlyphs(lookahead);
            chainRule.recordCount = (UInt16)lookups.size();
            chainRule.lookupRecord = createArray<LookupRecord>(lookups.size());

            for (size_t j = 0; j < lookups.size(); j++) {
                LookupRecord &lookupRecord = chainRule.lookupRecord[j];
                lookupRecord.sequenceIndex = lookups[j].first;
                lookupRecord.lookupListIndex = lookups[j].second;
            }
        }
    }
    
    return subtable;
}

ChainContextSubtable &Builder::createChainContext(
    const vector<Glyph> initialGlyphs,
    const reference_wrapper<ClassDefTable> classDefs[3],
    const vector<rule_chain_context> rules)
{
    map<Glyph, vector<size_t>> classSets;

    /* Extract all initial glyphs with their rules. */
    for (size_t i = 0; i < rules.size(); i++) {
        UInt16 initialClass = get<1>(rules[i])[0];
        vector<size_t> *ruleIndexes = nullptr;

        auto classEntry = classSets.find(initialClass);
        if (classEntry != classSets.end()) {
            ruleIndexes = &classEntry->second;
        } else {
            ruleIndexes = &(*classSets.insert({ initialClass, vector<size_t>() }).first).second;
        }

        ruleIndexes->push_back(i);
    }

    /* Add rule sets for empty classes. */
    if (!classSets.empty()) {
        int previousClass = -1;

        for (auto entry = classSets.begin(); entry != classSets.end(); entry++) {
            UInt16 currentClass = entry->first;
            UInt16 emptyClass = (UInt16)(previousClass + 1);

            for (; emptyClass < currentClass; emptyClass++) {
                entry = classSets.insert({ emptyClass, vector<size_t>() }).first;
            }

            previousClass = currentClass;
        }
    }

    ChainContextSubtable &subtable = createObject<ChainContextSubtable>();
    subtable.format = 2;
    subtable.format2.coverage = &createCoverage(createGlyphs(initialGlyphs), (UInt16)initialGlyphs.size());
    subtable.format2.backtrackClassDef = &classDefs[0].get();
    subtable.format2.inputClassDef = &classDefs[1].get();
    subtable.format2.lookaheadClassDef = &classDefs[2].get();
    subtable.format2.chainClassSetCnt = (UInt16)classSets.size();
    subtable.format2.chainClassSet = createArray<ChainClassSet *>(classSets.size());

    size_t classSetIndex = 0;

    for (const auto &entry : classSets) {
        const vector<size_t> &ruleIndexes = entry.second;

        ChainClassSet *&chainClassSet = subtable.format2.chainClassSet[classSetIndex++];
        if (ruleIndexes.size() > 0) {
            chainClassSet = &createObject<ChainClassSet>();
            chainClassSet->chainClassRuleCnt = (UInt16)ruleIndexes.size();
            chainClassSet->chainClassRule = createArray<ChainClassRule>(ruleIndexes.size());

            for (size_t i = 0; i < ruleIndexes.size(); i++) {
                const rule_chain_context &currentRule = rules[ruleIndexes[i]];
                const vector<Glyph> &backtrack = get<0>(currentRule);
                const vector<Glyph> &input = get<1>(currentRule);
                const vector<Glyph> &lookahead = get<2>(currentRule);
                const vector<pair<UInt16, UInt16>> &lookups = get<3>(currentRule);

                ChainClassRule &chainClassRule = chainClassSet->chainClassRule[i];
                chainClassRule.backtrackGlyphCount = (UInt16)backtrack.size();
                chainClassRule.backtrack = createGlyphs(backtrack.rbegin(), backtrack.rend(),
                                                        [](Glyph glyph) { return glyph; });
                chainClassRule.inputGlyphCount = (UInt16)input.size();
                chainClassRule.input = createGlyphs(input.begin() + 1, input.end(),
                                                    [](Glyph glyph) { return glyph; });
                chainClassRule.lookaheadGlyphCount = (UInt16)lookahead.size();
                chainClassRule.lookAhead = createGlyphs(lookahead);
                chainClassRule.recordCount = (UInt16)lookups.size();
                chainClassRule.lookupRecord = createArray<LookupRecord>(lookups.size());

                for (size_t j = 0; j < lookups.size(); j++) {
                    LookupRecord &lookupRecord = chainClassRule.lookupRecord[j];
                    lookupRecord.sequenceIndex = lookups[j].first;
                    lookupRecord.lookupListIndex = lookups[j].second;
                }
            }
        } else {
            chainClassSet = NULL;
        }
    }
    
    return subtable;
}

ChainContextSubtable &Builder::createChainContext(
    const vector<vector<Glyph>> backtrack,
    const vector<vector<Glyph>> input,
    const vector<vector<Glyph>> lookahead,
    const vector<pair<UInt16, UInt16>> lookups)
{
    ChainContextSubtable &subtable = createObject<ChainContextSubtable>();
    subtable.format = 3;
    subtable.format3.backtrackGlyphCount = (UInt16)backtrack.size();
    subtable.format3.backtrackGlyphCoverage = createArray<CoverageTable>(backtrack.size());
    subtable.format3.inputGlyphCount = (UInt16)input.size();
    subtable.format3.inputGlyphCoverage = createArray<CoverageTable>(input.size());
    subtable.format3.lookaheadGlyphCount = (UInt16)lookahead.size();
    subtable.format3.lookaheadGlyphCoverage = createArray<CoverageTable>(lookahead.size());
    subtable.format3.recordCount = (UInt16)lookups.size();
    subtable.format3.lookupRecord = createArray<LookupRecord>(lookups.size());

    for (size_t i = 0; i < backtrack.size(); i++) {
        initCoverage(subtable.format3.backtrackGlyphCoverage[backtrack.size() - i - 1],
                     createGlyphs(backtrack[i]), (UInt16)backtrack[i].size());
    }

    for (size_t i = 0; i < input.size(); i++) {
        initCoverage(subtable.format3.inputGlyphCoverage[i],
                     createGlyphs(input[i]), (UInt16)input[i].size());
    }

    for (size_t i = 0; i < lookahead.size(); i++) {
        initCoverage(subtable.format3.lookaheadGlyphCoverage[i],
                     createGlyphs(lookahead[i]), (UInt16)lookahead[i].size());
    }
    
    for (size_t i = 0; i < lookups.size(); i++) {
        LookupRecord &lookupRecord = subtable.format3.lookupRecord[i];
        lookupRecord.sequenceIndex = lookups[i].first;
        lookupRecord.lookupListIndex = lookups[i].second;
    }
    
    return subtable;
}

ExtensionSubtable &Builder::createExtension(LookupType lookupType, const LookupSubtable &innerTable)
{
    ExtensionSubtable &subtable = createObject<ExtensionSubtable>();
    subtable.format = 1;
    subtable.extensionLookupType = lookupType;
    subtable.extensionTable = (LookupSubtable *)(&innerTable);

    return subtable;
}

template<class InputIt, class Operation>
ValueFormat Builder::findValueFormat(InputIt begin, InputIt end, Operation operation)
{
    ValueFormat format = ValueFormat::None;

    while (begin != end) {
        const ValueRecord &record = operation(*begin++);

        if (record.xPlacement != 0) {
            format = format | ValueFormat::XPlacement;
        }
        if (record.yPlacement != 0) {
            format = format | ValueFormat::YPlacement;
        }
        if (record.xAdvance != 0) {
            format = format | ValueFormat::XAdvance;
        }
        if (record.yAdvance != 0) {
            format = format | ValueFormat::YAdvance;
        }
        if (record.xPlaDevice != NULL) {
            format = format | ValueFormat::XPlaDevice;
        }
        if (record.yPlaDevice != NULL) {
            format = format | ValueFormat::YPlaDevice;
        }
        if (record.xAdvDevice != NULL) {
            format = format | ValueFormat::XAdvDevice;
        }
        if (record.yAdvDevice != NULL) {
            format = format | ValueFormat::YAdvDevice;
        }
    }

    return format;
}

ValueFormat Builder::findValueFormat(const vector<reference_wrapper<ValueRecord>> &valueRecords)
{
    return findValueFormat(valueRecords.begin(), valueRecords.end(),
                           [](const reference_wrapper<ValueRecord> &record) {
                               return record.get();
                           });
}

ValueRecord &Builder::createValueRecord(
    const array<Int16, 4> metrics,
    const array<DeviceTable *, 4> devices)
{
    ValueRecord &valueRecord = createObject<ValueRecord>();
    valueRecord.xPlacement = metrics[0];
    valueRecord.yPlacement = metrics[1];
    valueRecord.xAdvance = metrics[2];
    valueRecord.yAdvance = metrics[3];
    valueRecord.xPlaDevice = devices[0];
    valueRecord.yPlaDevice = devices[1];
    valueRecord.xAdvDevice = devices[2];
    valueRecord.yAdvDevice = devices[3];

    return valueRecord;
}

AnchorTable &Builder::createAnchor(Int16 xCoordinate, Int16 yCoordinate)
{
    AnchorTable &anchor = createObject<AnchorTable>();
    anchor.anchorFormat = 1;
    anchor.xCoordinate = xCoordinate;
    anchor.yCoordinate = yCoordinate;

    return anchor;
}

AnchorTable &Builder::createAnchor(Int16 xCoordinate, Int16 yCoordinate, UInt16 anchorPoint)
{
    AnchorTable &anchor = createObject<AnchorTable>();
    anchor.anchorFormat = 2;
    anchor.xCoordinate = xCoordinate;
    anchor.yCoordinate = yCoordinate;
    anchor.format2.anchorPoint = anchorPoint;

    return anchor;

}

AnchorTable &Builder::createAnchor(Int16 xCoordinate, Int16 yCoordinate,
    DeviceTable *xDevice, DeviceTable *yDevice)
{
    AnchorTable &anchor = createObject<AnchorTable>();
    anchor.anchorFormat = 3;
    anchor.xCoordinate = xCoordinate;
    anchor.yCoordinate = yCoordinate;
    anchor.format3.xDeviceTable = xDevice;
    anchor.format3.yDeviceTable = yDevice;

    return anchor;
}

SinglePosSubtable &Builder::createSinglePos(const vector<Glyph> glyphs, ValueRecord &valueRecord)
{
    SinglePosSubtable &subtable = createObject<SinglePosSubtable>();
    subtable.posFormat = 1;
    subtable.coverage = &createCoverage(createGlyphs(glyphs), (UInt16)glyphs.size());
    subtable.valueFormat = findValueFormat({ valueRecord });
    subtable.format1.value = &valueRecord;

    return subtable;
}

SinglePosSubtable &Builder::createSinglePos(
    const vector<Glyph> glyphs,
    const vector<reference_wrapper<ValueRecord>> valueRecords)
{
    SinglePosSubtable &subtable = createObject<SinglePosSubtable>();
    subtable.posFormat = 2;
    subtable.coverage = &createCoverage(createGlyphs(glyphs), (UInt16)glyphs.size());
    subtable.valueFormat = findValueFormat(valueRecords);
    subtable.format2.value = createArray<ValueRecord>(valueRecords.size());
    subtable.format2.valueCount = (UInt16)valueRecords.size();

    for (size_t i = 0; i < valueRecords.size(); i++) {
        subtable.format2.value[i] = valueRecords[i];
    }

    return subtable;
}

PairAdjustmentPosSubtable &Builder::createPairPos(const vector<pair_rule> rules)
{
    map<Glyph, vector<size_t>> pairSets;

    /* Extract all initial glyphs with their pair sets. */
    for (size_t i = 0; i < rules.size(); i++) {
        Glyph pairInitial = get<0>(rules[i]);
        vector<size_t> *pairIndexes = nullptr;

        auto entry = pairSets.find(pairInitial);
        if (entry != pairSets.end()) {
            pairIndexes = &entry->second;
        } else {
            pairIndexes = &(*pairSets.insert({ pairInitial, vector<size_t>() }).first).second;
        }

        pairIndexes->push_back(i);
    }

    Glyph *initials = createGlyphs(pairSets.begin(), pairSets.end(),
                                   [](const decltype(pairSets)::value_type &pair) {
                                       return pair.first;
                                   });

    PairAdjustmentPosSubtable &subtable = createObject<PairAdjustmentPosSubtable>();
    subtable.posFormat = 1;
    subtable.coverage = &createCoverage(initials, (UInt16)pairSets.size());
    subtable.valueFormat1 = findValueFormat(rules.begin(), rules.end(),
                                            [](const decltype(rules)::value_type &rule) {
                                                return get<2>(rule).get();
                                            });
    subtable.valueFormat2 = findValueFormat(rules.begin(), rules.end(),
                                            [](const decltype(rules)::value_type &rule) {
                                                return get<3>(rule).get();
                                            });
    subtable.format1.pairSetCount = (UInt16)pairSets.size();
    subtable.format1.pairSetTable = createArray<PairSetTable>(pairSets.size());

    size_t pairSetIndex = 0;

    for (const auto &entry : pairSets) {
        const vector<size_t> &ruleIndexes = entry.second;

        PairSetTable &pairSet = subtable.format1.pairSetTable[pairSetIndex++];
        pairSet.pairValueCount = (UInt16)ruleIndexes.size();
        pairSet.pairValueRecord = createArray<PairValueRecord>(ruleIndexes.size());

        for (size_t i = 0; i < ruleIndexes.size(); i++) {
            const pair_rule &currentRule = rules[ruleIndexes[i]];

            PairValueRecord &pairValueRecord = pairSet.pairValueRecord[i];
            pairValueRecord.secondGlyph = get<1>(currentRule);
            pairValueRecord.value1 = &get<2>(currentRule).get();
            pairValueRecord.value2 = &get<3>(currentRule).get();
        }
    }

    return subtable;
}

PairAdjustmentPosSubtable &Builder::createPairPos(
    const vector<Glyph> initialGlyphs,
    const reference_wrapper<ClassDefTable> classDefs[2],
    const vector<pair_rule> rules)
{
    map<UInt16, map<UInt16, size_t>> classSets;

    /* Extract all classes with their rules. */
    for (size_t i = 0; i < rules.size(); i++) {
        UInt16 class1 = get<0>(rules[i]);
        UInt16 class2 = get<1>(rules[i]);
        map<UInt16, size_t> *classRules = nullptr;

        auto classEntry = classSets.find(class1);
        if (classEntry != classSets.end()) {
            classRules = &classEntry->second;
        } else {
            classRules = &(*classSets.insert({ class1, map<UInt16, size_t>() }).first).second;
        }

        classRules->insert({class2, i});
    }

    PairAdjustmentPosSubtable &subtable = createObject<PairAdjustmentPosSubtable>();
    subtable.posFormat = 2;
    subtable.coverage = &createCoverage(createGlyphs(initialGlyphs), (UInt16)initialGlyphs.size());
    subtable.valueFormat1 = findValueFormat(rules.begin(), rules.end(),
                                            [](const decltype(rules)::value_type &rule) {
                                                return get<2>(rule).get();
                                            });
    subtable.valueFormat2 = findValueFormat(rules.begin(), rules.end(),
                                            [](const decltype(rules)::value_type &rule) {
                                                return get<3>(rule).get();
                                            });
    subtable.format2.classDef1 = &classDefs[0].get();
    subtable.format2.classDef2 = &classDefs[1].get();
    subtable.format2.class1Count = findMaxClass(classDefs[0]) + 1;
    subtable.format2.class2Count = findMaxClass(classDefs[1]) + 1;
    subtable.format2.class1Record = createArray<Class1Record>(subtable.format2.class1Count);

    map<UInt16, size_t> emptyRules;
    ValueRecord &emptyRecord = createValueRecord({ 0, 0, 0, 0 });

    for (UInt16 class1 = 0; class1 < subtable.format2.class1Count; class1++) {
        const map<UInt16, size_t> *classRules = nullptr;

        auto class1Entry = classSets.find(class1);
        if (class1Entry == classSets.end()) {
            classRules = &emptyRules;
        } else {
            classRules = &class1Entry->second;
        }

        Class1Record &class1Record = subtable.format2.class1Record[class1];
        class1Record.class2Record = createArray<Class2Record>(subtable.format2.class2Count);

        for (UInt16 class2 = 0; class2 < subtable.format2.class2Count; class2++) {
            Class2Record &class2Record = class1Record.class2Record[class2];

            auto class2Entry = classRules->find(class2);
            if (class2Entry == classRules->end()) {
                class2Record.value1 = &emptyRecord;
                class2Record.value2 = &emptyRecord;
            } else {
                const pair_rule &currentRule = rules[class2Entry->second];

                class2Record.value1 = &get<2>(currentRule).get();
                class2Record.value2 = &get<3>(currentRule).get();
            }
        }
    }

    return subtable;
}

CursiveAttachmentPosSubtable &Builder::createCursivePos(
    const map<Glyph, pair<AnchorTable *, AnchorTable *>> rules)
{
    Glyph *glyphs = createGlyphs(rules.begin(), rules.end(),
                                 [](const decltype(rules)::value_type &rule) {
                                     return rule.first;
                                 });

    CursiveAttachmentPosSubtable &subtable = createObject<CursiveAttachmentPosSubtable>();
    subtable.posFormat = 1;
    subtable.coverage = &createCoverage(glyphs, (UInt16)rules.size());
    subtable.entryExitCount = (UInt16)rules.size();
    subtable.entryExitRecord = createArray<EntryExitRecord>(rules.size());

    size_t recordIndex = 0;

    for (const auto &entry : rules) {
        EntryExitRecord &record = subtable.entryExitRecord[recordIndex++];
        record.entryAnchor = get<0>(entry.second);
        record.exitAnchor = get<1>(entry.second);
    }

    return subtable;
}

MarkToBaseAttachmentPosSubtable &Builder::createMarkToBasePos(
    UInt16 classCount,
    const map<Glyph, pair<UInt16, reference_wrapper<AnchorTable>>> markRules,
    const map<Glyph, vector<reference_wrapper<AnchorTable>>> baseRules)
{
    Glyph *markGlyphs = createGlyphs(markRules.begin(), markRules.end(),
                                     [](const decltype(markRules)::value_type &rule) {
                                         return get<0>(rule);
                                     });
    Glyph *baseGlyphs = createGlyphs(baseRules.begin(), baseRules.end(),
                                     [](const typename decltype(baseRules)::value_type &rule) {
                                         return get<0>(rule);
                                     });

    MarkToBaseAttachmentPosSubtable &subtable = createObject<MarkToBaseAttachmentPosSubtable>();
    subtable.posFormat = 1;
    subtable.markCoverage = &createCoverage(markGlyphs, (UInt16)markRules.size());
    subtable.baseCoverage = &createCoverage(baseGlyphs, (UInt16)baseRules.size());
    subtable.classCount = classCount;
    subtable.markArray = &createObject<MarkArrayTable>();
    subtable.baseArray = &createObject<BaseArrayTable>();

    MarkArrayTable &markArray = *subtable.markArray;
    markArray.markCount = (UInt16)markRules.size();
    markArray.markRecord = createArray<MarkRecord>(markRules.size());

    size_t markIndex = 0;

    for (const auto &entry : markRules) {
        MarkRecord &markRecord = markArray.markRecord[markIndex++];
        markRecord.clazz = get<0>(entry.second);
        markRecord.markAnchor = &get<1>(entry.second).get();
    }

    BaseArrayTable &baseArray = *subtable.baseArray;
    baseArray.baseCount = (UInt16)baseRules.size();
    baseArray.baseRecord = createArray<BaseRecord>(baseRules.size());

    size_t baseIndex = 0;

    for (const auto &entry : baseRules) {
        BaseRecord &baseRecord = baseArray.baseRecord[baseIndex++];
        baseRecord.baseAnchor = createArray<AnchorTable>(classCount);

        for (size_t i = 0; i < classCount; i++) {
            baseRecord.baseAnchor[i] = entry.second[i];
        }
    }

    return subtable;
}

MarkToLigatureAttachmentPosSubtable &Builder::createMarkToLigaturePos(
     UInt16 classCount,
     const map<Glyph, pair<UInt16, reference_wrapper<AnchorTable>>> markRules,
     const map<Glyph, vector<vector<reference_wrapper<AnchorTable>>>> ligatureRules)
{
    Glyph *markGlyphs = createGlyphs(markRules.begin(), markRules.end(),
                                     [](const decltype(markRules)::value_type &rule) {
                                         return get<0>(rule);
                                     });
    Glyph *ligatureGlyphs = createGlyphs(ligatureRules.begin(), ligatureRules.end(),
                                         [](const typename decltype(ligatureRules)::value_type &rule) {
                                             return get<0>(rule);
                                         });

    MarkToLigatureAttachmentPosSubtable &subtable = createObject<MarkToLigatureAttachmentPosSubtable>();
    subtable.posFormat = 1;
    subtable.markCoverage = &createCoverage(markGlyphs, (UInt16)markRules.size());
    subtable.ligatureCoverage = &createCoverage(ligatureGlyphs, (UInt16)ligatureRules.size());
    subtable.classCount = classCount;
    subtable.markArray = &createObject<MarkArrayTable>();
    subtable.ligatureArray = &createObject<LigatureArrayTable>();

    MarkArrayTable &markArray = *subtable.markArray;
    markArray.markCount = (UInt16)markRules.size();
    markArray.markRecord = createArray<MarkRecord>(markRules.size());

    size_t markIndex = 0;

    for (const auto &entry : markRules) {
        MarkRecord &markRecord = markArray.markRecord[markIndex++];
        markRecord.clazz = get<0>(entry.second);
        markRecord.markAnchor = &get<1>(entry.second).get();
    }

    LigatureArrayTable &ligatureArray = *subtable.ligatureArray;
    ligatureArray.ligatureCount = (UInt16)ligatureRules.size();
    ligatureArray.ligatureAttach = createArray<LigatureAttachTable>(ligatureRules.size());

    size_t ligatureIndex = 0;

    for (const auto &entry : ligatureRules) {
        const auto &componentRules = entry.second;

        LigatureAttachTable &ligatureAttach = ligatureArray.ligatureAttach[ligatureIndex++];
        ligatureAttach.componentCount = (UInt16)componentRules.size();
        ligatureAttach.componentRecord = createArray<ComponentRecord>(componentRules.size());

        for (size_t i = 0; i < componentRules.size(); i++) {
            ComponentRecord &componentRecord = ligatureAttach.componentRecord[i];
            componentRecord.ligatureAnchor = createArray<AnchorTable>(classCount);

            for (size_t j = 0; j < classCount; j++) {
                componentRecord.ligatureAnchor[j] = componentRules[i][j];
            }
        }
    }
    
    return subtable;
}

MarkToMarkAttachmentPosSubtable &Builder::createMarkToMarkPos(
     UInt16 classCount,
     const map<Glyph, pair<UInt16, reference_wrapper<AnchorTable>>> mark1Rules,
     const map<Glyph, vector<reference_wrapper<AnchorTable>>> mark2Rules)
{
    Glyph *mark1Glyphs = createGlyphs(mark1Rules.begin(), mark1Rules.end(),
                                     [](const decltype(mark1Rules)::value_type &rule) {
                                         return get<0>(rule);
                                     });
    Glyph *mark2Glyphs = createGlyphs(mark2Rules.begin(), mark2Rules.end(),
                                     [](const typename decltype(mark2Rules)::value_type &rule) {
                                         return get<0>(rule);
                                     });

    MarkToMarkAttachmentPosSubtable &subtable = createObject<MarkToMarkAttachmentPosSubtable>();
    subtable.posFormat = 1;
    subtable.mark1Coverage = &createCoverage(mark1Glyphs, (UInt16)mark1Rules.size());
    subtable.mark2Coverage = &createCoverage(mark2Glyphs, (UInt16)mark2Rules.size());
    subtable.classCount = classCount;
    subtable.mark1Array = &createObject<MarkArrayTable>();
    subtable.mark2Array = &createObject<Mark2ArrayTable>();

    MarkArrayTable &mark1Array = *subtable.mark1Array;
    mark1Array.markCount = (UInt16)mark1Rules.size();
    mark1Array.markRecord = createArray<MarkRecord>(mark1Rules.size());

    size_t markIndex = 0;

    for (const auto &entry : mark1Rules) {
        MarkRecord &markRecord = mark1Array.markRecord[markIndex++];
        markRecord.clazz = get<0>(entry.second);
        markRecord.markAnchor = &get<1>(entry.second).get();
    }

    Mark2ArrayTable &mark2Array = *subtable.mark2Array;
    mark2Array.mark2Count = (UInt16)mark2Rules.size();
    mark2Array.mark2Record = createArray<Mark2Record>(mark2Rules.size());

    size_t mark2Index = 0;

    for (const auto &entry : mark2Rules) {
        Mark2Record &mark2Record = mark2Array.mark2Record[mark2Index++];
        mark2Record.mark2Anchor = createArray<AnchorTable>(classCount);

        for (size_t i = 0; i < classCount; i++) {
            mark2Record.mark2Anchor[i] = entry.second[i];
        }
    }
    
    return subtable;
}
