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

#include <cassert>
#include <cstddef>

extern "C" {
#include <Source/SFPattern.h>
#include <Source/SFPatternBuilder.h>
}

#include "Utilities/SFPattern+Testing.h"
#include "PatternTester.h"

using namespace SheenFigure::Tester;

PatternTester::PatternTester()
{
}

void PatternTester::testNoFeatures()
{
    SFPatternRef pattern = SFPatternCreate();

    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);

    SFFont font;
    SFPatternBuilderSetFont(&builder, &font);
    SFPatternBuilderSetScript(&builder, SFTagMake('a', 'r', 'a', 'b'), SFTextDirectionRightToLeft);
    SFPatternBuilderSetLanguage(&builder, SFTagMake('U', 'R', 'D', 'U'));
    SFPatternBuilderBuild(&builder);

    SFPatternBuilderFinalize(&builder);

    SFPattern expected = {
        .font = &font,
        .featureTags = { NULL, 0 },
        .featureUnits = { NULL, 0, 0 },
        .scriptTag = SFTagMake('a', 'r', 'a', 'b'),
        .languageTag = SFTagMake('U', 'R', 'D', 'U'),
        .defaultDirection = SFTextDirectionRightToLeft,
    };
    assert(SFPatternEqualToPattern(pattern, &expected));

    SFPatternRelease(pattern);
}

void PatternTester::testDistinctFeatures()
{
    /* Test with only substitution features. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 1, 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 2, 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'l', 'i', 'g'), 3, 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            SFTagMake('c', 'c', 'm', 'p'),
            SFTagMake('l', 'i', 'g', 'a'),
            SFTagMake('c', 'l', 'i', 'g'),
        };
        SFFeatureUnit expectedUnits[] = {
            { { NULL, 0 }, { 0, 1 }, 0x01 },
            { { NULL, 0 }, { 1, 1 }, 0x02 },
            { { NULL, 0 }, { 2, 1 }, 0x04 },
        };
        SFPattern expectedPattern = {
            .font = NULL,
            .featureTags = { expectedTags, 3 },
            .featureUnits = { expectedUnits, 3, 0 },
            .scriptTag = 0,
            .languageTag = 0,
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test with only positioning features. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

        SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 1, 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('k', 'e', 'r', 'n'), 2, 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('m', 'a', 'r', 'k'), 3, 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            SFTagMake('d', 'i', 's', 't'),
            SFTagMake('k', 'e', 'r', 'n'),
            SFTagMake('m', 'a', 'r', 'k'),
        };
        SFFeatureUnit expectedUnits[] = {
            { { NULL, 0 }, { 0, 1 }, 0x01 },
            { { NULL, 0 }, { 1, 1 }, 0x02 },
            { { NULL, 0 }, { 2, 1 }, 0x04 },
        };
        SFPattern expectedPattern = {
            .font = NULL,
            .featureTags = { expectedTags, 3 },
            .featureUnits = { expectedUnits, 0, 3 },
            .scriptTag = 0,
            .languageTag = 0,
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }
}

void PatternTester::testSimultaneousFeatures()
{
    SFPatternRef pattern = SFPatternCreate();

    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);

    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

    SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 1, 0x01);
    SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 2, 0x02);
    SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'l', 'i', 'g'), 3, 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

    SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 1, 0x01);
    SFPatternBuilderAddFeature(&builder, SFTagMake('k', 'e', 'r', 'n'), 2, 0x02);
    SFPatternBuilderAddFeature(&builder, SFTagMake('m', 'a', 'r', 'k'), 3, 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBuild(&builder);
    SFPatternBuilderFinalize(&builder);

    SFTag expectedTags[] = {
        SFTagMake('c', 'c', 'm', 'p'),
        SFTagMake('l', 'i', 'g', 'a'),
        SFTagMake('c', 'l', 'i', 'g'),
        SFTagMake('d', 'i', 's', 't'),
        SFTagMake('k', 'e', 'r', 'n'),
        SFTagMake('m', 'a', 'r', 'k'),
    };
    SFFeatureUnit expectedUnits[] = {
        { { NULL, 0 }, { 0, 3 }, (0x01 | 0x02 | 0x04) },
        { { NULL, 0 }, { 3, 3 }, (0x01 | 0x02 | 0x04) },
    };
    SFPattern expectedPattern = {
        .font = NULL,
        .featureTags = { expectedTags, 6 },
        .featureUnits = { expectedUnits, 1, 1 },
        .scriptTag = 0,
        .languageTag = 0,
        .defaultDirection = SFTextDirectionLeftToRight,
    };
    assert(SFPatternEqualToPattern(pattern, &expectedPattern));

    SFPatternRelease(pattern);
}

void PatternTester::testLookupIndexSorting()
{
    /* Test with no index collision. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 1, 0);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 0);
        SFPatternBuilderAddLookup(&builder, 2);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 1);

        SFPatternBuilderMakeFeatureUnit(&builder);
        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

        SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 2, 0);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 5);
        SFPatternBuilderAddLookup(&builder, 6);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 8);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBuild(&builder);
        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            SFTagMake('c', 'c', 'm', 'p'),
            SFTagMake('d', 'i', 's', 't'),
        };
        SFLookupInfo expectedSubLookup[] = { {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1} };
        SFLookupInfo expectedPosLookup[] = { {4, 2}, {5, 2}, {6, 2}, {7, 2}, {8, 2} };
        SFFeatureUnit expectedUnits[] = {
            { { expectedSubLookup, 5 }, { 0, 1 }, 0x00 },
            { { expectedPosLookup, 5 }, { 1, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            .font = NULL,
            .featureTags = { expectedTags, 2 },
            .featureUnits = { expectedUnits, 1, 1 },
            .scriptTag = 0,
            .languageTag = 0,
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));

        SFPatternRelease(pattern);
    }

    /* Test with index collision in feature unit. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 1, 0);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 5);
        SFPatternBuilderAddLookup(&builder, 1);
        SFPatternBuilderAddLookup(&builder, 0);

        SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 2, 0);
        SFPatternBuilderAddLookup(&builder, 2);
        SFPatternBuilderAddLookup(&builder, 1);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 6);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBuild(&builder);
        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            SFTagMake('c', 'c', 'm', 'p'),
            SFTagMake('l', 'i', 'g', 'a'),
        };
        SFLookupInfo expectedSubLookup[] = { {0, 1}, {1, 2}, {2, 2}, {3, 1},
                                             {4, 2}, {5, 1}, {6, 2}, {7, 2} };
        SFFeatureUnit expectedUnits[] = {
            { { expectedSubLookup, 8 }, { 0, 2 }, 0x00 }
        };
        SFPattern expectedPattern = {
            .font = NULL,
            .featureTags = { expectedTags, 2 },
            .featureUnits = { expectedUnits, 1, 0 },
            .scriptTag = 0,
            .languageTag = 0,
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        assert(SFPatternEqualToPattern(pattern, &expectedPattern));
        
        SFPatternRelease(pattern);
    }
}

void PatternTester::test()
{
    testNoFeatures();
    testDistinctFeatures();
    testSimultaneousFeatures();
    testLookupIndexSorting();
}
