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

#include <map>
#include <string>

extern "C" {
#include <SFGeneralCategory.h>
#include <SFJoiningType.h>
}

#include "Convert.h"

using namespace std;
using namespace SheenFigure::Tester::Utilities;

static map<SFGeneralCategory, string> createGeneralCategoryMap() {
    map<SFGeneralCategory, string> map;
    map[SFGeneralCategoryNil] = "Nil";
    map[SFGeneralCategoryLU]  = "Lu";
    map[SFGeneralCategoryLL]  = "Ll";
    map[SFGeneralCategoryLT]  = "Lt";
    map[SFGeneralCategoryLM]  = "Lm";
    map[SFGeneralCategoryLO]  = "Lo";
    map[SFGeneralCategoryMN]  = "Mn";
    map[SFGeneralCategoryMC]  = "Mc";
    map[SFGeneralCategoryME]  = "Me";
    map[SFGeneralCategoryND]  = "Nd";
    map[SFGeneralCategoryNL]  = "Nl";
    map[SFGeneralCategoryNO]  = "No";
    map[SFGeneralCategoryPC]  = "Pc";
    map[SFGeneralCategoryPD]  = "Pd";
    map[SFGeneralCategoryPS]  = "Ps";
    map[SFGeneralCategoryPE]  = "Pe";
    map[SFGeneralCategoryPI]  = "Pi";
    map[SFGeneralCategoryPF]  = "Pf";
    map[SFGeneralCategoryPO]  = "Po";
    map[SFGeneralCategorySM]  = "Sm";
    map[SFGeneralCategorySC]  = "Sc";
    map[SFGeneralCategorySK]  = "Sk";
    map[SFGeneralCategorySO]  = "So";
    map[SFGeneralCategoryZS]  = "Zs";
    map[SFGeneralCategoryZL]  = "Zl";
    map[SFGeneralCategoryZP]  = "Zp";
    map[SFGeneralCategoryCC]  = "Cc";
    map[SFGeneralCategoryCF]  = "Cf";
    map[SFGeneralCategoryCS]  = "Cs";
    map[SFGeneralCategoryCO]  = "Co";
    map[SFGeneralCategoryCN]  = "Cn";

    return map;
}

static map<SFGeneralCategory, string> MAP_GENERAL_CATEGORY_TO_STRING = createGeneralCategoryMap();

char Convert::toChar(SFJoiningType joiningType)
{
    switch (joiningType) {
    case SFJoiningTypeU:
        return 'U';

    case SFJoiningTypeL:
        return 'L';

    case SFJoiningTypeR:
        return 'R';

    case SFJoiningTypeD:
        return 'D';

    case SFJoiningTypeC:
        return 'C';

    case SFJoiningTypeT:
        return 'T';
    }

    return '\0';
}

std::string &Convert::toString(SFGeneralCategory generalCategory)
{
    return MAP_GENERAL_CATEGORY_TO_STRING[generalCategory];
}
