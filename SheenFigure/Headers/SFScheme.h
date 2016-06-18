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

#ifndef _SF_SCHEME_H
#define _SF_SCHEME_H

#include "SFBase.h"
#include "SFFont.h"
#include "SFPattern.h"

/**
 * The type used to represent an open type scheme.
 */
typedef struct _SFScheme *SFSchemeRef;

SFSchemeRef SFSchemeCreate(void);

void SFSchemeSetFont(SFSchemeRef scheme, SFFontRef font);
void SFSchemeSetScriptTag(SFSchemeRef scheme, SFTag scriptTag);
void SFSchemeSetLanguageTag(SFSchemeRef scheme, SFTag languageTag);

SFPatternRef SFSchemeBuildPattern(SFSchemeRef scheme);

SFSchemeRef SFSchemeRetain(SFSchemeRef scheme);
void SFSchemeRelease(SFSchemeRef scheme);

#endif
