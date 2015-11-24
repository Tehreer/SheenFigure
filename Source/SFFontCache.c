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

#include <SFConfig.h>
#include <SFTypes.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPES_H

#include <stddef.h>
#include <stdlib.h>

#include "SFCommon.h"
#include "SFData.h"
#include "SFGSUB.h"
#include "SFGPOS.h"
#include "SFTableCache.h"

static void _SFLoadTableFromFTFace(FT_Face ftFace, FT_ULong tag, SFData *table) {
    FT_ULong length = 0;
    FT_Error error;

    error = FT_Load_Sfnt_Table(ftFace, tag, 0, NULL, &length);

    if (!error) {
        FT_Byte *buffer = malloc(length);
        error = FT_Load_Sfnt_Table(ftFace, tag, 0, buffer, &length);
        *table = error ? NULL : buffer;
    } else {
        *table = NULL;
    }
}

SF_INTERNAL void SFTableCacheInitialize(SFTableCacheRef tableCache, FT_Face ftFace) {
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'D', 'E', 'F'), &tableCache->gdef);
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'S', 'U', 'B'), &tableCache->gsub);
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'P', 'O', 'S'), &tableCache->gpos);
}

SF_INTERNAL void SFTableCacheFinalize(SFTableCacheRef tableCache) {
    free(tableCache->gdef);
    free(tableCache->gsub);
    free(tableCache->gpos);
}
