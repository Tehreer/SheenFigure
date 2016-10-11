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

#include <cstdint>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

extern "C" {
#include <Source/SFAlbum.h>
#include <Source/SFAssert.h>
}

#include "OpenType/Common.h"
#include "OpenType/GSUB.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

typedef list<shared_ptr<void>> ObjectPool;

template<class T, class... Args>
T &createObject(ObjectPool &pool, Args&&... args)
{
    shared_ptr<T> object = make_shared<T>(std::forward<Args>(args)...);
    pool.push_back(object);

    return *object;
}

template<class T>
T *createArray(ObjectPool &pool, size_t size)
{
    return &createObject<vector<T>>(pool, size)[0];
}

template<class InputIt, class Operation>
static Glyph *createGlyphs(ObjectPool &pool, InputIt begin, InputIt end, Operation operation)
{
    vector<Glyph> &glyphs = createObject<vector<Glyph>>(pool);

    while (begin != end) {
        glyphs.push_back(operation(*begin++));
    }

    return &glyphs[0];
}

template<class Collection>
static Glyph *createGlyphs(ObjectPool &pool, const Collection &glyphs)
{
    return createGlyphs(pool, glyphs.begin(), glyphs.end(),
                        [](Glyph glyph) { return glyph; });
}

static void initCoverage(CoverageTable &coverage, Glyph *glyphs, UInt16 count)
{
    coverage.coverageFormat = 1;
    coverage.format1.glyphCount = count;
    coverage.format1.glyphArray = glyphs;
}

static CoverageTable &createCoverage(ObjectPool &pool, Glyph *glyphs, UInt16 count)
{
    CoverageTable &coverage = createObject<CoverageTable>(pool);
    initCoverage(coverage, glyphs, count);

    return coverage;
}

static SingleSubstSubtable &createSingleSubst(ObjectPool &pool, const set<Glyph> glyphs, Int16 delta)
{
    SingleSubstSubtable &subtable = createObject<SingleSubstSubtable>(pool);
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(pool, createGlyphs(pool, glyphs), (UInt16)glyphs.size());
    subtable.format1.deltaGlyphID = delta;

    return subtable;
}


static SingleSubstSubtable &createSingleSubst(ObjectPool &pool, const map<Glyph, Glyph> glyphs)
{
    Glyph *input = createGlyphs(pool, glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });
    Glyph *output = createGlyphs(pool, glyphs.begin(), glyphs.end(),
                                 [](const decltype(glyphs)::value_type &pair) {
                                     return pair.second;
                                 });

    SingleSubstSubtable &subtable = createObject<SingleSubstSubtable>(pool);
    subtable.substFormat = 2;
    subtable.coverage = &createCoverage(pool, input, (UInt16)glyphs.size());
    subtable.format2.glyphCount = (UInt16)glyphs.size();
    subtable.format2.substitute = output;

    return subtable;
}

static MultipleSubstSubtable &createMultipleSubst(ObjectPool &pool, const map<Glyph, const vector<Glyph>> glyphs)
{
    Glyph *input = createGlyphs(pool, glyphs.begin(), glyphs.end(),
                                [](const decltype(glyphs)::value_type &pair) {
                                    return pair.first;
                                });

    SequenceTable *sequences = createArray<SequenceTable>(pool, glyphs.size());
    for (size_t i = 0; i < glyphs.size(); i++) {
        const vector<Glyph> &substitutes = glyphs.at(input[i]);

        SequenceTable &current = sequences[i];
        current.glyphCount = (UInt16)substitutes.size();
        current.substitute = createGlyphs(pool, substitutes);
    }

    MultipleSubstSubtable &subtable = createObject<MultipleSubstSubtable>(pool);
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(pool, input, (UInt16)glyphs.size());
    subtable.sequenceCount = (UInt16)glyphs.size();
    subtable.sequence = sequences;

    return subtable;
}

static LigatureSubstSubtable &createLigatureSubst(ObjectPool &pool, const map<const vector<Glyph>, Glyph> glyphs)
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

    LigatureSubstSubtable &subtable = createObject<LigatureSubstSubtable>(pool);
    subtable.substFormat = 1;
    subtable.coverage = &createCoverage(pool, createGlyphs(pool, initials), (UInt16)initials.size());
    subtable.ligSetCount = (UInt16)components.size();
    subtable.ligatureSet = createArray<LigatureSetTable>(pool, components.size());

    LigatureSetTable *ligatureSet = nullptr;
    LigatureTable *ligature = nullptr;
    int previous = -1;
    size_t index = 0;

    for (auto &pair : glyphs) {
        const vector<Glyph> &sequence = pair.first;

        if (sequence[0] != previous) {
            ligatureSet = &subtable.ligatureSet[index];
            ligatureSet->ligatureCount = (UInt16)components[index];
            ligatureSet->ligature = createArray<LigatureTable>(pool, components[index]);

            ligature = ligatureSet->ligature;
            index++;
        }

        ligature->ligGlyph = pair.second;
        ligature->compCount = (UInt16)sequence.size();
        ligature->component = createGlyphs(pool, sequence.begin() + 1, sequence.end(),
                                           [](Glyph glyph) { return glyph; });

        previous = sequence[0];
        ligature++;
    }

    return subtable;
}

static ChainContextSubtable &createChainContext(ObjectPool &pool,
    const vector<const vector<Glyph>> backtrack,
    const vector<const vector<Glyph>> input,
    const vector<const vector<Glyph>> lookahead,
    const vector<const tuple<UInt16, UInt16>> lookups)
{
    ChainContextSubtable &subtable = createObject<ChainContextSubtable>(pool);
    subtable.format = 3;
    subtable.format3.backtrackGlyphCount = (UInt16)backtrack.size();
    subtable.format3.backtrackGlyphCoverage = createArray<CoverageTable>(pool, backtrack.size());
    subtable.format3.inputGlyphCount = (UInt16)input.size();
    subtable.format3.inputGlyphCoverage = createArray<CoverageTable>(pool, input.size());
    subtable.format3.lookaheadGlyphCount = (UInt16)lookahead.size();
    subtable.format3.lookaheadGlyphCoverage = createArray<CoverageTable>(pool, lookahead.size());
    subtable.format3.recordCount = (UInt16)lookups.size();
    subtable.format3.lookupRecord = createArray<LookupRecord>(pool, lookups.size());

    for (size_t i = 0; i < backtrack.size(); i++) {
        initCoverage(subtable.format3.backtrackGlyphCoverage[i],
                     createGlyphs(pool, backtrack[i]), (UInt16)backtrack[i].size());
    }

    for (size_t i = 0; i < input.size(); i++) {
        initCoverage(subtable.format3.inputGlyphCoverage[i],
                     createGlyphs(pool, input[i]), (UInt16)input[i].size());
    }

    for (size_t i = 0; i < lookahead.size(); i++) {
        initCoverage(subtable.format3.lookaheadGlyphCoverage[i],
                     createGlyphs(pool, lookahead[i]), (UInt16)lookahead[i].size());
    }

    for (size_t i = 0; i < lookups.size(); i++) {
        LookupRecord &lookupRecord = subtable.format3.lookupRecord[i];
        lookupRecord.sequenceIndex = get<0>(lookups[i]);
        lookupRecord.lookupListIndex = get<1>(lookups[i]);
    }

    return subtable;
}

void TextProcessorTester::testSingleSubstitution()
{
    ObjectPool pool;

    /* Test the first format. */
    {
        /* Test with unmatching glyph. */
        testSubstitution(createSingleSubst(pool, { 0 }, 0), { 1 }, { 1 });
        /* Test with zero delta. */
        testSubstitution(createSingleSubst(pool, { 1 }, 0), { 1 }, { 1 });
        /* Test with positive delta. */
        testSubstitution(createSingleSubst(pool, { 1 }, 99), { 1 }, { 100 });
        /* Test with negative delta. */
        testSubstitution(createSingleSubst(pool, { 100 }, -99), { 100 }, { 1 });
        /* Test with opposite delta. */
        testSubstitution(createSingleSubst(pool, { 1 }, -1), { 1 }, { 0 });
    }

    /* Test the second format. */
    {
        /* Test with unmatching glyph. */
        testSubstitution(createSingleSubst(pool, { {0, 0} }), { 1 }, { 1 });
        /* Test with zero glyph. */
        testSubstitution(createSingleSubst(pool, { {0, 1} }), { 0 }, { 1 });
        /* Test with zero substitution. */
        testSubstitution(createSingleSubst(pool, { {1, 0} }), { 1 }, { 0 });
        /* Test with same substitution. */
        testSubstitution(createSingleSubst(pool, { {1, 1} }), { 1 }, { 1 });
        /* Test with a different substitution. */
        testSubstitution(createSingleSubst(pool, { {1, 100} }), { 1 }, { 100 });
    }
}

void TextProcessorTester::testMultipleSubstitution()
{
    ObjectPool pool;

    /* Test with unmatching glyph. */
    testSubstitution(createMultipleSubst(pool, { {0, { 1, 2, 3 }} }), { 1 }, { 1 });
    /* Test with no glyph. */
    testSubstitution(createMultipleSubst(pool, { {1, { }} }), { 1 }, { 1 });
    /* Test with zero glyph. */
    testSubstitution(createMultipleSubst(pool, { {0, { 1 }} }), { 0 }, { 1 });
    /* Test with zero substitution. */
    testSubstitution(createMultipleSubst(pool, { {1, { 0 }} }), { 1 }, { 0 });
    /* Test with same substitution. */
    testSubstitution(createMultipleSubst(pool, { {1, { 1 }} }), { 1 }, { 1 });
    /* Test with different single substitution. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100 }} }), { 1 }, { 100 });
    /* Test with different two substitutions. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100, 200 }} }), { 1 }, { 100, 200 });
    /* Test with different multiple substitutions. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100, 200, 300 }} }), { 1 }, { 100, 200, 300 });
    /* Test with multiple substitutions having input glyph at the start. */
    testSubstitution(createMultipleSubst(pool, { {1, { 1, 200, 300 }} }), { 1 }, { 1, 200, 300 });
    /* Test with multiple substitutions having input glyph at the middle. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100, 1, 300 }} }), { 1 }, { 100, 1, 300 });
    /* Test with multiple substitutions having input glyph at the end. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100, 200, 1 }} }), { 1 }, { 100, 200, 1 });
    /* Test with multiple substitutions having input glyph everywhere. */
    testSubstitution(createMultipleSubst(pool, { {1, { 1, 1, 1 }} }), { 1 }, { 1, 1, 1 });
    /* Test with multiple repeating substitutions. */
    testSubstitution(createMultipleSubst(pool, { {1, { 100, 100, 100 }} }), { 1 }, { 100, 100, 100 });
    /* Test with multiple zero substitutions. */
    testSubstitution(createMultipleSubst(pool, { {1, { 0, 0, 0 }} }), { 1 }, { 0, 0, 0 });
}

void TextProcessorTester::testLigatureSubstitution()
{
    ObjectPool pool;

    /* Test with unmatching glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2, 3 }, 0} }), { 1 }, { 1 });
    /* Test with zero glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 0 }, 1} }), { 0 }, { 1 });
    /* Test with zero substitution. */
    testSubstitution(createLigatureSubst(pool, { {{ 1 }, 0} }), { 1 }, { 0 });
    /* Test with same substitution. */
    testSubstitution(createLigatureSubst(pool, { {{ 1 }, 1} }), { 1 }, { 1 });
    /* Test with different substitution. */
    testSubstitution(createLigatureSubst(pool, { {{ 1 }, 100} }), { 1 }, { 100 });
    /* Test with two different glyphs. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2 }, 100} }), { 1, 2 }, { 100 });
    /* Test with multiple different glyphs. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2, 3 }, 100} }), { 1, 2, 3 }, { 100 });
    /* Test with multiple glyphs translating to first input glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2, 3 }, 1} }), { 1, 2, 3 }, { 1 });
    /* Test with multiple glyphs translating to middle input glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2, 3 }, 2} }), { 1, 2, 3 }, { 2 });
    /* Test with multiple glyphs translating to last input glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 2, 3 }, 3} }), { 1, 2, 3 }, { 3 });
    /* Test with multiple same glyphs translating to itself. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 1, 1 }, 1} }), { 1, 1, 1 }, { 1 });
    /* Test with multiple same glyphs translating to a different glyph. */
    testSubstitution(createLigatureSubst(pool, { {{ 1, 1, 1 }, 100} }), { 1, 1, 1 }, { 100 });
    /* Test with multiple zero glyphs. */
    testSubstitution(createLigatureSubst(pool, { {{ 0, 0, 0 }, 100} }), { 0, 0, 0 }, { 100 });
}

void TextProcessorTester::testChainContextSubstitution()
{
    ObjectPool pool;

    /* Test with simple substitution. */
    {
        vector<LookupSubtable *> referrals = {
            &createSingleSubst(pool, { 2 }, 1)
        };
        ChainContextSubtable &subtable = createChainContext(pool,
            { { 1 }, { 1 }, { 1 } },
            { { 1 }, { 2 }, { 3 } },
            { { 3 }, { 3 }, { 3 } },
            { { 1, 1 } }
        );
        testSubstitution(subtable,
                         { 1, 1, 1, 1, 2, 3, 3, 3, 3 }, { 1, 1, 1, 1, 3, 3, 3, 3, 3 },
                         referrals.data(), referrals.size());
    }

    /* Test with complex substitutions. */
    {
        vector<LookupSubtable *> referrals = {
            &createSingleSubst(pool, { 1, 2, 3, 4, 5, 6 }, 1),
            &createMultipleSubst(pool, { {2, { 4, 5, 6 }} }),
            &createLigatureSubst(pool, { {{ 1, 4 }, 10}, {{ 6, 4 }, 20} })
        };
        ChainContextSubtable &subtable = createChainContext(pool,
            { { 1 }, { 1 }, { 1 } },
            { { 1 }, { 2 }, { 3 } },
            { { 3 }, { 3 }, { 3 } },
            { { 2, 1 }, { 1, 2 }, { 3, 3 }, { 0, 3 }, { 1, 1 } }
        );
        testSubstitution(subtable,
                         {  1, 1, 1, 1, 2, 3, 3, 3, 3 }, { 1, 1, 1, 10, 6, 20, 3, 3, 3 },
                         referrals.data(), referrals.size());
    }
}
