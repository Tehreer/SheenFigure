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

#ifndef __SHEENFIGURE_TESTER__OPEN_TYPE__GSUB_H
#define __SHEENFIGURE_TESTER__OPEN_TYPE__GSUB_H

#include "Common.h"
#include "DataTypes.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

struct SingleSubstSubtable : public LookupSubtable {
    UInt16 substFormat;             // Format identifier-format = 1
    CoverageTable *coverage;        // Offset to Coverage table-from beginning of Substitution table

    union {
        struct {
            Int16 deltaGlyphID;     // Add to original GlyphID to get substitute GlyphID
        } format1;

        struct {
            UInt16 glyphCount;      // Number of GlyphIDs in the Substitute array
            Glyph *substitute;      // Array of substitute GlyphIDs-ordered by Coverage Index
        } format2;
    };

    LookupType lookupType() override {
        return LookupType::sSingle;
    }

    void write(Writer &writer) override {
        switch (substFormat) {
            case 1: {
                writer.enter();

                writer.write(substFormat);
                int coverageOffset = writer.reserveOffset();
                writer.write((UInt16)format1.deltaGlyphID);

                writer.writeTable(coverage, coverageOffset);

                writer.exit();
                break;
            }
                
            case 2: {
                writer.enter();

                writer.write(substFormat);
                int coverageOffset = writer.reserveOffset();
                writer.write(format2.glyphCount);
                writer.write(format2.substitute, format2.glyphCount);

                writer.writeTable(coverage, coverageOffset);

                writer.exit();
                break;
            }
        }
    }
};

struct SequenceTable : public Table {
    UInt16 glyphCount;              // Number of GlyphIDs in the Substitute array. This should always be greater than 0.
    Glyph *substitute;              // String of GlyphIDs to substitute

    void write(Writer &writer) override {
        writer.enter();

        writer.write(glyphCount);
        writer.write(substitute, glyphCount);

        writer.exit();
    }
};

struct MultipleSubstSubtable : public LookupSubtable {
    UInt16 substFormat;             // Format identifier-format = 2
    CoverageTable *coverage;        // Offset to Coverage table-from beginning of Substitution table
    UInt16 sequenceCount;           // Number of Sequence table offsets in the Sequence array
    SequenceTable *sequence;        // Array of offsets to Sequence tables-from beginning of Substitution table-ordered by Coverage Index

    LookupType lookupType() override {
        return LookupType::sMultiple;
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(substFormat);
        int coverageOffset = writer.reserveOffset();
        writer.write(sequenceCount);
        int sequenceRefs[sequenceCount];
        for (int i = 0; i < sequenceCount; i++) {
            sequenceRefs[i] = writer.reserveOffset();
        }

        writer.writeTable(coverage, coverageOffset);
        for (int i = 0; i < sequenceCount; i++) {
            writer.writeTable(&sequence[i], sequenceRefs[i]);
        }

        writer.exit();
    }
};

struct AlternateSetTable : public Table {
    UInt16 glyphCount;              // Number of GlyphIDs in the Alternate array
    Glyph *substitute;              // Array of alternate GlyphIDs-in arbitrary order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(glyphCount);
        writer.write(substitute, glyphCount);

        writer.exit();
    }
};

struct AlternateSubstSubtable : public LookupSubtable {
    UInt16 substFormat;             // Format identifier-format = 1
    CoverageTable *coverage;        // Offset to Coverage table-from beginning of Substitution table
    UInt16 alternateSetCount;       // Number of AlternateSet tables
    AlternateSetTable *alternateSet;// Array of offsets to AlternateSet tables-from beginning of Substitution table-ordered by Coverage Index

    LookupType lookupType() override {
        return LookupType::sAlternate;
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(substFormat);
        int coverageOffset = writer.reserveOffset();
        writer.write(alternateSetCount);
        int sequenceRefs[alternateSetCount];
        for (int i = 0; i < alternateSetCount; i++) {
            sequenceRefs[i] = writer.reserveOffset();
        }

        writer.writeTable(coverage, coverageOffset);
        for (int i = 0; i < alternateSetCount; i++) {
            writer.writeTable(&alternateSet[i], sequenceRefs[i]);
        }

        writer.exit();
    }
};

struct LigatureTable : public Table {
    Glyph ligGlyph;             // GlyphID of ligature to substitute
    UInt16 compCount;           // Number of components in the ligature
    Glyph *component;           // [CompCount - 1]. Array of component GlyphIDs-start with the second component-ordered in writing direction

    void write(Writer &writer) override {
        writer.enter();

        writer.write(ligGlyph);
        writer.write(compCount);
        writer.write(component, compCount - 1);

        writer.exit();
    }
};

struct LigatureSetTable : public Table {
    UInt16 ligatureCount;           // Number of Ligature tables
    LigatureTable *ligature;        // Array of offsets to Ligature tables-from beginning of LigatureSet table-ordered by preference

    void write(Writer &writer) override {
        writer.enter();

        writer.write(ligatureCount);
        int ligatureOffset = writer.reserveOffset();

        writer.writeTable(ligature, ligatureOffset);

        writer.exit();
    }
};

struct LigatureSubstSubtable : public LookupSubtable {
    UInt16 substFormat;             // Format identifier-format = 1
    CoverageTable *coverage;        // Offset to Coverage table-from beginning of Substitution table
    UInt16 ligSetCount;             // Number of LigatureSet tables
    LigatureSetTable *ligatureSet;  // Array of offsets to LigatureSet tables-from beginning of Substitution table-ordered by Coverage Index

    LookupType lookupType() override {
        return LookupType::sLigature;
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(substFormat);
        int coverageOffset = writer.reserveOffset();
        writer.write(ligSetCount);
        int ligatureSetOffsets[ligSetCount];
        for (int i = 0; i < ligSetCount; i++) {
            ligatureSetOffsets[i] = writer.reserveOffset();
        }

        writer.writeTable(coverage, coverageOffset);
        for (int i = 0; i < ligSetCount; i++) {
            writer.writeTable(&ligatureSet[i], ligatureSetOffsets[i]);
        }

        writer.exit();
    }
};

struct GSUB : public Table {
    UInt32 version;                 // Version of the GSUB table-initially set to 0x00010000
    ScriptListTable *scriptList;    // Offset to ScriptList table-from beginning of GSUB table
    FeatureListTable *featureList;  // Offset to FeatureList table-from beginning of GSUB table
    LookupListTable *lookupList;    // Offset to LookupList table-from beginning of GSUB table

    void write(Writer &writer) override {
        writer.enter();

        writer.write(version);
        int scriptListRef = writer.reserveOffset();
        int featureListRef = writer.reserveOffset();
        int lookupListRef = writer.reserveOffset();

        writer.writeTable(scriptList, scriptListRef);
        writer.writeTable(featureList, featureListRef);
        writer.writeTable(lookupList, lookupListRef);

        writer.exit();
    }
};

}
}
}

#endif
