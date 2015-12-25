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

#ifndef SF_SCHEME_H
#define SF_SCHEME_H

#include "SFConfig.h"
#include "SFFont.h"
#include "SFLanguageTag.h"
#include "SFPattern.h"
#include "SFScriptTag.h"
#include "SFTypes.h"

struct _SFScheme;
typedef struct _SFScheme SFScheme;
/**
 * The type used to represent a scheme.
 */
typedef SFScheme *SFSchemeRef;

SFSchemeRef SFSchemeCreate(void);

void SFSchemeSetFont(SFSchemeRef scheme, SFFontRef font);
void SFSchemeSetScript(SFSchemeRef scheme, SFScriptTag scriptTag);
void SFSchemeSetLanguage(SFSchemeRef scheme, SFLanguageTag languageTag);

SFPatternRef SFSchemeBuildPattern(SFSchemeRef scheme);

SFSchemeRef SFSchemeRetain(SFSchemeRef scheme);
void SFSchemeRelease(SFSchemeRef scheme);

#endif
