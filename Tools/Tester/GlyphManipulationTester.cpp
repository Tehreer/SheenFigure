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

#include <functional>
#include <vector>

#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

void TextProcessorTester::testContextSubtable()
{
    Builder builder;

    vector<LookupSubtable *> simpleReferral = {
        &builder.createSingleSubst({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10)
    };
    vector<LookupSubtable *> complexReferral = {
        &builder.createSingleSubst({ 1, 2, 3, 4, 5, 6 }, 1),
        &builder.createMultipleSubst({ {2, { 4, 5, 6 }} }),
        &builder.createLigatureSubst({ {{ 1, 4 }, 10}, {{ 6, 4 }, 20} })
    };

    /* Test the format 1. */
    {
        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } }
                         }),
                         { 0, 2, 3 }, { 0, 2, 3 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } }
                         }),
                         { 1, 0, 3 }, { 1, 0, 3 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } }
                         }),
                         { 1, 2, 0 }, { 1, 2, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1 }, { {0, 1} } }
                         }),
                         { 1 }, { 11 }, simpleReferral);
        /* Test by providing same input glyph sequence. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 1, 1 }, { {1, 1} } }
                         }),
                         { 1, 1, 1 }, { 1, 11, 1 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {0, 1} } }
                         }),
                         { 1, 2, 3 }, { 11, 2, 3 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {2, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 2, 13 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {0, 1}, {1, 1}, {2, 1} } }
                         }),
                         { 1, 2, 3 }, { 11, 12, 13 }, simpleReferral);
        /* Test by letting middle rule match in a single rule set. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } },
                            rule_context { { 1, 2, 4 }, { {1, 1} } },
                            rule_context { { 1, 2, 5 }, { {1, 1} } },
                         }),
                         { 1, 2, 4 }, { 1, 12, 4 }, simpleReferral);
        /* Test by letting last rule match in a single rule set. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } },
                            rule_context { { 1, 2, 4 }, { {1, 1} } },
                            rule_context { { 1, 2, 5 }, { {1, 1} } },
                         }),
                         { 1, 2, 5 }, { 1, 12, 5 }, simpleReferral);
        /* Test by letting middle rule match in multiple rule sets. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } },
                            rule_context { { 4, 5, 6 }, { {1, 1} } },
                            rule_context { { 7, 8, 9 }, { {1, 1} } },
                         }),
                         { 4, 5, 6 }, { 4, 15, 6 }, simpleReferral);
        /* Test by letting last rule match in multiple rule sets. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {1, 1} } },
                            rule_context { { 4, 5, 6 }, { {1, 1} } },
                            rule_context { { 7, 8, 9 }, { {1, 1} } },
                         }),
                         { 7, 8, 9 }, { 7, 18, 9 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createContext({
                            rule_context { { 1, 2, 3 }, { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} } }
                         }),
                         {  1, 2, 3 }, { 10, 6, 20 }, complexReferral);
    }

    /* Test the format 2. */
    {
        ClassDefTable &classDef = builder.createClassDef({
            class_range { 1, 3, 1 },
            class_range { 4, 6, 2 },
            class_range { 7, 9, 3 },
        });

        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } }
                         }),
                         { 0, 2, 3 }, { 0, 2, 3 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } }
                         }),
                         { 1, 0, 3 }, { 1, 0, 3 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } }
                         }),
                         { 1, 2, 0 }, { 1, 2, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1 }, { {0, 1} } }
                         }),
                         { 1 }, { 11 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {0, 1} } }
                         }),
                         { 1, 2, 3 }, { 11, 2, 3 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {2, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 2, 13 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {0, 1}, {1, 1}, {2, 1} } }
                         }),
                         { 1, 2, 3 }, { 11, 12, 13 }, simpleReferral);
        /* Test by letting middle rule match in a single rule set. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } },
                            rule_context { { 1, 1, 2 }, { {1, 1} } },
                            rule_context { { 1, 1, 3 }, { {1, 1} } },
                         }),
                         { 1, 2, 5 }, { 1, 12, 5 }, simpleReferral);
        /* Test by letting last rule match in a single rule set. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } },
                            rule_context { { 1, 1, 2 }, { {1, 1} } },
                            rule_context { { 1, 1, 3 }, { {1, 1} } },
                         }),
                         { 1, 2, 8 }, { 1, 12, 8 }, simpleReferral);
        /* Test by letting middle rule match in multiple rule sets. */
        testSubstitution(builder.createContext({ 4 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } },
                            rule_context { { 2, 2, 2 }, { {1, 1} } },
                            rule_context { { 3, 3, 3 }, { {1, 1} } },
                         }),
                         { 4, 5, 6 }, { 4, 15, 6 }, simpleReferral);
        /* Test by letting last rule match in multiple rule sets. */
        testSubstitution(builder.createContext({ 7 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {1, 1} } },
                            rule_context { { 2, 2, 2 }, { {1, 1} } },
                            rule_context { { 3, 3, 3 }, { {1, 1} } },
                         }),
                         { 7, 8, 9 }, { 7, 18, 9 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createContext({ 1 }, classDef, {
                            rule_context { { 1, 1, 1 }, { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} } }
                         }),
                         {  1, 2, 3 }, { 10, 6, 20 }, complexReferral);
    }

    /* Test the format 3. */
    {
        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {1, 1} }),
                         { 0, 2, 3 }, { 0, 2, 3 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {1, 1} }),
                         { 1, 0, 3 }, { 1, 0, 3 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {1, 1} }),
                         { 1, 2, 0 }, { 1, 2, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createContext({ { 1 } }, { {0, 1} }),
                         { 1 }, { 11 }, simpleReferral);
        /* Test by providing same input glyph sequence. */
        testSubstitution(builder.createContext({ { 1 }, { 1 }, { 1 } }, { {1, 1} }),
                         { 1, 1, 1 }, { 1, 11, 1 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {0, 1} }),
                         { 1, 2, 3 }, { 11, 2, 3 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {1, 1} }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {2, 1} }),
                         { 1, 2, 3 }, { 1, 2, 13 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {0, 1}, {1, 1}, {2, 1} }),
                         { 1, 2, 3 }, { 11, 12, 13 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createContext({ { 1 }, { 2 }, { 3 } }, { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} }),
                         {  1, 2, 3 }, { 10, 6, 20 }, complexReferral);
    }
}

void TextProcessorTester::testChainContextSubtable()
{
    Builder builder;

    vector<LookupSubtable *> simpleReferral = {
        &builder.createSingleSubst({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }, 10)
    };
    vector<LookupSubtable *> complexReferral = {
        &builder.createSingleSubst({ 1, 2, 3, 4, 5, 6 }, 1),
        &builder.createMultipleSubst({ {2, { 4, 5, 6 }} }),
        &builder.createLigatureSubst({ {{ 1, 4 }, 10}, {{ 6, 4 }, 20} })
    };

    /* Test the format 1. */
    {
        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1, 2, 3 }, { 31 }, { {1, 1} } }
                         }),
                         { 21, 0, 2, 3, 31 }, { 21, 0, 2, 3, 31 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1, 2, 3 }, { 31 }, { {1, 1} } }
                         }),
                         { 21, 1, 0, 3, 31 }, { 21, 1, 0, 3, 31 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1, 2, 3 }, { 31 }, { {1, 1} } }
                         }),
                         { 21, 1, 2, 0, 31 }, { 21, 1, 2, 0, 31 }, simpleReferral);
        /* Test with unmatching first backtrack glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1 }, { 31 }, { {0, 1} } }
                         }),
                         { 0, 22, 23, 1, 31 }, { 0, 22, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching middle backtrack glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1 }, { 31 }, { {0, 1} } }
                         }),
                         { 21, 0, 23, 1, 31 }, { 21, 0, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching last backtrack glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1 }, { 31 }, { {0, 1} } }
                         }),
                         { 21, 22, 0, 1, 31 }, { 21, 22, 0, 1, 31 }, simpleReferral);
        /* Test with unmatching first lookahead glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1 }, { 31, 32, 33 }, { {0, 1} } }
                         }),
                         { 21, 1, 0, 32, 33 }, { 21, 1, 0, 32, 33 }, simpleReferral);
        /* Test with unmatching middle lookahead glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1 }, { 31, 32, 33 }, { {0, 1} } }
                         }),
                         { 21, 1, 31, 0, 33 }, { 21, 1, 31, 0, 33 }, simpleReferral);
        /* Test with unmatching last lookahead glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21 }, { 1 }, { 31, 32, 33 }, { {0, 1} } }
                         }),
                         { 21, 1, 31, 32, 0 }, { 21, 1, 31, 32, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1 }, { 31, 32, 33 }, { {0, 1} } }
                         }),
                         { 21, 22, 23, 1, 31, 32, 33 }, { 21, 22, 23, 11, 31, 32, 33 }, simpleReferral);
        /* Test by providing same input glyph sequence. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 1, 1 }, { 31, 32, 33 }, { {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 1, 1, 31, 32, 33 }, { 21, 22, 23, 1, 11, 1, 31, 32, 33 }, simpleReferral);
        /* Test by providing no backtrack and lookahead glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { }, { 1, 2, 3 }, { }, { {1, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {0, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 2, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 12, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {2, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 2, 13, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {0, 1}, {1, 1}, {2, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 12, 13, 31, 32, 33 }, simpleReferral);
        /* Test by letting middle rule match in a single rule set. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {1, 1} } },
                            rule_chain_context { { 24, 25, 26 }, { 1, 2, 4 }, { 34, 35, 36 }, { {1, 1} } },
                            rule_chain_context { { 27, 28, 29 }, { 1, 2, 5 }, { 37, 38, 39 }, { {1, 1} } },
                         }),
                         { 24, 25, 26, 1, 2, 4, 34, 35, 36 }, { 24, 25, 26, 1, 12, 4, 34, 35, 36 }, simpleReferral);
        /* Test by letting last rule match in a single rule set. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {1, 1} } },
                            rule_chain_context { { 24, 25, 26 }, { 1, 2, 4 }, { 34, 35, 36 }, { {1, 1} } },
                            rule_chain_context { { 27, 28, 29 }, { 1, 2, 5 }, { 37, 38, 39 }, { {1, 1} } },
                         }),
                         { 27, 28, 29, 1, 2, 5, 37, 38, 39 }, { 27, 28, 29, 1, 12, 5, 37, 38, 39 }, simpleReferral);
        /* Test by letting middle rule match in multiple rule sets. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {1, 1} } },
                            rule_chain_context { { 24, 25, 26 }, { 4, 5, 6 }, { 34, 35, 36 }, { {1, 1} } },
                            rule_chain_context { { 27, 28, 29 }, { 7, 8, 9 }, { 37, 38, 39 }, { {1, 1} } },
                         }),
                         { 24, 25, 26, 4, 5, 6, 34, 35, 36 }, { 24, 25, 26, 4, 15, 6, 34, 35, 36 }, simpleReferral);
        /* Test by letting last rule match in multiple rule sets. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {1, 1} } },
                            rule_chain_context { { 24, 25, 26 }, { 4, 5, 6 }, { 34, 35, 36 }, { {1, 1} } },
                            rule_chain_context { { 27, 28, 29 }, { 7, 8, 9 }, { 37, 38, 39 }, { {1, 1} } },
                         }),
                         { 27, 28, 29, 7, 8, 9, 37, 38, 39 }, { 27, 28, 29, 7, 18, 9, 37, 38, 39 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createChainContext({
                            rule_chain_context { { 21, 22, 23 }, { 1, 2, 3 }, { 31, 32, 33 }, { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 10, 6, 20, 31, 32, 33 }, complexReferral);
    }

    /* Test the format 2. */
    {
        reference_wrapper<ClassDefTable> classDefs[] = {
            builder.createClassDef(21, 9, { 1, 2, 3, 1, 2, 3, 1, 2, 3 }),
            builder.createClassDef(1, 9, { 4, 5, 6, 4, 5, 6, 4, 5, 6 }),
            builder.createClassDef(31, 9, { 7, 8, 9, 7, 8, 9, 7, 8, 9 }),
        };

        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1 }, { 4, 5, 6 }, { 7 }, { {1, 1} } }
                         }),
                         { 21, 0, 2, 3, 31 }, { 21, 0, 2, 3, 31 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1 }, { 4, 5, 6 }, { 7 }, { {1, 1} } }
                         }),
                         { 21, 1, 0, 3, 31 }, { 21, 1, 0, 3, 31 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 4 }, { 4, 5, 6 }, { 7 }, { {1, 1} } }
                         }),
                         { 21, 1, 2, 0, 31 }, { 21, 1, 2, 0 , 31}, simpleReferral);
        /* Test with unmatching first backtrack glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4 }, { 7 }, { {0, 1} } }
                         }),
                         { 0, 22, 23, 1, 31 }, { 0, 22, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching middle backtrack glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4 }, { 7 }, { {0, 1} } }
                         }),
                         { 21, 0, 23, 1, 31 }, { 21, 0, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching last backtrack glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4 }, { 7 }, { {0, 1} } }
                         }),
                         { 21, 22, 0, 1, 31 }, { 21, 22, 0, 1, 31 }, simpleReferral);
        /* Test with unmatching first lookahead glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1 }, { 4 }, { 7, 8, 9 }, { {0, 1} } }
                         }),
                         { 21, 1, 0, 32, 33 }, { 21, 1, 0, 32, 33 }, simpleReferral);
        /* Test with unmatching middle lookahead glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1 }, { 4 }, { 7, 8, 9 }, { {0, 1} } }
                         }),
                         { 21, 1, 31, 0, 33 }, { 21, 1, 31, 0, 33 }, simpleReferral);
        /* Test with unmatching last lookahead glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1 }, { 4 }, { 7, 8, 9 }, { {0, 1} } }
                         }),
                         { 21, 1, 31, 32, 0 }, { 21, 1, 31, 32, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4 }, { 7, 8, 9 }, { {0, 1} } }
                         }),
                         { 21, 22, 23, 1, 31, 32, 33 }, { 21, 22, 23, 11, 31, 32, 33 }, simpleReferral);
        /* Test by providing same input glyph sequence. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 4, 4 }, { 7, 8, 9 }, { {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 1, 1, 31, 32, 33 }, { 21, 22, 23, 1, 11, 1, 31, 32, 33 }, simpleReferral);
        /* Test by providing no backtrack and lookahead glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { }, { 4, 5, 6 }, { }, { {1, 1} } }
                         }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {0, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 2, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 12, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {2, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 2, 13, 31 ,32, 33 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {0, 1}, {1, 1}, {2, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 12, 13, 31, 32, 33 }, simpleReferral);
        /* Test by letting middle rule match in a single rule set. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 4 }, { 7, 8, 9 }, { {1, 1} } },
                            rule_chain_context { { 2, 3, 1 }, { 4, 5, 5 }, { 8, 9, 7 }, { {1, 1} } },
                            rule_chain_context { { 3, 1, 2 }, { 4, 5, 6 }, { 9, 7, 8 }, { {1, 1} } },
                         }),
                         { 25, 26, 27, 1, 2, 5, 35, 36, 37 }, { 25, 26, 27, 1, 12, 5, 35, 36, 37 }, simpleReferral);
        /* Test by letting last rule match in a single rule set. */
        testSubstitution(builder.createChainContext({ 1, 4, 7 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 4 }, { 7, 8, 9 }, { {1, 1} } },
                            rule_chain_context { { 2, 3, 1 }, { 4, 5, 5 }, { 8, 9, 7 }, { {1, 1} } },
                            rule_chain_context { { 3, 1, 2 }, { 4, 5, 6 }, { 9, 7, 8 }, { {1, 1} } },
                         }),
                         { 26, 27, 28, 1, 2, 6, 36, 37, 38 }, { 26, 27, 28, 1, 12, 6, 36, 37, 38 }, simpleReferral);
        /* Test by letting middle rule match in multiple rule sets. */
        testSubstitution(builder.createChainContext({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {1, 1} } },
                            rule_chain_context { { 2, 3, 1 }, { 5, 6, 4 }, { 8, 9, 7 }, { {1, 1} } },
                            rule_chain_context { { 3, 1, 2 }, { 6, 4, 5 }, { 9, 7, 8 }, { {1, 1} } },
                         }),
                         { 25, 26, 27, 5, 6, 7, 35, 36, 37 }, { 25, 26, 27, 5, 16, 7, 35, 36, 37 }, simpleReferral);
        /* Test by letting last rule match in multiple rule sets. */
        testSubstitution(builder.createChainContext({ 1, 2, 3, 4, 5, 6, 7, 8, 9 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {1, 1} } },
                            rule_chain_context { { 2, 3, 1 }, { 5, 6, 4 }, { 8, 9, 7 }, { {1, 1} } },
                            rule_chain_context { { 3, 1, 2 }, { 6, 4, 5 }, { 9, 7, 8 }, { {1, 1} } },
                         }),
                         { 26, 27, 28, 6, 7, 8, 36, 37, 38 }, { 26, 27, 28, 6, 17, 8, 36, 37, 38 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createChainContext({ 1 }, classDefs, {
                            rule_chain_context { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 }, { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} } }
                         }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 10, 6, 20, 31, 32, 33 }, complexReferral);
    }

    /* Test the format 3. */
    {
        /* Test with unmatching first input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 } },
                            { {1, 1} }),
                         { 21, 0, 2, 3, 31 }, { 21, 0, 2, 3, 31 }, simpleReferral);
        /* Test with unmatching middle input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 } },
                            { {1, 1} }),
                         { 21, 1, 0, 3, 31 }, { 21, 1, 0, 3, 31 }, simpleReferral);
        /* Test with unmatching last input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 } },
                            { {1, 1} }),
                         { 21, 1, 2, 0, 31 }, { 21, 1, 2, 0, 31 }, simpleReferral);
        /* Test with unmatching first backtrack glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 } },
                            { { 31 } },
                            { {0, 1} }),
                         { 0, 22, 23, 1, 31 }, { 0, 22, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching middle backtrack glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 } },
                            { { 31 } },
                            { {0, 1} }),
                         { 21, 0, 23, 1, 31 }, { 21, 0, 23, 1, 31 }, simpleReferral);
        /* Test with unmatching last backtrack glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 } },
                            { { 31 } },
                            { {0, 1} }),
                         { 21, 22, 0, 1, 31 }, { 21, 22, 0, 1, 31 }, simpleReferral);
        /* Test with unmatching first lookahead glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1} }),
                         { 21, 1, 0, 32, 33 }, { 21, 1, 0, 32, 33 }, simpleReferral);
        /* Test with unmatching middle lookahead glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1} }),
                         { 21, 1, 31, 0, 33 }, { 21, 1, 31, 0, 33 }, simpleReferral);
        /* Test with unmatching last lookahead glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 } },
                            { { 1 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1} }),
                         { 21, 1, 31, 32, 0 }, { 21, 1, 31, 32, 0 }, simpleReferral);
        /* Test by providing single input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1} }),
                         { 21, 22, 23, 1, 31, 32, 33 }, { 21, 22, 23, 11, 31, 32, 33 }, simpleReferral);
        /* Test by providing same input glyph sequence. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 1 }, { 1 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {1, 1} }),
                         { 21, 22, 23, 1, 1, 1, 31, 32, 33 }, { 21, 22, 23, 1, 11, 1, 31, 32, 33 }, simpleReferral);
        /* Test by providing no backtrack and lookahead glyph. */
        testSubstitution(builder.createChainContext(
                            { },
                            { { 1 }, { 2 }, { 3 } },
                            { },
                            { {1, 1} }),
                         { 1, 2, 3 }, { 1, 12, 3 }, simpleReferral);
        /* Test by applying lookup on first input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1} }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 2, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on middle input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {1, 1} }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 12, 3, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on last input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {2, 1} }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 1, 2, 13, 31, 32, 33 }, simpleReferral);
        /* Test by applying lookup on each input glyph. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {0, 1}, {1, 1}, {2, 1} }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 11, 12, 13, 31, 32, 33 }, simpleReferral);
        /* Test by applying complex lookups on input glyphs. */
        testSubstitution(builder.createChainContext(
                            { { 21 }, { 22 }, { 23 } },
                            { { 1 }, { 2 }, { 3 } },
                            { { 31 }, { 32 }, { 33 } },
                            { {2, 1}, {1, 2}, {3, 3}, {0, 3}, {1, 1} }),
                         { 21, 22, 23, 1, 2, 3, 31, 32, 33 }, { 21, 22, 23, 10, 6, 20, 31, 32, 33 }, complexReferral);
    }
}
