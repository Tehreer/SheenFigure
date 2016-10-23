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
    shared_ptr<T> object = make_shared<T>(std::forward<Args>(args)...);
    m_pool.push_back(object);

    return *object;
}

template<class T>
T *Builder::createArray(size_t size)
{
    return &createObject<vector<T>>(size)[0];
}

template<class InputIt, class Operation>
Glyph *Builder::createGlyphs(InputIt begin, InputIt end, Operation operation)
{
    vector<Glyph> &glyphs = createObject<vector<Glyph>>();

    while (begin != end) {
        glyphs.push_back(operation(*begin++));
    }

    return &glyphs[0];
}

template<class Collection>
Glyph *Builder::createGlyphs(const Collection &glyphs)
{
    return createGlyphs(glyphs.begin(), glyphs.end(),
                        [](Glyph glyph) { return glyph; });
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
    const std::reference_wrapper<ClassDefTable> classDefs[3],
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
        initCoverage(subtable.format3.backtrackGlyphCoverage[i],
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
