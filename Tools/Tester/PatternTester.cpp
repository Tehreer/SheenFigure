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

#include "Utilities/General.h"
#include "Utilities/SFPattern+Testing.h"
#include "PatternTester.h"

using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::Utilities;

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
    SFPatternBuilderSetScript(&builder, tag("arab"), SFTextDirectionRightToLeft);
    SFPatternBuilderSetLanguage(&builder, tag("URDU"));
    SFPatternBuilderBuild(&builder);

    SFPatternBuilderFinalize(&builder);

    SFPattern expected = {
        &font,
        { NULL, 0 },
        { NULL, 0, 0 },
        tag("arab"),
        tag("URDU"),
        SFTextDirectionRightToLeft,
        1
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

        SFPatternBuilderAddFeature(&builder, tag("ccmp"), 1, 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, tag("liga"), 2, 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, tag("clig"), 3, 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            tag("ccmp"),
            tag("liga"),
            tag("clig"),
        };
        SFFeatureUnit expectedUnits[] = {
            { { NULL, 0 }, { 0, 1 }, 0x01 },
            { { NULL, 0 }, { 1, 1 }, 0x02 },
            { { NULL, 0 }, { 2, 1 }, 0x04 },
        };
        SFPattern expectedPattern = {
            NULL,
            { expectedTags, 3 },
            { expectedUnits, 3, 0 },
            0,
            0,
            SFTextDirectionLeftToRight,
            1
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

        SFPatternBuilderAddFeature(&builder, tag("dist"), 1, 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, tag("kern"), 2, 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, tag("mark"), 3, 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFTag expectedTags[] = {
            tag("dist"),
            tag("kern"),
            tag("mark"),
        };
        SFFeatureUnit expectedUnits[] = {
            { { NULL, 0 }, { 0, 1 }, 0x01 },
            { { NULL, 0 }, { 1, 1 }, 0x02 },
            { { NULL, 0 }, { 2, 1 }, 0x04 },
        };
        SFPattern expectedPattern = {
            NULL,
            { expectedTags, 3 },
            { expectedUnits, 0, 3 },
            0,
            0,
            SFTextDirectionLeftToRight,
            1
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

    SFPatternBuilderAddFeature(&builder, tag("ccmp"), 1, 0x01);
    SFPatternBuilderAddFeature(&builder, tag("liga"), 2, 0x02);
    SFPatternBuilderAddFeature(&builder, tag("clig"), 3, 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

    SFPatternBuilderAddFeature(&builder, tag("dist"), 1, 0x01);
    SFPatternBuilderAddFeature(&builder, tag("kern"), 2, 0x02);
    SFPatternBuilderAddFeature(&builder, tag("mark"), 3, 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBuild(&builder);
    SFPatternBuilderFinalize(&builder);

    SFTag expectedTags[] = {
        tag("ccmp"), tag("liga"), tag("clig"),
        tag("dist"), tag("kern"), tag("mark"),
    };
    SFFeatureUnit expectedUnits[] = {
        { { NULL, 0 }, { 0, 3 }, (0x01 | 0x02 | 0x04) },
        { { NULL, 0 }, { 3, 3 }, (0x01 | 0x02 | 0x04) },
    };
    SFPattern expectedPattern = {
        NULL,
        { expectedTags, 6 },
        { expectedUnits, 1, 1 },
        0,
        0,
        SFTextDirectionLeftToRight,
        1
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

        SFPatternBuilderAddFeature(&builder, tag("ccmp"), 1, 0);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 0);
        SFPatternBuilderAddLookup(&builder, 2);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 1);

        SFPatternBuilderMakeFeatureUnit(&builder);
        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

        SFPatternBuilderAddFeature(&builder, tag("dist"), 2, 0);
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
            tag("ccmp"),
            tag("dist"),
        };
        SFLookupInfo expectedLookups[] = {
            {0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1},
            {4, 2}, {5, 2}, {6, 2}, {7, 2}, {8, 2},
        };
        SFFeatureUnit expectedUnits[] = {
            { { &expectedLookups[0], 5 }, { 0, 1 }, 0x00 },
            { { &expectedLookups[5], 5 }, { 1, 1 }, 0x00 },
        };
        SFPattern expectedPattern = {
            NULL,
            { expectedTags, 2 },
            { expectedUnits, 1, 1 },
            0,
            0,
            SFTextDirectionLeftToRight,
            1
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

        SFPatternBuilderAddFeature(&builder, tag("ccmp"), 1, 0);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 5);
        SFPatternBuilderAddLookup(&builder, 1);
        SFPatternBuilderAddLookup(&builder, 0);

        SFPatternBuilderAddFeature(&builder, tag("liga"), 2, 0);
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
            tag("ccmp"),
            tag("liga"),
        };
        SFLookupInfo expectedLookups[] = { {0, 1}, {1, 2}, {2, 2}, {3, 1},
                                           {4, 2}, {5, 1}, {6, 2}, {7, 2} };
        SFFeatureUnit expectedUnits[] = {
            { { expectedLookups, 8 }, { 0, 2 }, 0x00 }
        };
        SFPattern expectedPattern = {
            NULL,
            { expectedTags, 2 },
            { expectedUnits, 1, 0 },
            0,
            0,
            SFTextDirectionLeftToRight,
            1
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
