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

#ifndef __SHEENFIGURE_TESTER__OPENTYPE__COMMON_H
#define __SHEENFIGURE_TESTER__OPENTYPE__COMMON_H

#include <cstddef>
#include "DataTypes.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

/************************************SCRIPT TABLES***************************************/

struct LangSysTable : public Table {
    Offset lookupOrder = 0;         // (reserved for an offset to a reordering table)
    UInt16 reqFeatureIndex;         // Index of a feature required for this language system- if no required features = 0xFFFF
    UInt16 featureCount;            // Number of FeatureIndex values for this language system-excludes the required feature
    UInt16 *featureIndex;           // Array of indices into the FeatureList-in arbitrary order

    void write(Writer &writer) override {
        writer.write(lookupOrder);
        writer.write(reqFeatureIndex);
        writer.write(featureCount);
        writer.write(featureIndex, featureCount);
    }
};

struct LangSysRecord : public Table {
    unsigned char langSysTag[4];    // 4-byte LangSysTag identifier
    LangSysTable *langSys;          // Offset to LangSys table-from beginning of Script table

    void write(Writer &writer) override {
        writer.write(langSysTag, 4);
        int langSysOffset = writer.reserveOffset();

        writer.writeTable(langSys, langSysOffset);
    }
};

struct ScriptTable : public Table {
    LangSysTable *defaultLangSys;   // Offset to DefaultLangSys table-from beginning of Script table-may be NULL
    UInt16 langSysCount;            // Number of LangSysRecords for this script-excluding the DefaultLangSys
    LangSysRecord *langSysRecord;   // Array of LangSysRecords-listed alphabetically by LangSysTag

    void write(Writer &writer) override {
        writer.enter();

        int defaultLangSysOffset = writer.reserveOffset();
        writer.write(langSysCount);
        writer.write(langSysRecord, langSysCount);

        writer.writeTable(defaultLangSys, defaultLangSysOffset);
        writer.exit();
    }
};

struct ScriptRecord : public Table {
    unsigned char scriptTag[4];     // 4-byte ScriptTag identifier
    ScriptTable	*script;            // Offset to Script table-from beginning of ScriptList

    void write(Writer &writer) override {
        writer.write(scriptTag, 4);
        int scriptOffset = writer.reserveOffset();

        writer.writeTable(script, scriptOffset);
    }
};

struct ScriptListTable : public Table {
    UInt16 scriptCount;             // Number of ScriptRecords
    ScriptRecord *scriptRecord;     // Array of ScriptRecords -listed alphabetically by ScriptTag

    void write(Writer &writer) override {
        writer.enter();

        writer.write(scriptCount);
        writer.write(scriptRecord, scriptCount);

        writer.exit();
    }
};

/********************************END SCRIPT TABLES**************************************/

/**********************************FEATURE TABLES***************************************/

struct FeatureTable : public Table {
    Offset featureParams = 0;       // (reserved for offset to FeatureParams)
    UInt16 lookupCount;             // Number of LookupList indices for this feature
    UInt16 *lookupListIndex;        // Array of LookupList indices for this feature -zero-based (first lookup is LookupListIndex = 0)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(featureParams);
        writer.write(lookupCount);
        writer.write(lookupListIndex, lookupCount);

        writer.exit();
    }
};

struct FeatureRecord : public Table {
    unsigned char featureTag[4];    // 4-byte feature identification tag
    FeatureTable *feature;          // Offset to Feature table-from beginning of FeatureList

    void write(Writer &writer) override {
        writer.write(featureTag, 4);
        int featureOffset = writer.reserveOffset();

        writer.writeTable(feature, featureOffset);
    }
};

struct FeatureListTable : public Table {
    UInt16 featureCount;            // Number of FeatureRecords in this table
    FeatureRecord *featureRecord;   // Array of FeatureRecords-zero-based (first feature has FeatureIndex = 0 -listed alphabetically by FeatureTag

    void write(Writer &writer) override {
        writer.enter();

        writer.write(featureCount);
        writer.write(featureRecord, featureCount);

        writer.exit();
    }
};

/*********************************END FEATURE TABLES*************************************/

/************************************LOOKUP TABLES***************************************/

enum class LookupType : UInt16 {
    //////////////////////////////////GSUB LOOKUP TYPES///////////////////////////////////
    sSingle = 1,                    // Replace one glyph with one glyph
    sMultiple = 2,                  // Replace one glyph with more than one glyph
    sAlternate = 3,                 // Replace one glyph with one of many glyphs
    sLigature = 4,                  // Replace multiple glyphs with one glyph
    sContext = 5,                   // One or more glyphs in context
    sChainingContext = 6,           // Replace one or more glyphs in chained context
    sExtensionSubstitution = 7,     // Extension mechanism for other substitutions (i.e. this excludes the Extension type substitution itself)
    sReverseChainingContextSingle = 8, // Applied in reverse order, replace single glyph in chaining context
    sReserved = 9,                  // For future use (set to zero)

    //////////////////////////////////GPOS LOOKUP TYPES///////////////////////////////////
    pSingleAdjustment = 1,          // Adjust position of a single glyph
    pPairAdjustment = 2,            // Adjust position of a pair of glyphs
    pCursiveAttachment = 3,         // Attach cursive glyphs
    pMarkToBaseAttachment = 4,      // Attach a combining mark to a base glyph
    pMarkToLigatureAttachment = 5,  // Attach a combining mark to a ligature
    pMarkToMarkAttachment = 6,      // Attach a combining mark to another mark
    pContextPositioning = 7,        // Position one or more glyphs in context
    pChainedContextPositioning = 8, // Position one or more glyphs in chained context
    pExtensionPositioning = 9,      // Extension mechanism for other positionings
    pReserved = 10,                 // For future use (set to zero)
};

enum class LookupFlag : UInt16 {
    // This bit relates only to the correct processing of the cursive attachment lookup type (GPOS lookup type 3). When this bit is set, the last glyph in a given sequence to which the cursive attachment lookup is applied, will be positioned on the baseline.
    // Note: Setting of this bit is not intended to be used by operating systems or applications to determine text direction.
    RightToLeft = 0x0001,
    IgnoreBaseGlyphs = 0x0002,      // If set, skips over base glyphs
    IgnoreLigatures = 0x0004,       // If set, skips over ligatures
    IgnoreMarks = 0x0008,           // If set, skips over all combining marks
    // If set, indicates that the lookup table structure is followed by a MarkFilteringSet field. The layout engine skips over all mark glyphs not in the mark filtering set indicated.
    UseMarkFilteringSet = 0x0010,
    Reserved = 0x00E0,              // Reserved for future use (Set to zero)
    MarkAttachmentType = 0xFF00,    // If not zero, skips over all marks of attachment type different from specified.
};

struct LookupSubtable : public Table {
    virtual LookupType lookupType() = 0;
};

struct LookupTable : public Table {
    LookupType lookupType;          // Different enumerations for GSUB and GPOS
    LookupFlag lookupFlag;          // Lookup qualifiers
    UInt16 subTableCount;           // Number of SubTables for this lookup
    LookupSubtable *subtables;      // Array of offsets to SubTables-from beginning of Lookup table
    UInt16 markFilteringSet;        // Index (base 0) into GDEF mark glyph sets structure. This field is only present if bit UseMarkFilteringSet of lookup flags is set.

    void write(Writer &writer) override {
        writer.enter();

        writer.write((UInt16)lookupType);
        writer.write((UInt16)lookupFlag);
        writer.write(subTableCount);
        int subtableOffsets[subTableCount];
        for (int i = 0; i < subTableCount; i++) {
            subtableOffsets[i] = writer.reserveOffset();
        }
        writer.write(markFilteringSet);

        for (int i = 0; i < subTableCount; i++) {
            writer.writeTable(&subtables[i], subtableOffsets[i]);
        }

        writer.exit();
    }
};

struct LookupListTable : public Table {
    UInt16 lookupCount;             // Number of lookups in this table
    LookupTable *lookupTables;      // Array of offsets to Lookup tables-from beginning of LookupList -zero based (first lookup is Lookup index = 0)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(lookupCount);
        int lookupTableOffsets[lookupCount];
        for (int i = 0; i < lookupCount; i++) {
            lookupTableOffsets[i] = writer.reserveOffset();
        }

        for (int i = 0; i < lookupCount; i++) {
            writer.writeTable(&lookupTables[i], lookupTableOffsets[i]);
        }

        writer.exit();
    }
};

/**********************************END LOOKUP TABLES*************************************/

struct ClassRangeRecord : public Table {
    Glyph start;                    // First GlyphID in the range
    Glyph end;                      // Last GlyphID in the range
    UInt16 clazz;                   // Applied to all glyphs in the range

    void write(Writer &writer) override {
        writer.write(start);
        writer.write(end);
        writer.write(clazz);
    }
};

struct ClassDefTable : public Table {
    UInt16 classFormat;             // Format identifier

    union {
        struct {
            UInt16 startGlyph;      // First GlyphID of the ClassValueArray
            UInt16 glyphCount;      // Size of the ClassValueArray
            UInt16 *classValueArray;// Array of Class Values-one per GlyphID
        } format1;

        struct {
            UInt16 classRangeCount; // Number of ClassRangeRecords
            ClassRangeRecord *classRangeRecord; //Array of ClassRangeRecords-ordered by Start GlyphID
        } format2;
    };

    void write(Writer &writer) override {
        switch (classFormat) {
            case 1:
                writer.enter();

                writer.write(classFormat);
                writer.write(format1.startGlyph);
                writer.write(format1.glyphCount);
                writer.write(format1.classValueArray, format1.glyphCount);

                writer.exit();
                break;

            case 2:
                writer.enter();

                writer.write(classFormat);
                writer.write(format2.classRangeCount);
                writer.write(format2.classRangeRecord, format2.classRangeCount);
                
                writer.exit();
                break;
        }
    }
};

struct RangeRecord : public Table {
    Glyph start;                    // First GlyphID in the range
    Glyph end;                      // Last GlyphID in the range
    UInt16 startCoverageIndex;      // Coverage Index of first GlyphID in range

    void write(Writer &writer) override {
        writer.write(start);
        writer.write(end);
        writer.write(startCoverageIndex);
    }
};

struct CoverageTable : public Table {
    UInt16 coverageFormat;          // Format identifier-format = 1

    union {
        struct {
            UInt16 glyphCount;      // Number of glyphs in the GlyphArray
            Glyph *glyphArray;      // Array of GlyphIDs-in numerical order
        } format1;

        struct {
            UInt16 rangeCount;      // Number of RangeRecords
            RangeRecord *rangeRecord; // Array of glyph ranges-ordered by Start GlyphID
        } format2;
    };

    void write(Writer &writer) override {
        switch (coverageFormat) {
            case 1:
                writer.enter();

                writer.write(coverageFormat);
                writer.write(format1.glyphCount);
                writer.write(format1.glyphArray, format1.glyphCount);

                writer.exit();
                break;

            case 2:
                writer.enter();

                writer.write(coverageFormat);
                writer.write(format2.rangeCount);
                writer.write(format2.rangeRecord, format2.rangeCount);

                writer.exit();
                break;
        }
    }
};

struct DeviceTable : public Table {
    UInt16 startSize;               // Smallest size to correct-in ppem
    UInt16 endSize;                 // Largest size to correct-in ppem
    UInt16 deltaFormat;             // Format of DeltaValue array data: 1, 2, or 3

    //Format
    //1     2   Signed 2-bit value, 8 values per uint16
    //2     4   Signed 4-bit value, 4 values per uint16
    //3     8   Signed 8-bit value, 2 values per uint16

    UInt16 deltaValue;              // Array of compressed data

    void write(Writer &writer) override {
        writer.enter();

        writer.write(startSize);
        writer.write(endSize);
        writer.write(deltaFormat);
        writer.write(deltaValue);

        writer.exit();
    }
};

struct LookupRecord : public Table {
    UInt16 sequenceIndex;           // Index into current glyph sequence-first glyph = 0
    UInt16 lookupListIndex;         // Lookup to apply to that position-zero-based

    void write(Writer &writer) override {
        writer.write(sequenceIndex);
        writer.write(lookupListIndex);
    }
};

struct ChainContextSubtable : public LookupSubtable {
    UInt16 format;                                  // Format identifier

    union {
        struct {
            UInt16 backtrackGlyphCount;             // Number of glyphs in the backtracking sequence
            CoverageTable *backtrackGlyphCoverage;  // Array of offsets to coverage tables in backtracking sequence, in glyph sequence order
            UInt16 inputGlyphCount;                 // Number of glyphs in input sequence
            CoverageTable *inputGlyphCoverage;      // Array of offsets to coverage tables in input sequence, in glyph sequence order
            UInt16 lookaheadGlyphCount;             // Number of glyphs in lookahead sequence
            CoverageTable *lookaheadGlyphCoverage;  // Array of offsets to coverage tables in lookahead sequence, in glyph sequence order
            UInt16 recordCount;                     // Number of LookupRecords
            LookupRecord *lookupRecord;             // Array of LookupRecords, in design order
        } format3;
    };

    LookupType lookupType() override {
        return LookupType::sChainingContext;
    }

    void write(Writer &writer) override {
        writer.enter();

        switch (format) {
            case 3: {
                writer.write(format);
                writer.write(format3.backtrackGlyphCount);
                int batrackCoverageOffsets[format3.backtrackGlyphCount];
                for (int i = 0; i < format3.backtrackGlyphCount; i++) {
                    batrackCoverageOffsets[i] = writer.reserveOffset();
                }
                writer.write(format3.inputGlyphCount);
                int inputCoverageOffsets[format3.inputGlyphCount];
                for (int i = 0; i < format3.inputGlyphCount; i++) {
                    inputCoverageOffsets[i] = writer.reserveOffset();
                }
                writer.write(format3.lookaheadGlyphCount);
                int lookaheadCoverageOffsets[format3.lookaheadGlyphCount];
                for (int i = 0; i < format3.lookaheadGlyphCount; i++) {
                    lookaheadCoverageOffsets[i] = writer.reserveOffset();
                }
                writer.write(format3.recordCount);
                writer.write(format3.lookupRecord, format3.recordCount);

                for (int i = 0; i < format3.backtrackGlyphCount; i++) {
                    writer.writeTable(&format3.backtrackGlyphCoverage[i], batrackCoverageOffsets[i]);
                }
                for (int i = 0; i < format3.inputGlyphCount; i++) {
                    writer.writeTable(&format3.inputGlyphCoverage[i], inputCoverageOffsets[i]);
                }
                for (int i = 0; i < format3.lookaheadGlyphCount; i++) {
                    writer.writeTable(&format3.lookaheadGlyphCoverage[i], lookaheadCoverageOffsets[i]);
                }
                break;
            }
        }

        writer.exit();
    }
};

struct ExtensionSubtable : public Table {
    UInt16 format;                  // Format identifier
    LookupType extensionLookupType; // Lookup type of subtable referenced by ExtensionOffset (i.e. the extension subtable)
    Table *extensionTable;          // Offset to the extension subtable, of lookup type ExtensionLookupType, relative to the start of the extension subtable

    void write(Writer &writer) override {
        writer.enter();

        writer.write(format);
        writer.write((UInt16)extensionLookupType);
        int extensionOffset = writer.reserveLong();

        writer.writeTable(extensionTable, extensionOffset, true);

        writer.exit();
    }
};

}
}
}

#endif
