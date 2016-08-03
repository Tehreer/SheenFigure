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
#include <cstring>

#include "PatternTester.h"

extern "C" {
#include <SheenFigure/Source/SFAssert.h>
#include <SheenFigure/Source/SFPatternBuilder.h>
}

using namespace SheenFigure::Tester;

static SFBoolean SFPatternEqualToPattern(SFPatternRef pattern1, SFPatternRef pattern2)
{
    return (pattern1->font == pattern2->font
            && pattern1->scriptTag == pattern2->scriptTag
            && pattern1->languageTag == pattern2->languageTag
            && pattern1->defaultDirection == pattern2->defaultDirection
            && pattern1->featureTags.count == pattern2->featureTags.count
            && memcmp(pattern1->featureTags.items,
                      pattern2->featureTags.items,
                      pattern1->featureTags.count * sizeof(SFTag)) == 0
            && pattern1->featureUnits.gsub == pattern2->featureUnits.gsub
            && pattern1->featureUnits.gpos == pattern2->featureUnits.gpos
            && memcmp(pattern1->featureUnits.items,
                      pattern2->featureUnits.items,
                      (pattern1->featureUnits.gsub + pattern1->featureUnits.gpos) * sizeof(SFTag)) == 0);
}

PatternTester::PatternTester()
{
}

void PatternTester::testNoFeatures()
{
    /* Test with font only. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFFont font;
        SFPatternBuilderSetFont(&builder, &font);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFPattern expected = {
            .font = &font,
            .featureTags = { NULL, 0 },
            .featureUnits = { NULL, 0, 0 },
            .scriptTag = 0,
            .languageTag = 0,
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        SFAssert(SFPatternEqualToPattern(pattern, &expected));

        SFPatternRelease(pattern);
    }

    /* Test with script only. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderSetScript(&builder, SFTagMake('a', 'r', 'a', 'b'), SFTextDirectionRightToLeft);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFPattern expected = {
            .font = NULL,
            .featureTags = { NULL, 0 },
            .featureUnits = { NULL, 0, 0 },
            .scriptTag = SFTagMake('a', 'r', 'a', 'b'),
            .languageTag = 0,
            .defaultDirection = SFTextDirectionRightToLeft,
        };
        SFAssert(SFPatternEqualToPattern(pattern, &expected));

        SFPatternRelease(pattern);
    }

    /* Test with language only. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderSetLanguage(&builder, SFTagMake('U', 'R', 'D', 'U'));
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFPattern expected = {
            .font = NULL,
            .featureTags = { NULL, 0 },
            .featureUnits = { NULL, 0, 0 },
            .scriptTag = 0,
            .languageTag = SFTagMake('U', 'R', 'D', 'U'),
            .defaultDirection = SFTextDirectionLeftToRight,
        };
        SFAssert(SFPatternEqualToPattern(pattern, &expected));

        SFPatternRelease(pattern);
    }
}

void PatternTester::testDistinctFeatures()
{
    /* Test with only substitution features. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'l', 'i', 'g'), 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFAssert(pattern->font == NULL);
        SFAssert(pattern->featureTags.items != NULL);
        SFAssert(pattern->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
        SFAssert(pattern->featureTags.items[1] == SFTagMake('l', 'i', 'g', 'a'));
        SFAssert(pattern->featureTags.items[2] == SFTagMake('c', 'l', 'i', 'g'));
        SFAssert(pattern->featureTags.count == 3);
        SFAssert(pattern->featureUnits.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.start == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[0].featureMask == 0x01);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[1].coveredRange.start == 1);
        SFAssert(pattern->featureUnits.items[1].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[1].featureMask == 0x02);
        SFAssert(pattern->featureUnits.items[2].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[2].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[2].coveredRange.start == 2);
        SFAssert(pattern->featureUnits.items[2].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[2].featureMask == 0x04);
        SFAssert(pattern->featureUnits.gsub == 3);
        SFAssert(pattern->featureUnits.gpos == 0);
        SFAssert(pattern->scriptTag == 0);
        SFAssert(pattern->languageTag == 0);
        SFAssert(pattern->defaultDirection == SFTextDirectionLeftToRight);

        SFPatternRelease(pattern);
    }

    /* Test with only positioning features. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

        SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 0x01);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('k', 'e', 'r', 'n'), 0x02);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderAddFeature(&builder, SFTagMake('m', 'a', 'r', 'k'), 0x04);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);
        SFPatternBuilderBuild(&builder);

        SFPatternBuilderFinalize(&builder);

        SFAssert(pattern->font == NULL);
        SFAssert(pattern->featureTags.items != NULL);
        SFAssert(pattern->featureTags.items[0] == SFTagMake('d', 'i', 's', 't'));
        SFAssert(pattern->featureTags.items[1] == SFTagMake('k', 'e', 'r', 'n'));
        SFAssert(pattern->featureTags.items[2] == SFTagMake('m', 'a', 'r', 'k'));
        SFAssert(pattern->featureTags.count == 3);
        SFAssert(pattern->featureUnits.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.start == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[0].featureMask == 0x01);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[1].coveredRange.start == 1);
        SFAssert(pattern->featureUnits.items[1].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[1].featureMask == 0x02);
        SFAssert(pattern->featureUnits.items[2].lookupIndexes.items == NULL);
        SFAssert(pattern->featureUnits.items[2].lookupIndexes.count == 0);
        SFAssert(pattern->featureUnits.items[2].coveredRange.start == 2);
        SFAssert(pattern->featureUnits.items[2].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[2].featureMask == 0x04);
        SFAssert(pattern->featureUnits.gsub == 0);
        SFAssert(pattern->featureUnits.gpos == 3);
        SFAssert(pattern->scriptTag == 0);
        SFAssert(pattern->languageTag == 0);
        SFAssert(pattern->defaultDirection == SFTextDirectionLeftToRight);

        SFPatternRelease(pattern);
    }
}

void PatternTester::testSimultaneousFeatures()
{
    SFPatternRef pattern = SFPatternCreate();

    SFPatternBuilder builder;
    SFPatternBuilderInitialize(&builder, pattern);

    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

    SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 0x01);
    SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 0x02);
    SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'l', 'i', 'g'), 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

    SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 0x01);
    SFPatternBuilderAddFeature(&builder, SFTagMake('k', 'e', 'r', 'n'), 0x02);
    SFPatternBuilderAddFeature(&builder, SFTagMake('m', 'a', 'r', 'k'), 0x04);

    SFPatternBuilderMakeFeatureUnit(&builder);
    SFPatternBuilderEndFeatures(&builder);

    SFPatternBuilderBuild(&builder);
    SFPatternBuilderFinalize(&builder);

    SFAssert(pattern->font == NULL);
    SFAssert(pattern->featureTags.items != NULL);
    SFAssert(pattern->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
    SFAssert(pattern->featureTags.items[1] == SFTagMake('l', 'i', 'g', 'a'));
    SFAssert(pattern->featureTags.items[2] == SFTagMake('c', 'l', 'i', 'g'));
    SFAssert(pattern->featureTags.items[3] == SFTagMake('d', 'i', 's', 't'));
    SFAssert(pattern->featureTags.items[4] == SFTagMake('k', 'e', 'r', 'n'));
    SFAssert(pattern->featureTags.items[5] == SFTagMake('m', 'a', 'r', 'k'));
    SFAssert(pattern->featureTags.count == 6);
    SFAssert(pattern->featureUnits.items != NULL);
    SFAssert(pattern->featureUnits.items[0].lookupIndexes.items == NULL);
    SFAssert(pattern->featureUnits.items[0].lookupIndexes.count == 0);
    SFAssert(pattern->featureUnits.items[0].coveredRange.start == 0);
    SFAssert(pattern->featureUnits.items[0].coveredRange.count == 3);
    SFAssert(pattern->featureUnits.items[0].featureMask == (0x01 | 0x02 | 0x04));
    SFAssert(pattern->featureUnits.items[1].lookupIndexes.items == NULL);
    SFAssert(pattern->featureUnits.items[1].lookupIndexes.count == 0);
    SFAssert(pattern->featureUnits.items[1].coveredRange.start == 3);
    SFAssert(pattern->featureUnits.items[1].coveredRange.count == 3);
    SFAssert(pattern->featureUnits.items[1].featureMask == (0x01 | 0x02 | 0x04));
    SFAssert(pattern->featureUnits.gsub == 1);
    SFAssert(pattern->featureUnits.gpos == 1);
    SFAssert(pattern->scriptTag == 0);
    SFAssert(pattern->languageTag == 0);
    SFAssert(pattern->defaultDirection == SFTextDirectionLeftToRight);

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

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 0);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 0);
        SFPatternBuilderAddLookup(&builder, 2);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 1);

        SFPatternBuilderMakeFeatureUnit(&builder);
        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindPositioning);

        SFPatternBuilderAddFeature(&builder, SFTagMake('d', 'i', 's', 't'), 0);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 5);
        SFPatternBuilderAddLookup(&builder, 6);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 8);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBuild(&builder);
        SFPatternBuilderFinalize(&builder);

        SFAssert(pattern->font == NULL);
        SFAssert(pattern->featureTags.items != NULL);
        SFAssert(pattern->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
        SFAssert(pattern->featureTags.items[1] == SFTagMake('d', 'i', 's', 't'));
        SFAssert(pattern->featureTags.count == 2);
        SFAssert(pattern->featureUnits.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[0] == 0);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[1] == 1);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[2] == 2);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[3] == 3);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[4] == 4);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.count == 5);
        SFAssert(pattern->featureUnits.items[0].coveredRange.start == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[0].featureMask == 0);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items != NULL);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items[0] == 4);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items[1] == 5);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items[2] == 6);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items[3] == 7);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.items[4] == 8);
        SFAssert(pattern->featureUnits.items[1].lookupIndexes.count == 5);
        SFAssert(pattern->featureUnits.items[1].coveredRange.start == 1);
        SFAssert(pattern->featureUnits.items[1].coveredRange.count == 1);
        SFAssert(pattern->featureUnits.items[1].featureMask == 0);
        SFAssert(pattern->featureUnits.gsub == 1);
        SFAssert(pattern->featureUnits.gpos == 1);
        SFAssert(pattern->scriptTag == 0);
        SFAssert(pattern->languageTag == 0);
        SFAssert(pattern->defaultDirection == SFTextDirectionLeftToRight);
        
        SFPatternRelease(pattern);
    }

    /* Test with index collision in feature unit. */
    {
        SFPatternRef pattern = SFPatternCreate();

        SFPatternBuilder builder;
        SFPatternBuilderInitialize(&builder, pattern);

        SFPatternBuilderBeginFeatures(&builder, SFFeatureKindSubstitution);

        SFPatternBuilderAddFeature(&builder, SFTagMake('c', 'c', 'm', 'p'), 0);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 3);
        SFPatternBuilderAddLookup(&builder, 5);
        SFPatternBuilderAddLookup(&builder, 1);
        SFPatternBuilderAddLookup(&builder, 0);

        SFPatternBuilderAddFeature(&builder, SFTagMake('l', 'i', 'g', 'a'), 0);
        SFPatternBuilderAddLookup(&builder, 2);
        SFPatternBuilderAddLookup(&builder, 1);
        SFPatternBuilderAddLookup(&builder, 4);
        SFPatternBuilderAddLookup(&builder, 7);
        SFPatternBuilderAddLookup(&builder, 6);
        SFPatternBuilderMakeFeatureUnit(&builder);

        SFPatternBuilderEndFeatures(&builder);

        SFPatternBuilderBuild(&builder);
        SFPatternBuilderFinalize(&builder);

        SFAssert(pattern->font == NULL);
        SFAssert(pattern->featureTags.items != NULL);
        SFAssert(pattern->featureTags.items[0] == SFTagMake('c', 'c', 'm', 'p'));
        SFAssert(pattern->featureTags.items[1] == SFTagMake('l', 'i', 'g', 'a'));
        SFAssert(pattern->featureTags.count == 2);
        SFAssert(pattern->featureUnits.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items != NULL);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[0] == 0);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[1] == 1);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[2] == 2);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[3] == 3);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[4] == 4);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[5] == 5);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[6] == 6);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.items[7] == 7);
        SFAssert(pattern->featureUnits.items[0].lookupIndexes.count == 8);
        SFAssert(pattern->featureUnits.items[0].coveredRange.start == 0);
        SFAssert(pattern->featureUnits.items[0].coveredRange.count == 2);
        SFAssert(pattern->featureUnits.items[0].featureMask == 0);
        SFAssert(pattern->featureUnits.gsub == 1);
        SFAssert(pattern->featureUnits.gpos == 0);
        SFAssert(pattern->scriptTag == 0);
        SFAssert(pattern->languageTag == 0);
        SFAssert(pattern->defaultDirection == SFTextDirectionLeftToRight);
        
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
