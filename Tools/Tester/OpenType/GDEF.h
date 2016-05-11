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

#ifndef __SHEENFIGURE_TESTER__OPEN_TYPE__GDEF_H
#define __SHEENFIGURE_TESTER__OPEN_TYPE__GDEF_H

#include "Common.h"
#include "DataTypes.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

struct AttachPointTable : public Table {
    UInt16 pointCount;              // Number of attachment points on this glyph
    UInt16 *pointIndex;             // Array of contour point indices -in increasing numerical order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(pointCount);
        writer.write(pointIndex, pointCount);

        writer.exit();
    }
};

struct AttachListTable : public Table {
    CoverageTable *coverage;        // Offset to Coverage table - from beginning of AttachList table
    UInt16 glyphCount;              // Number of glyphs with attachment points
    AttachPointTable *attachPoints; // Array of offsets to AttachPoint tables-from beginning of AttachList table-in Coverage Index order

    void write(Writer &writer) override {
        writer.enter();

        int coverageOffset = writer.reserveOffset();
        writer.write(glyphCount);
        int attachPointOffsets[glyphCount];
        for (int i = 0; i < glyphCount; i++) {
            attachPointOffsets[i] = writer.reserveOffset();
        }

        writer.writeTable(coverage, coverageOffset);
        for (int i = 0; i < glyphCount; i++) {
            writer.writeTable(&attachPoints[i], attachPointOffsets[i]);
        }
        writer.exit();
    }
};

struct CaretValuesTable : public Table {
    UInt16 caretValueFormat;        // Format identifier

    union {
        struct {
            Int16 coordinate;       // X or Y value, in design units
        } format1;

        struct {
            UInt16 caretValuePoint; // Contour point index on glyph;
        } format2;

        struct {
            Int16 coordinate;       // X or Y value, in design units
            DeviceTable *deviceTable; // Offset to Device table for X or Y value-from beginning of CaretValue table
        } format3;
    };

    void write(Writer &writer) override {
        switch (caretValueFormat) {
            case 1:
                writer.enter();

                writer.write(caretValueFormat);
                writer.write((UInt16)format1.coordinate);

                writer.exit();
                break;
                
            case 2:
                writer.enter();

                writer.write(caretValueFormat);
                writer.write(format2.caretValuePoint);

                writer.exit();
                break;

            case 3: {
                writer.enter();

                writer.write(caretValueFormat);
                writer.write((UInt16)format3.coordinate);
                int deviceOffset = writer.reserveOffset();

                writer.write(format3.deviceTable, deviceOffset);
                writer.exit();
                break;
            }
        }
    }
};

struct LigatureGlyphTable : public Table {
    UInt16 caretCount;              // Number of CaretValues for this ligature (components - 1)
    CaretValuesTable *caretValue;   // Array of offsets to CaretValue tables-from beginning of LigGlyph table-in increasing coordinate order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(caretCount);
        int caretValueOffsets[caretCount];
        for (int i = 0; i < caretCount; i++) {
            caretValueOffsets[i] = writer.reserveOffset();
        }

        for (int i = 0; i < caretCount; i++) {
            writer.writeTable(&caretValue[i], caretValueOffsets[i]);
        }
        writer.exit();
    }
};

struct LigatureCaretListTable : public Table {
    CoverageTable *coverage;        // Offset to Coverage table - from beginning of LigCaretList table
    UInt16 ligGlyphCount;           // Number of ligature glyphs
    LigatureGlyphTable *ligGlyph;   // Array of offsets to LigGlyph tables-from beginning of LigCaretList table-in Coverage Index order

    void write(Writer &writer) override {
        writer.enter();

        int coverageOffset = writer.reserveOffset();
        writer.write(ligGlyphCount);
        int ligGlyphOffsets[ligGlyphCount];
        for (int i = 0; i < ligGlyphCount; i++) {
            ligGlyphOffsets[i] = writer.reserveOffset();
        }

        writer.writeTable(coverage, coverageOffset);
        for (int i = 0; i < ligGlyphCount; i++) {
            writer.writeTable(&ligGlyph[i], ligGlyphOffsets[i]);
        }
        writer.exit();
    }
};

struct MarkGlyphSetsDefTable : public Table {
    UInt16 markSetTableFormat;      // Format identifier == 1
    UInt16 markSetCount;            // Number of mark sets defined
    CoverageTable *coverage;        // Array of offsets to mark set coverage tables.

    void write(Writer &writer) override {
        writer.enter();

        writer.write(markSetTableFormat);
        writer.write(markSetCount);
        int coverageOffsets[markSetCount];
        for (int i = 0; i < markSetCount; i++) {
            coverageOffsets[i] = writer.reserveOffset();
        }

        for (int i = 0; i < markSetCount; i++) {
            writer.writeTable(&coverage[i], coverageOffsets[i]);
        }
        writer.exit();
    }
};

struct GDEF : public Table {
    UInt32 version;                 // Version of the GDEF table-0x00010000 or 0x00010002
    ClassDefTable *glyphClassDef;   // Offset to class definition table for glyph type-from beginning of GDEF header (may be NULL)
    AttachListTable *attachList;    // Offset to list of glyphs with attachment points-from beginning of GDEF header (may be NULL)
    LigatureCaretListTable *ligCaretList; // Offset to list of positioning points for ligature carets-from beginning of GDEF header (may be NULL)
    ClassDefTable *markAttachClassDef; // Offset to class definition table for mark attachment type-from beginning of GDEF header (may be NULL)
    MarkGlyphSetsDefTable *markGlyphSetsDef; // Offset to the table of mark set definitions - from beginning of GDEF header (may be NULL)

    void write(Writer &writer) override {
        writer.enter();

        writer.write(version);
        int glyphClassDefOffset = writer.reserveOffset();
        int attachListOffset = writer.reserveOffset();
        int ligCaretListOffset = writer.reserveOffset();
        int markAttachClassDefOffset = writer.reserveOffset();
        int markGlyphSetsDefOffset = writer.reserveOffset();

        writer.writeTable(glyphClassDef, glyphClassDefOffset);
        writer.writeTable(attachList, attachListOffset);
        writer.writeTable(ligCaretList, ligCaretListOffset);
        writer.writeTable(markAttachClassDef, markAttachClassDefOffset);
        writer.writeTable(markGlyphSetsDef, markGlyphSetsDefOffset);
        writer.exit();
    }
};

}
}
}

#endif
