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

#include "Base.h"
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
        writer.defer(langSys);
    }
};

struct ScriptTable : public Table {
    LangSysTable *defaultLangSys;   // Offset to DefaultLangSys table-from beginning of Script table-may be NULL
    UInt16 langSysCount;            // Number of LangSysRecords for this script-excluding the DefaultLangSys
    LangSysRecord *langSysRecord;   // Array of LangSysRecords-listed alphabetically by LangSysTag

    void write(Writer &writer) override {
        writer.enter();

        writer.defer(defaultLangSys);
        writer.write(langSysCount);
        for (int i = 0; i < langSysCount; i++) {
            writer.write(&langSysRecord[i]);
        }

        writer.exit();
    }
};

struct ScriptRecord : public Table {
    unsigned char scriptTag[4];     // 4-byte ScriptTag identifier
    ScriptTable	*script;            // Offset to Script table-from beginning of ScriptList

    void write(Writer &writer) override {
        writer.write(scriptTag, 4);
        writer.defer(script);
    }
};

struct ScriptListTable : public Table {
    UInt16 scriptCount;             // Number of ScriptRecords
    ScriptRecord *scriptRecord;     // Array of ScriptRecords -listed alphabetically by ScriptTag

    void write(Writer &writer) override {
        writer.enter();

        writer.write(scriptCount);
        for (int i = 0; i < scriptCount; i++) {
            writer.write(&scriptRecord[i]);
        }

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
        writer.defer(feature);
    }
};

struct FeatureListTable : public Table {
    UInt16 featureCount;            // Number of FeatureRecords in this table
    FeatureRecord *featureRecord;   // Array of FeatureRecords-zero-based (first feature has FeatureIndex = 0 -listed alphabetically by FeatureTag

    void write(Writer &writer) override {
        writer.enter();

        writer.write(featureCount);
        for (int i = 0; i < featureCount; i++) {
            writer.write(&featureRecord[i]);
        }

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
        for (int i = 0; i < subTableCount; i++) {
            writer.defer(&subtables[i]);
        }
        writer.write(markFilteringSet);

        writer.exit();
    }
};

struct LookupListTable : public Table {
    UInt16 lookupCount;             // Number of lookups in this table
    LookupTable *lookupTables;      // Array of offsets to Lookup tables-from beginning of LookupList -zero based (first lookup is Lookup index = 0)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(lookupCount);
        for (int i = 0; i < lookupCount; i++) {
            writer.defer(&lookupTables[i]);
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
    UInt16 classFormat;                         // Format identifier

    union {
        struct {
            Glyph startGlyph;                   // First GlyphID of the ClassValueArray
            UInt16 glyphCount;                  // Size of the ClassValueArray
            UInt16 *classValueArray;            // Array of Class Values-one per GlyphID
        } format1;

        struct {
            UInt16 classRangeCount;             // Number of ClassRangeRecords
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
            for (int i = 0; i < format2.classRangeCount; i++) {
                writer.write(&format2.classRangeRecord[i]);
            }
            
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
            for (int i = 0; i < format2.rangeCount; i++) {
                writer.write(&format2.rangeRecord[i]);
            }

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

struct Rule : public Table {
    UInt16 glyphCount;              // Total number of glyphs in input glyph sequence-includes the first glyph
    UInt16 recordCount;             // Number of LookupRecords
    Glyph *input;                   // Array of input GlyphIDs-start with second glyph
    LookupRecord *lookupRecord;     // Array of LookupRecords-in design order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(glyphCount);
        writer.write(recordCount);
        writer.write(input, glyphCount - 1);
        for (int i = 0; i < recordCount; i++) {
            writer.write(&lookupRecord[i]);
        }

        writer.exit();
    }
};

struct RuleSet : public Table {
    UInt16 ruleCount;               // Number of Rule tables.
    Rule *rule;                     // Array of offsets to Rule tables-from beginning of RuleSet table-ordered by preference

    void write(Writer &writer) override {
        writer.enter();

        writer.write(ruleCount);
        for (int i = 0; i < ruleCount; i++) {
            writer.defer(&rule[i]);
        }
        
        writer.exit();
    }
};

struct ClassRule : public Table {
    UInt16 glyphCount;              // Total number of classes specified for the context in the rule-includes the first class
    UInt16 recordCount;             // Number of LookupRecords
    UInt16 *clazz;                  // Array of classes-beginning with the second class-to be matched to the input glyph class sequence
    LookupRecord *lookupRecord;     // Array of lookups-in design order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(glyphCount);
        writer.write(recordCount);
        writer.write(clazz, glyphCount - 1);
        for (int i = 0; i < recordCount; i++) {
            writer.write(&lookupRecord[i]);
        }

        writer.exit();
    }
};

struct ClassSet : public Table {
    UInt16 classRuleCnt;            // Number of ClassRule tables.
    ClassRule *classRule;           // Array of offsets to ClassRule tables-from beginning of ClassSet-ordered by preference

    void write(Writer &writer) override {
        writer.enter();

        writer.write(classRuleCnt);
        for (int i = 0; i < classRuleCnt; i++) {
            writer.defer(&classRule[i]);
        }
        
        writer.exit();
    }
};

struct ContextSubtable : public LookupSubtable {
    UInt16 format;                      // Format identifier

    union {
        struct {
            CoverageTable *coverage;    // Offset to Coverage table-from beginning of Substitution table
            UInt16 ruleSetCount;        // Number of RuleSet tables-must equal GlyphCount in Coverage table
            RuleSet *ruleSet;           // Array of offsets to RuleSet tables-from beginning of subtable-ordered by Coverage Index
        } format1;

        struct {
            CoverageTable *coverage;    // Offset to Coverage table-from beginning of Substitution table
            ClassDefTable *classDef;    // Offset to glyph ClassDef table-from beginning of Substitution table
            UInt16 classSetCnt;         // Number of ClassSet tables
            ClassSet **classSet;        // Array of offsets to ClassSet tables-from beginning of subtable-ordered by class-may be NULL
        } format2;

        struct {
            UInt16 glyphCount;          // Number of glyphs in the input glyph sequence
            UInt16 recordCount;         // Number of LookupRecords
            CoverageTable *coverage;    // Array of offsets to Coverage table-from beginning of subtable-in glyph sequence order
            LookupRecord *lookupRecord; // Array of LookupRecords-in design order
        } format3;
    };

    LookupType lookupType() override {
        return LookupType::sContext;
    }

    void write(Writer &writer) override {
        switch (format) {
            case 1:
                writer.enter();

                writer.write(format);
                writer.defer(format1.coverage);
                writer.write(format1.ruleSetCount);
                for (int i = 0; i < format1.ruleSetCount; i++) {
                    writer.defer(&format1.ruleSet[i]);
                }

                writer.exit();
                break;

            case 2:
                writer.enter();

                writer.write(format);
                writer.defer(format2.coverage);
                writer.defer(format2.classDef);
                writer.write(format2.classSetCnt);
                for (int i = 0; i < format2.classSetCnt; i++) {
                    writer.defer(format2.classSet[i]);
                }

                writer.exit();
                break;

            case 3:
                writer.enter();

                writer.write(format);
                writer.write(format3.glyphCount);
                writer.write(format3.recordCount);
                for (int i = 0; i < format3.glyphCount; i++) {
                    writer.defer(&format3.coverage[i]);
                }
                for (int i = 0; i < format3.recordCount; i++) {
                    writer.write(&format3.lookupRecord[i]);
                }

                writer.exit();
                break;
        }
    }
};

struct ChainRule : public Table {
    UInt16 backtrackGlyphCount;     // Total number of glyphs in the backtrack sequence (number of glyphs to be matched before the first glyph)
    Glyph *backtrack;               // Array of backtracking GlyphID's (to be matched before the input sequence)
    UInt16 inputGlyphCount;         // Total number of glyphs in the input sequence (includes the first glyph)
    Glyph *input;                   // Array of input GlyphIDs (start with second glyph)
    UInt16 lookaheadGlyphCount;     // Total number of glyphs in the look ahead sequence (number of glyphs to be matched after the input sequence)
    Glyph *lookAhead;               // Array of lookahead GlyphID's (to be matched after the input sequence)
    UInt16 recordCount;             // Number of LookupRecords
    LookupRecord *lookupRecord;     // Array of LookupRecords (in design order)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(backtrackGlyphCount);
        writer.write(backtrack, backtrackGlyphCount);
        writer.write(inputGlyphCount);
        writer.write(input, inputGlyphCount - 1);
        writer.write(lookaheadGlyphCount);
        writer.write(lookAhead, lookaheadGlyphCount);
        writer.write(recordCount);
        for (int i = 0; i < recordCount; i++) {
            writer.write(&lookupRecord[i]);
        }

        writer.exit();
    }
};

struct ChainRuleSet : public Table {
    UInt16 chainRuleCount;          // Number of ChainRule tables.
    ChainRule *chainRule;           // Array of offsets to ChainRule tables-from beginning of ChainRuleSet table-ordered by preference

    void write(Writer &writer) override {
        writer.enter();

        writer.write(chainRuleCount);
        for (int i = 0; i < chainRuleCount; i++) {
            writer.defer(&chainRule[i]);
        }

        writer.exit();
    }
};

struct ChainClassRule : public Table {
    UInt16 backtrackGlyphCount;     // Total number of glyphs in the backtrack sequence (number of glyphs to be matched before the first glyph)
    UInt16 *backtrack;              // Array of backtracking classes(to be matched before the input sequence)
    UInt16 inputGlyphCount;         // Total number of classes in the input sequence (includes the first class)
    UInt16 *input;                  // Array of input classes(start with second class; to be matched with the input glyph sequence)
    UInt16 lookaheadGlyphCount;     // Total number of classes in the look ahead sequence (number of classes to be matched after the input sequence)
    UInt16 *lookAhead;              // Array of lookahead classes(to be matched after the input sequence)
    UInt16 recordCount;             // Number of LookupRecords
    LookupRecord *lookupRecord;     // Array of LookupRecords (in design order)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(backtrackGlyphCount);
        writer.write(backtrack, backtrackGlyphCount);
        writer.write(inputGlyphCount);
        writer.write(input, inputGlyphCount - 1);
        writer.write(lookaheadGlyphCount);
        writer.write(lookAhead, lookaheadGlyphCount);
        writer.write(recordCount);
        for (int i = 0; i < recordCount; i++) {
            writer.write(&lookupRecord[i]);
        }

        writer.exit();
    }
};

struct ChainClassSet : public Table {
    UInt16 chainClassRuleCnt;       // Number of ChainSubClassRule tables
    ChainClassRule *chainClassRule; // Array of offsets to ChainClassRule tables-from beginning of ChainClassSet-ordered by preference

    void write(Writer &writer) override {
        writer.enter();

        writer.write(chainClassRuleCnt);
        for (int i = 0; i < chainClassRuleCnt; i++) {
            writer.defer(&chainClassRule[i]);
        }
        
        writer.exit();
    }
};

struct ChainContextSubtable : public LookupSubtable {
    UInt16 format;                                  // Format identifier

    union {
        struct {
            CoverageTable *coverage;                // Offset to Coverage table-from beginning of Substitution table
            UInt16 chainRuleSetCount;               // Number of ChainRuleSet tables-must equal GlyphCount in Coverage table
            ChainRuleSet *chainRuleSet;             // Array of offsets to ChainRuleSet tables-from beginning of Substitution table-ordered by Coverage Index
        } format1;

        struct {
            CoverageTable *coverage;                // Offset to Coverage table-from beginning of Substitution table
            ClassDefTable *backtrackClassDef;       // Offset to glyph ClassDef table containing backtrack sequence data-from beginning of Substitution table
            ClassDefTable *inputClassDef;           // Offset to glyph ClassDef table containing input sequence data-from beginning of Substitution table
            ClassDefTable *lookaheadClassDef;       // Offset to glyph ClassDef table containing lookahead sequence data-from beginning of Substitution table
            UInt16 chainClassSetCnt;                // Number of ChainClassSet tables
            ChainClassSet **chainClassSet;          // Array of offsets to ChainClassSet tables-from beginning of Substitution table-ordered by input class-may be NULL
        } format2;

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
        switch (format) {
        case 1:
            writer.enter();

            writer.write(format);
            writer.defer(format1.coverage);
            writer.write(format1.chainRuleSetCount);
            for (int i = 0; i < format1.chainRuleSetCount; i++) {
                writer.defer(&format1.chainRuleSet[i]);
            }

            writer.exit();
            break;

        case 2:
            writer.enter();

            writer.write(format);
            writer.defer(format2.coverage);
            writer.defer(format2.backtrackClassDef);
            writer.defer(format2.inputClassDef);
            writer.defer(format2.lookaheadClassDef);
            writer.write(format2.chainClassSetCnt);
            for (int i = 0; i < format2.chainClassSetCnt; i++) {
                writer.defer(format2.chainClassSet[i]);
            }

            writer.exit();
            break;

        case 3:
            writer.enter();

            writer.write(format);
            writer.write(format3.backtrackGlyphCount);
            for (int i = 0; i < format3.backtrackGlyphCount; i++) {
                writer.defer(&format3.backtrackGlyphCoverage[i]);
            }
            writer.write(format3.inputGlyphCount);
            for (int i = 0; i < format3.inputGlyphCount; i++) {
                writer.defer(&format3.inputGlyphCoverage[i]);
            }
            writer.write(format3.lookaheadGlyphCount);
            for (int i = 0; i < format3.lookaheadGlyphCount; i++) {
                writer.defer(&format3.lookaheadGlyphCoverage[i]);
            }
            writer.write(format3.recordCount);
            for (int i = 0; i < format3.recordCount; i++) {
                writer.write(&format3.lookupRecord[i]);
            }

            writer.exit();
            break;
        }
    }
};

struct ExtensionSubtable : public LookupSubtable {
    UInt16 format;                  // Format identifier
    LookupType extensionLookupType; // Lookup type of subtable referenced by ExtensionOffset (i.e. the extension subtable)
    LookupSubtable *extensionTable; // Offset to the extension subtable, of lookup type ExtensionLookupType, relative to the start of the extension subtable

    LookupType lookupType() override {
        return LookupType::sExtensionSubstitution;
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(format);
        writer.write((UInt16)extensionLookupType);
        writer.defer(extensionTable, true);

        writer.exit();
    }
};

}
}
}

#endif
