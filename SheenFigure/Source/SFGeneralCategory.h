/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
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

#ifndef _SF_GENERAL_CATEGORY_H
#define _SF_GENERAL_CATEGORY_H

#include "SFBase.h"

enum {
    SFGeneralCategoryNil = 0x00,

    SFGeneralCategoryLU  = 0x01, /**< Letter: Uppercase Letter */
    SFGeneralCategoryLL  = 0x02, /**< Letter: Lowercase Letter */
    SFGeneralCategoryLT  = 0x03, /**< Letter: Titlecase Letter */
    SFGeneralCategoryLM  = 0x04, /**< Letter: Modifier Letter */
    SFGeneralCategoryLO  = 0x05, /**< Letter: Other Letter */

    SFGeneralCategoryMN  = 0x06, /**< Mark: Nonspacing Mark */
    SFGeneralCategoryMC  = 0x07, /**< Mark: Spacing Mark */
    SFGeneralCategoryME  = 0x08, /**< Mark: Enclosing Mark */

    SFGeneralCategoryND  = 0x09, /**< Number: Decimal Number */
    SFGeneralCategoryNL  = 0x0A, /**< Number: Letter Number */
    SFGeneralCategoryNO  = 0x0B, /**< Number: Other Number */

    SFGeneralCategoryPC  = 0x0C, /**< Punctuation: Connector Punctuation */
    SFGeneralCategoryPD  = 0x0D, /**< Punctuation: Dash Punctuation */
    SFGeneralCategoryPS  = 0x0E, /**< Punctuation: Open Punctuation */
    SFGeneralCategoryPE  = 0x0F, /**< Punctuation: Close Punctuation */
    SFGeneralCategoryPI  = 0x10, /**< Punctuation: Initial Punctuation */
    SFGeneralCategoryPF  = 0x11, /**< Punctuation: Final Punctuation */
    SFGeneralCategoryPO  = 0x12, /**< Punctuation: Other Punctuation */

    SFGeneralCategorySM  = 0x13, /**< Symbol: Math Symbol */
    SFGeneralCategorySC  = 0x14, /**< Symbol: Currency Symbol */
    SFGeneralCategorySK  = 0x15, /**< Symbol: Modifier Symbol */
    SFGeneralCategorySO  = 0x16, /**< Symbol: Other Symbol */

    SFGeneralCategoryZS  = 0x17, /**< Separator: Space Separator */
    SFGeneralCategoryZL  = 0x18, /**< Separator: Line Separator */
    SFGeneralCategoryZP  = 0x19, /**< Separator: Paragraph Separator */

    SFGeneralCategoryCC  = 0x1A, /**< Other: Control */
    SFGeneralCategoryCF  = 0x1B, /**< Other: Format */
    SFGeneralCategoryCS  = 0x1C, /**< Other: Surrogate */
    SFGeneralCategoryCO  = 0x1D, /**< Other: Private_Use */
    SFGeneralCategoryCN  = 0x1E  /**< Other: Unassigned */
};
typedef SFUInt8 SFGeneralCategory;

#define SFGeneralCategoryIsLetter(gc)                   \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryLU)                           \
        <= (SFGeneralCategoryLO - SFGeneralCategoryLU)  \
)

#define SFGeneralCategoryIsMark(gc)                     \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryMN)                           \
        <= (SFGeneralCategoryME - SFGeneralCategoryMN)  \
)

#define SFGeneralCategoryIsNumber(gc)                   \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryND)                           \
        <= (SFGeneralCategoryNO - SFGeneralCategoryND)  \
)

#define SFGeneralCategoryIsPunctuation(gc)              \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryPC)                           \
        <= (SFGeneralCategoryPO - SFGeneralCategoryPC)  \
)

#define SFGeneralCategoryIsSymbol(gc)                   \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategorySM)                           \
        <= (SFGeneralCategorySO - SFGeneralCategorySM)  \
)

#define SFGeneralCategoryIsSeparator(gc)                \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryZS)                           \
        <= (SFGeneralCategoryZP - SFGeneralCategoryZS)  \
)

#define SFGeneralCategoryIsOther(gc)                    \
(                                                       \
 (SFGeneralCategory)                                    \
 ((gc) - SFGeneralCategoryCC)                           \
        <= (SFGeneralCategoryCN - SFGeneralCategoryCC)  \
)

#endif
