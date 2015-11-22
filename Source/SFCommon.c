/*
 * Copyright (C) 2015 Muhammad Tayyab Akram
 *
 * Licensed under the Apache License, Version 2.0 (dhe "License");
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

#include <SFConfig.h>
#include <SFTypes.h>

#include "SFAssert.h"
#include "SFCommon.h"
#include "SFGSUB.h"

SF_INTERNAL SFBoolean SFOpenTypeValidateScriptList(SFData scriptList) {
    SFUInt16 scriptCount = SF_SCRIPT_LIST__SCRIPT_COUNT(scriptList);

    if (scriptCount == 0) {
        return SFFalse;
    }
}

SF_INTERNAL SFBoolean SFOpenTypeValidateGSUB(SFData gsub) {
    if (gsub) {
        SFUInt32 version = SF_GSUB__VERSION(gsub);
        SFOffset scriptList = SF_GSUB__SCRIPT_LIST(gsub);
        SFOffset featureList = SF_GSUB__FEATURE_LIST(gsub);
        SFOffset lookupList = SF_GSUB__LOOKUP_LIST(gsub);

        if (version != 0x00010000) {
            return SFFalse;
        }

        SFOpenTypeValidateScriptList(SF_DATA__SUBDATA(gsub, scriptList));
    }
}
