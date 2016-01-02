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

#include "SFFontCache.h"

static void _SFLoadTableFromFTFace(FT_Face ftFace, FT_ULong tag, SFData *table) {
    FT_ULong length = 0;
    FT_Error error;

    *table = NULL;
    error = FT_Load_Sfnt_Table(ftFace, tag, 0, NULL, &length);

    if (!error) {
        FT_Byte *buffer = malloc(length);
        error = FT_Load_Sfnt_Table(ftFace, tag, 0, buffer, &length);
        if (!error) {
            *table = buffer;
        } else {
            free(buffer);
        }
    }
}

SF_INTERNAL void SFFontCacheInitialize(SFFontCacheRef fontCache, FT_Face ftFace) {
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'D', 'E', 'F'), &fontCache->gdef);
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'S', 'U', 'B'), &fontCache->gsub);
    _SFLoadTableFromFTFace(ftFace, FT_MAKE_TAG('G', 'P', 'O', 'S'), &fontCache->gpos);
}

SF_INTERNAL void SFFontCacheFinalize(SFFontCacheRef fontCache) {
    free((void *)fontCache->gdef);
    free((void *)fontCache->gsub);
    free((void *)fontCache->gpos);
}
