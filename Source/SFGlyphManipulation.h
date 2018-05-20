/*
 * Copyright (C) 2015-2018 Muhammad Tayyab Akram
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

#ifndef _SF_INTERNAL_GLYPH_MANIPULATION_H
#define _SF_INTERNAL_GLYPH_MANIPULATION_H

#include <SFConfig.h>

#include "SFData.h"
#include "SFPattern.h"
#include "SFTextProcessor.h"

SF_PRIVATE SFBoolean _SFApplyContextSubtable(SFTextProcessorRef textProcessor, SFData contextSubtable);
SF_PRIVATE SFBoolean _SFApplyChainContextSubtable(SFTextProcessorRef textProcessor, SFData chainContextSubtable);
SF_PRIVATE SFBoolean _SFApplyExtensionSubtable(SFTextProcessorRef textProcessor, SFData extensionSubtable);
SF_PRIVATE SFBoolean _SFApplyReverseChainSubst(SFTextProcessorRef textProcessor, SFData reverseChain);

#endif
