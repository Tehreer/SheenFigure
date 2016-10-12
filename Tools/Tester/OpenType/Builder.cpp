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

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>
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

MultipleSubstSubtable &Builder::createMultipleSubst(const map<Glyph, const vector<Glyph>> glyphs)
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

LigatureSubstSubtable &Builder::createLigatureSubst(const map<const vector<Glyph>, Glyph> glyphs)
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

ChainContextSubtable &Builder::createChainContext(
    const vector<const vector<Glyph>> backtrack,
    const vector<const vector<Glyph>> input,
    const vector<const vector<Glyph>> lookahead,
    const vector<const pair<UInt16, UInt16>> lookups)
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
