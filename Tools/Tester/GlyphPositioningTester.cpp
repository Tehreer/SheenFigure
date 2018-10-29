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

#include "OpenType/Builder.h"
#include "OpenType/Common.h"
#include "OpenType/GPOS.h"
#include "TextProcessorTester.h"

using namespace std;
using namespace SheenFigure::Tester;
using namespace SheenFigure::Tester::OpenType;

void TextProcessorTester::testSinglePositioning()
{
    Builder builder;

    /* Test the first format. */
    {
        /* Test with unmatching glyph. */
        testPositioning(builder.createSinglePos({ 0 }, builder.createValueRecord({ -100, -200, -300, 0 })),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with empty value record. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ 0, 0, 0, 0 })),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with negative values. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ -100, -200, -300, 0 })),
                        { 1 }, { {-100, -200} }, { -300 });
        /* Test with positive values. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ 100, 200, 300, 0 })),
                        { 1 }, { {100, 200} }, { 300 });
        /* Test with device adjustments. */
        testPositioning(builder.createSinglePos({ 1 }, builder.createValueRecord({ -100, 0, 100, 0 }, {
                            &builder.createDevice({8, 10}, { -10, -20, -30 }),
                            &builder.createDevice({8, 10}, { 10, 20, 30 }),
                            &builder.createDevice({8, 10}, { 40, 50, 60 }),
                            nullptr
                        })),
                        { 1 }, { {-110, 30} }, { 140 });
    }

    /* Test the second format. */
    {
        /* Test with unmatching glyph. */
        testPositioning(builder.createSinglePos({ 0 },
                        { builder.createValueRecord({ -100, -200, -300, 0 }) }),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with empty value record. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ 0, 0, 0, 0 }) }),
                        { 1 }, { {0, 0} }, { 0 });
        /* Test with negative values. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ -100, -200, -300, 0 }) }),
                        { 1 }, { {-100, -200} }, { -300 });
        /* Test with positive values. */
        testPositioning(builder.createSinglePos({ 1 },
                        { builder.createValueRecord({ 100, 200, 300, 0 }) }),
                        { 1 }, { {100, 200} }, { 300 });
        /* Test with device adjustments. */
        testPositioning(builder.createSinglePos({ 1 }, {
                            builder.createValueRecord({ -100, 0, 100, 0 }, {
                                &builder.createDevice({8, 10}, { -10, -20, -30 }),
                                &builder.createDevice({8, 10}, { 10, 20, 30 }),
                                &builder.createDevice({8, 10}, { 40, 50, 60 }),
                                nullptr
                            })
                        }),
                        { 1 }, { {-110, 30} }, { 140 });
    }
}

void TextProcessorTester::testPairPositioning()
{
    Builder builder;

    ValueRecord &empty = builder.createValueRecord({ 0, 0, 0, 0 });
    ValueRecord &positive1 = builder.createValueRecord({ 100, 200, 300, 400 });
    ValueRecord &positive2 = builder.createValueRecord({ 600, 700, 800, 900 });
    ValueRecord &negative1 = builder.createValueRecord({ -900, -800, -700, -600 });
    ValueRecord &negative2 = builder.createValueRecord({ -400, -300, -200, -100 });
    ValueRecord &device1 = builder.createValueRecord({ 100, 200, 300, 400 }, {
        &builder.createDevice({8, 10}, { -10, -20, -30 }),
        &builder.createDevice({8, 10}, { 10, 20, 30 }),
        &builder.createDevice({8, 10}, { 40, 50, 60 }),
        nullptr
    });
    ValueRecord &device2 = builder.createValueRecord({ -100, -200, -300, -400 }, {
        &builder.createDevice({8, 10}, { 10, 20, 30 }),
        &builder.createDevice({8, 10}, { 40, 50, 60 }),
        &builder.createDevice({8, 10}, { -10, -20, -30 }),
        nullptr
    });

    /* Test the first format. */
    {
        /* Test with first unmatching glyph. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with second unmatching glyph. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with empty values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, empty, empty }
                        }),
                        { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with positive values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 }
                        }),
                        { 1, 2 }, { {100, 200}, {600, 700} }, { 300, 800 });
        /* Test with negative values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, negative1, negative2 }
                        }),
                        { 1, 2 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test with device values. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, device1, device2 }
                        }),
                        { 1, 2 }, { {90, 230}, {-90, -140} }, { 340, -310 });
        /* Test by letting middle pair match in a single pair set. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 1, 3, positive2, negative1 },
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 1, 3 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in a single pair set. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 1, 3, positive2, negative1 },
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 1, 4 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 3, 4, positive2, negative1 },
                            pair_rule { 5, 6, negative1, negative2 }
                        }),
                        { 3, 4 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({
                            pair_rule { 1, 2, positive1, positive2 },
                            pair_rule { 3, 4, positive2, negative1 },
                            pair_rule { 5, 6, negative1, negative2 }
                        }),
                        { 5, 6 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
    }

    /* Test the second format. */
    {
        reference_wrapper<ClassDefTable> classDefs[] = {
            builder.createClassDef(11, 3, { 1, 2, 3 }),
            builder.createClassDef(21, 3, { 4, 5, 6 }),
        };

        /* Test with first unmatching glyph. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 0, 21 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with second unmatching glyph. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 11, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with empty values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, empty, empty }
                        }),
                        { 11, 21 }, { {0, 0}, {0, 0} }, { 0, 0 });
        /* Test with positive values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 }
                        }),
                        { 11, 21 }, { {100, 200}, {600, 700} }, { 300, 800 });
        /* Test with negative values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, negative1, negative2 }
                        }),
                        { 11, 21 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test with device values. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, device1, device2 }
                        }),
                        { 11, 21 }, { {90, 230}, {-90, -140} }, { 340, -310 });
        /* Test by letting middle pair match in a single class set. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 1, 5, positive2, negative1 },
                            pair_rule { 1, 6, negative1, negative2 }
                        }),
                        { 11, 22 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in a single class set. */
        testPositioning(builder.createPairPos({ 11 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 1, 5, positive2, negative1 },
                            pair_rule { 1, 6, negative1, negative2 }
                        }),
                        { 11, 23 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
        /* Test by letting middle pair match in multiple class sets. */
        testPositioning(builder.createPairPos({ 11, 12, 13 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 2, 5, positive2, negative1 },
                            pair_rule { 3, 6, negative1, negative2 }
                        }),
                        { 12, 22 }, { {600, 700}, {-900, -800} }, { 800, -700 });
        /* Test by letting last pair match in multiple pair sets. */
        testPositioning(builder.createPairPos({ 11, 12, 13 }, classDefs, {
                            pair_rule { 1, 4, positive1, positive2 },
                            pair_rule { 2, 5, positive2, negative1 },
                            pair_rule { 3, 6, negative1, negative2 }
                        }),
                        { 13, 23 }, { {-900, -800}, {-400, -300} }, { -700, -200 });
    }
}

void TextProcessorTester::testCursivePositioning()
{
    Builder builder;

    /* Test with first unmatching glyph. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, &builder.createAnchor(100, 200)}},
                        {2, {&builder.createAnchor(300, 400), nullptr}}
                    }),
                    { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with second unmatching glyph. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, &builder.createAnchor(100, 200)}},
                        {2, {&builder.createAnchor(300, 400), nullptr}}
                    }),
                    { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with no anchor. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, nullptr}},
                        {2, {nullptr, nullptr}}
                    }),
                    { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with non matching anchors. */
    testPositioning(builder.createCursivePos({
                        {1, {&builder.createAnchor(100, 200), nullptr}},
                        {2, {nullptr, &builder.createAnchor(300, 400)}}
                    }),
                    { 1, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with ascending glyph order. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, &builder.createAnchor(100, 200)}},
                        {2, {&builder.createAnchor(300, 400), nullptr}}
                    }),
                    { 1, 2 }, { {0, 0}, {-300, -200} }, { 100, -300 });
    /* Test with dscending glyph order. */
    testPositioning(builder.createCursivePos({
                        {1, {&builder.createAnchor(300, 400), nullptr}},
                        {2, {nullptr, &builder.createAnchor(100, 200)}}
                    }),
                    { 2, 1 }, { {0, 0}, {-300, -200} }, { 100, -300 });
    /* Test with LTR cursive chain. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, &builder.createAnchor(-800, -700)}},
                        {2, {&builder.createAnchor(-600, -500), &builder.createAnchor(-400, -300)}},
                        {3, {&builder.createAnchor(-200, -100), &builder.createAnchor(0, 100)}},
                        {4, {&builder.createAnchor(200, 300), &builder.createAnchor(400, 500)}},
                        {5, {&builder.createAnchor(600, 700), nullptr}},
                    }),
                    { 1, 2, 3, 4, 5 },
                    { {0, 0}, {600, -200}, {200, -400}, {-200, -600}, {-600, -800} },
                    { -800, 200, 200, 200, -600 });
    /* Test with RTL cursive chain. */
    testPositioning(builder.createCursivePos({
                        {1, {nullptr, &builder.createAnchor(-800, -700)}},
                        {2, {&builder.createAnchor(-600, -500), &builder.createAnchor(-400, -300)}},
                        {3, {&builder.createAnchor(-200, -100), &builder.createAnchor(0, 100)}},
                        {4, {&builder.createAnchor(200, 300), &builder.createAnchor(400, 500)}},
                        {5, {&builder.createAnchor(600, 700), nullptr}},
                    }),
                    { 1, 2, 3, 4, 5 },
                    { {800, 800}, {400, 600}, {0, 400}, {-400, 200}, {0, 0} },
                    { 800, -200, -200, -200, 600 }, { }, true);
}

void TextProcessorTester::testMarkToBasePositioning()
{
    Builder builder;

    /* Test with unmatching mark 1 glyph. */
    testPositioning(builder.createMarkToBasePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with unmatching mark 2 glyph. */
    testPositioning(builder.createMarkToBasePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with a mark 2 belonging to zero class. */
    testPositioning(builder.createMarkToBasePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with a mark 2 belonging to a non-zero class. */
    testPositioning(builder.createMarkToBasePos(2, {
                        {2, {1, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(0, 0), builder.createAnchor(900, 800) }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with middle matching rules. */
    testPositioning(builder.createMarkToBasePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { builder.createAnchor(900, 800) }},
                        {3, { builder.createAnchor(700, 600) }},
                        {5, { builder.createAnchor(550, 525) }},
                    }),
                    { 3, 4 }, { {0, 0}, {400, 200} }, { 0, 0 });
    /* Test with last matching rules. */
    testPositioning(builder.createMarkToBasePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { builder.createAnchor(900, 800) }},
                        {3, { builder.createAnchor(700, 600) }},
                        {5, { builder.createAnchor(550, 525) }},
                    }),
                    { 5, 6 }, { {0, 0}, {100, 50} }, { 0, 0 });
}

void TextProcessorTester::testMarkToLigaturePositioning()
{
    Builder builder;

    /* Test with unmatching ligature glyph. */
    testPositioning(builder.createMarkToLigaturePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { { builder.createAnchor(900, 800) } }}
                    }),
                    { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with unmatching mark glyph. */
    testPositioning(builder.createMarkToLigaturePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { { builder.createAnchor(900, 800) } }}
                    }),
                    { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with a mark belonging to zero class. */
    testPositioning(builder.createMarkToLigaturePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { { builder.createAnchor(900, 800) } }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with a mark belonging to a non-zero class. */
    testPositioning(builder.createMarkToLigaturePos(2, {
                        {2, {1, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { { builder.createAnchor(0, 0), builder.createAnchor(900, 800) } }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with middle matching rules. */
    testPositioning(builder.createMarkToLigaturePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { { builder.createAnchor(900, 800) } }},
                        {3, { { builder.createAnchor(700, 600) } }},
                        {5, { { builder.createAnchor(550, 525) } }},
                    }),
                    { 3, 4 }, { {0, 0}, {400, 200} }, { 0, 0 });
    /* Test with last matching rules. */
    testPositioning(builder.createMarkToLigaturePos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { { builder.createAnchor(900, 800) } }},
                        {3, { { builder.createAnchor(700, 600) } }},
                        {5, { { builder.createAnchor(550, 525) } }},
                    }),
                    { 5, 6 }, { {0, 0}, {100, 50} }, { 0, 0 });
}

void TextProcessorTester::testMarkToMarkPositioning()
{
    Builder builder;

    /* Test with unmatching base glyph. */
    testPositioning(builder.createMarkToMarkPos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 0, 2 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with unmatching mark glyph. */
    testPositioning(builder.createMarkToMarkPos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 1, 0 }, { {0, 0}, {0, 0} }, { 0, 0 });
    /* Test with a mark belonging to zero class. */
    testPositioning(builder.createMarkToMarkPos(1, {
                        {2, {0, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(900, 800) }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with a mark belonging to a non-zero class. */
    testPositioning(builder.createMarkToMarkPos(2, {
                        {2, {1, builder.createAnchor(100, 200)}}
                    }, {
                        {1, { builder.createAnchor(0, 0), builder.createAnchor(900, 800) }}
                    }),
                    { 1, 2 }, { {0, 0}, {800, 600} }, { 0, 0 });
    /* Test with middle matching rules. */
    testPositioning(builder.createMarkToMarkPos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { builder.createAnchor(900, 800) }},
                        {3, { builder.createAnchor(700, 600) }},
                        {5, { builder.createAnchor(550, 525) }},
                    }),
                    { 3, 4 }, { {0, 0}, {400, 200} }, { 0, 0 });
    /* Test with last matching rules. */
    testPositioning(builder.createMarkToMarkPos(1, {
                        {2, {0, builder.createAnchor(100, 200)}},
                        {4, {0, builder.createAnchor(300, 400)}},
                        {6, {0, builder.createAnchor(450, 475)}},
                    }, {
                        {1, { builder.createAnchor(900, 800) }},
                        {3, { builder.createAnchor(700, 600) }},
                        {5, { builder.createAnchor(550, 525) }},
                    }),
                    { 5, 6 }, { {0, 0}, {100, 50} }, { 0, 0 });
}
