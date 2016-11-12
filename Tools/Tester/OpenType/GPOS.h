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

#ifndef __SHEENFIGURE_TESTER__OPEN_TYPE__GPOS_H
#define __SHEENFIGURE_TESTER__OPEN_TYPE__GPOS_H

#include "Base.h"
#include "Common.h"
#include "Writer.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

enum class ValueFormat : UInt16 {
    None = 0x0000,
    XPlacement = 0x0001,                // Includes horizontal adjustment for placement
    YPlacement = 0x0002,                // Includes vertical adjustment for placement
    XAdvance = 0x0004,                  // Includes horizontal adjustment for advance
    YAdvance = 0x0008,                  // Includes vertical adjustment for advance
    XPlaDevice = 0x0010,                // Includes horizontal Device table for placement
    YPlaDevice = 0x0020,                // Includes vertical Device table for placement
    XAdvDevice = 0x0040,                // Includes horizontal Device table for advance
    YAdvDevice = 0x0080,                // Includes vertical Device table for advance
    Reserved = 0xF000,                  // For future use (set to zero)
};

inline ValueFormat operator|(ValueFormat a, ValueFormat b) { return (ValueFormat)((UInt16)a | (UInt16)b); }
inline UInt16 operator&(ValueFormat a, ValueFormat b) { return ((UInt16)a & (UInt16)b); }

struct ValueRecord : public Table {
    Int16 xPlacement;                   // Horizontal adjustment for placement-in design units
    Int16 yPlacement;                   // Vertical adjustment for placement-in design units
    Int16 xAdvance;                     // Horizontal adjustment for advance-in design units (only used for horizontal writing)
    Int16 yAdvance;                     // Vertical adjustment for advance-in design units (only used for vertical writing)
    DeviceTable *xPlaDevice;            // Offset to Device table for horizontal placement-measured from beginning of PosTable (may be NULL)
    DeviceTable	*yPlaDevice;            // Offset to Device table for vertical placement-measured from beginning of PosTable (may be NULL)
    DeviceTable	*xAdvDevice;            // Offset to Device table for horizontal advance-measured from beginning of PosTable (may be NULL)
    DeviceTable	*yAdvDevice;            // Offset to Device table for vertical advance-measured from beginning of PosTable (may be NULL)

    void preset(ValueFormat valueFormat) {
        m_valueFormat = valueFormat;
    }

    void write(Writer &writer) override {
        if (m_valueFormat & ValueFormat::XPlacement) {
            writer.write((UInt16)xPlacement);
        }
        if (m_valueFormat & ValueFormat::YPlacement) {
            writer.write((UInt16)yPlacement);
        }
        if (m_valueFormat & ValueFormat::XAdvance) {
            writer.write((UInt16)xAdvance);
        }
        if (m_valueFormat & ValueFormat::YAdvance) {
            writer.write((UInt16)yAdvance);
        }
        if (m_valueFormat & ValueFormat::XPlaDevice) {
            writer.defer(xPlaDevice);
        }
        if (m_valueFormat & ValueFormat::YPlaDevice) {
            writer.defer(yPlaDevice);
        }
        if (m_valueFormat & ValueFormat::XAdvDevice) {
            writer.defer(xAdvDevice);
        }
        if (m_valueFormat & ValueFormat::YAdvDevice) {
            writer.defer(yAdvDevice);
        }
    }

private:
    ValueFormat m_valueFormat;
};

struct SinglePosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier
    CoverageTable *coverage;            // Offset to Coverage table-from beginning of SinglePos subtable
    ValueFormat valueFormat;            // Defines the types of data in the ValueRecord

    union {
        struct {
            ValueRecord *value;         // Defines positioning value(s)-applied to all glyphs in the Coverage table
        } format1;

        struct {
            UInt16 valueCount;          // Number of ValueRecords
            ValueRecord *value;         // Array of ValueRecords-positioning values applied to glyphs
        } format2;
    };

    LookupType lookupType() override {
        return LookupType::pSingleAdjustment;
    }

    void write(Writer &writer) override {
        switch (posFormat) {
        case 1:
            format1.value->preset(valueFormat);

            writer.enter();

            writer.write(posFormat);
            writer.defer(coverage);
            writer.write((UInt16)valueFormat);
            writer.write(format1.value);

            writer.exit();
            break;
            
        case 2:
            for (int i = 0; i < format2.valueCount; i++) {
                format2.value[i].preset(valueFormat);
            }

            writer.enter();

            writer.write(posFormat);
            writer.defer(coverage);
            writer.write((UInt16)valueFormat);
            writer.write(format2.valueCount);
            for (int i = 0; i < format2.valueCount; i++) {
                writer.write(&format2.value[i]);
            }

            writer.exit();
            break;
        }
    }
};

struct PairValueRecord : public Table {
    Glyph secondGlyph;                  // GlyphID of second glyph in the pair-first glyph is listed in the Coverage table
    ValueRecord *value1;                // Positioning data for the first glyph in the pair
    ValueRecord *value2;                // Positioning data for the second glyph in the pair

    void preset(ValueFormat valueFormat1, ValueFormat valueFormat2) {
        if (value1 || valueFormat1 != ValueFormat::None) {
            value1->preset(valueFormat1);
        }
        if (value2 || valueFormat2 != ValueFormat::None) {
            value2->preset(valueFormat2);
        }
    }

    void write(Writer &writer) override {
        writer.write(secondGlyph);
        writer.write(value1);
        writer.write(value2);
    }
};

struct PairSetTable : public Table {
    UInt16 pairValueCount;              // Number of PairValueRecords
    PairValueRecord	*pairValueRecord;   // Array of PairValueRecords-ordered by GlyphID of the second glyph

    void preset(ValueFormat valueFormat1, ValueFormat valueFormat2) {
        for (int i = 0; i < pairValueCount; i++) {
            pairValueRecord[i].preset(valueFormat1, valueFormat2);
        }
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(pairValueCount);
        for (int i = 0; i < pairValueCount; i++) {
            writer.write(&pairValueRecord[i]);
        }

        writer.exit();
    }
};

struct Class2Record : public Table {
    ValueRecord *value1;                // Positioning for first glyph-empty if ValueFormat1 = 0
    ValueRecord *value2;                // Positioning for second glyph-empty if ValueFormat2 = 0

    void preset(ValueFormat valueFormat1, ValueFormat valueFormat2) {
        if (value1 || valueFormat1 != ValueFormat::None) {
            value1->preset(valueFormat1);
        }
        if (value2 || valueFormat2 != ValueFormat::None) {
            value2->preset(valueFormat2);
        }
    }

    void write(Writer &writer) override {
        writer.write(value1);
        writer.write(value2);
    }
};

struct Class1Record : public Table {
    Class2Record *class2Record;         // Array of Class2 records-ordered by Class2

    void preset(UInt16 class2Count, ValueFormat valueFormat1, ValueFormat valueFormat2) {
        m_class2Count = class2Count;

        for (int i = 0; i < m_class2Count; i++) {
            class2Record[i].preset(valueFormat1, valueFormat2);
        }
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_class2Count; i++) {
            writer.write(&class2Record[i]);
        }
    }

private:
    UInt16 m_class2Count;
};

struct PairAdjustmentPosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier
    CoverageTable *coverage;            // Offset to Coverage table-from beginning of PairPos subtable-for the first glyph of the pair
    ValueFormat valueFormat1;           // ValueRecord definition-for the first glyph of the pair-may be zero (0)
    ValueFormat valueFormat2;           // ValueRecord definition-for the second glyph of the pair-may be zero (0)

    union {
        struct {
            UInt16 pairSetCount;        // Number of PairSet tables
            PairSetTable *pairSetTable; // Array of offsets to PairSet tables-from beginning of PairPos subtable-ordered by Coverage Index
        } format1;

        struct {
            ClassDefTable *classDef1;   // Offset to ClassDef table-from beginning of PairPos subtable-for the first glyph of the pair
            ClassDefTable *classDef2;   // Offset to ClassDef table-from beginning of PairPos subtable-for the second glyph of the pair
            UInt16 class1Count;         // Number of classes in ClassDef1 table-includes Class0
            UInt16 class2Count;         // Number of classes in ClassDef2 table-includes Class0
            Class1Record *class1Record; // Array of Class1 records-ordered by Class1
        } format2;
    };

    LookupType lookupType() override {
        return LookupType::pPairAdjustment;
    }

    void write(Writer &writer) override {
        switch (posFormat) {
        case 1:
            for (int i = 0; i < format1.pairSetCount; i++) {
                format1.pairSetTable[i].preset(valueFormat1, valueFormat2);
            }

            writer.enter();

            writer.write(posFormat);
            writer.defer(coverage);
            writer.write((UInt16)valueFormat1);
            writer.write((UInt16)valueFormat2);
            writer.write(format1.pairSetCount);
            for (int i = 0; i < format1.pairSetCount; i++) {
                writer.defer(&format1.pairSetTable[i]);
            }

            writer.exit();
            break;

        case 2:
            for (int i = 0; i < format2.class1Count; i++) {
                format2.class1Record[i].preset(format2.class2Count, valueFormat1, valueFormat2);
            }

            writer.enter();

            writer.write(posFormat);
            writer.defer(coverage);
            writer.write((UInt16)valueFormat1);
            writer.write((UInt16)valueFormat2);
            writer.defer(format2.classDef1);
            writer.defer(format2.classDef2);
            writer.write(format2.class1Count);
            writer.write(format2.class2Count);
            for (int i = 0; i < format2.class1Count; i++) {
                writer.write(&format2.class1Record[i]);
            }

            writer.exit();
            break;
        }
    }
};

struct AnchorTable : public Table {
    UInt16 anchorFormat;                // Format identifier-format = 1
    Int16 xCoordinate;                  // Horizontal value-in design units
    Int16 yCoordinate;                  // Vertical value-in design units

    union {
        struct {
            UInt16 anchorPoint;         // Index to glyph contour point
        } format2;

        struct {
            DeviceTable *xDeviceTable;  // Offset to Device table for X coordinate- from beginning of Anchor table (may be NULL)
            DeviceTable	*yDeviceTable;  // Offset to Device table for Y coordinate- from beginning of Anchor table (may be NULL)
        } format3;
    };

    void write(Writer &writer) override {
        switch (anchorFormat) {
        case 1:
            writer.enter();

            writer.write(anchorFormat);
            writer.write((UInt16)xCoordinate);
            writer.write((UInt16)yCoordinate);

            writer.exit();
            break;

        case 2:
            writer.enter();

            writer.write(anchorFormat);
            writer.write((UInt16)xCoordinate);
            writer.write((UInt16)yCoordinate);
            writer.write(format2.anchorPoint);

            writer.exit();
            break;

        case 3:
            writer.enter();

            writer.write(anchorFormat);
            writer.write((UInt16)xCoordinate);
            writer.write((UInt16)yCoordinate);
            writer.defer(format3.xDeviceTable);
            writer.defer(format3.yDeviceTable);

            writer.exit();
            break;
        }
    }
};

struct EntryExitRecord : public Table {
    AnchorTable *entryAnchor;           // Offset to EntryAnchor table-from beginning of CursivePos subtable-may be NULL
    AnchorTable *exitAnchor;            // Offset to ExitAnchor table-from beginning of CursivePos subtable-may be NULL

    void write(Writer &writer) override {
        writer.defer(entryAnchor);
        writer.defer(exitAnchor);
    }
};

struct CursiveAttachmentPosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier-format = 1
    CoverageTable *coverage;            // Offset to Coverage table-from beginning of CursivePos subtable
    UInt16 entryExitCount;              // Number of EntryExit records
    EntryExitRecord *entryExitRecord;   // Array of EntryExit records-in Coverage Index order

    LookupType lookupType() override {
        return LookupType::pCursiveAttachment;
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(posFormat);
        writer.defer(coverage);
        writer.write(entryExitCount);
        for (int i = 0; i < entryExitCount; i++) {
            writer.write(&entryExitRecord[i]);
        }

        writer.exit();
    }
};

struct MarkRecord : public Table {
    UInt16 clazz;                       // Class defined for this mark
    AnchorTable *markAnchor;            // Offset to Anchor table-from beginning of MarkArray table

    void write(Writer &writer) override {
        writer.write(clazz);
        writer.defer(markAnchor);
    }
};

struct MarkArrayTable : public Table {
    UInt16 markCount;                   // Number of MarkRecords
    MarkRecord *markRecord;             // Array of MarkRecords-in Coverage order

    void write(Writer &writer) override {
        writer.enter();

        writer.write(markCount);
        for (int i = 0; i < markCount; i++) {
            writer.write(&markRecord[i]);
        }

        writer.exit();
    }
};

struct BaseRecord : public Table {
    AnchorTable *baseAnchor;            // Array of offsets (one per class) to Anchor tables-from beginning of BaseArray table-ordered by class-zero-based

    void preset(UInt16 classCount) {
        m_classCount = classCount;
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_classCount; i++) {
            writer.defer(&baseAnchor[i]);
        }
    }

private:
    UInt16 m_classCount;
};

struct BaseArrayTable : public Table {
    UInt16 baseCount;                   // Number of BaseRecords
    BaseRecord *baseRecord;             // Array of BaseRecords-in order of BaseCoverage Index

    void preset(UInt16 classCount) {
        for (int i = 0; i < baseCount; i++) {
            baseRecord[i].preset(classCount);
        }
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(baseCount);
        for (int i = 0; i < baseCount; i++) {
            writer.write(&baseRecord[i]);
        }

        writer.exit();
    }
};

struct MarkToBaseAttachmentPosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier-format = 1
    CoverageTable *markCoverage;        // Offset to MarkCoverage table-from beginning of MarkBasePos subtable
    CoverageTable *baseCoverage;        // Offset to BaseCoverage table-from beginning of MarkBasePos subtable
    UInt16 classCount;                  // Number of classes defined for marks
    MarkArrayTable *markArray;          // Offset to MarkArray table-from beginning of MarkBasePos subtable
    BaseArrayTable *baseArray;          // Offset to BaseArray table-from beginning of MarkBasePos subtable

    LookupType lookupType() override {
        return LookupType::pMarkToBaseAttachment;
    }

    void write(Writer &writer) override {
        baseArray->preset(classCount);

        writer.enter();

        writer.write(posFormat);
        writer.defer(markCoverage);
        writer.defer(baseCoverage);
        writer.write(classCount);
        writer.defer(markArray);
        writer.defer(baseArray);

        writer.exit();
    }
};

struct ComponentRecord : public Table {
    AnchorTable *ligatureAnchor;        // Array of offsets (one per class) to Anchor tables-from beginning of LigatureAttach table-ordered by class-NULL if a component does not have an attachment for a class-zero-based array

    void preset(UInt16 classCount) {
        m_classCount = classCount;
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_classCount; i++) {
            writer.defer(&ligatureAnchor[i]);
        }
    }

private:
    UInt16 m_classCount;
};

struct LigatureAttachTable : public Table {
    UInt16 componentCount;              // Number of ComponentRecords in this ligature
    ComponentRecord	*componentRecord;   // Array of Component records-ordered in writing direction

    void preset(UInt16 classCount) {
        for (int i = 0; i < componentCount; i++) {
            componentRecord[i].preset(classCount);
        }
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(componentCount);
        for (int i = 0; i < componentCount; i++) {
            writer.write(&componentRecord[i]);
        }

        writer.exit();
    }
};

struct LigatureArrayTable : public Table {
    UInt16 ligatureCount;               // Number of LigatureAttach table offsets
    LigatureAttachTable	*ligatureAttach;// Array of offsets to LigatureAttach tables-from beginning of LigatureArray table-ordered by LigatureCoverage Index

    void preset(UInt16 classCount) {
        for (int i = 0; i < ligatureCount; i++) {
            ligatureAttach[i].preset(classCount);
        }
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(ligatureCount);
        for (int i = 0; i < ligatureCount; i++) {
            writer.defer(&ligatureAttach[i]);
        }

        writer.exit();
    }
};

struct MarkToLigatureAttachmentPosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier-format = 1
    CoverageTable *markCoverage;        // Offset to Mark Coverage table-from beginning of MarkLigPos subtable
    CoverageTable *ligatureCoverage;    // Offset to Ligature Coverage table-from beginning of MarkLigPos subtable
    UInt16 classCount;                  // Number of defined mark classes
    MarkArrayTable *markArray;          // Offset to MarkArray table-from beginning of MarkLigPos subtable
    LigatureArrayTable *ligatureArray;  // Offset to LigatureArray table-from beginning of MarkLigPos subtable

    LookupType lookupType() override {
        return LookupType::pMarkToLigatureAttachment;
    }

    void write(Writer &writer) override {
        ligatureArray->preset(classCount);

        writer.enter();

        writer.write(posFormat);
        writer.defer(markCoverage);
        writer.defer(ligatureCoverage);
        writer.write(classCount);
        writer.defer(markArray);
        writer.defer(ligatureArray);

        writer.exit();
    }
};

struct Mark2Record : public Table {
    AnchorTable *mark2Anchor;           // Array of offsets (one per class) to Anchor tables-from beginning of Mark2Array table-zero-based array

    void preset(UInt16 classCount) {
        m_classCount = classCount;
    }

    void write(Writer &writer) override {
        for (int i = 0; i < m_classCount; i++) {
            writer.defer(&mark2Anchor[i]);
        }
    }

private:
    UInt16 m_classCount;
};

struct Mark2ArrayTable : public Table {
    UInt16 mark2Count;                  // Number of Mark2 records
    Mark2Record *mark2Record;           // Array of Mark2 records-in Coverage order

    void preset(UInt16 classCount) {
        for (int i = 0; i < mark2Count; i++) {
            mark2Record[i].preset(classCount);
        }
    }

    void write(Writer &writer) override {
        writer.enter();

        writer.write(mark2Count);
        for (int i = 0; i < mark2Count; i++) {
            writer.write(&mark2Record[i]);
        }

        writer.exit();
    }
};

struct MarkToMarkAttachmentPosSubtable : public LookupSubtable {
    UInt16 posFormat;                   // Format identifier-format = 1
    CoverageTable *mark1Coverage;       // Offset to Combining Mark Coverage table-from beginning of MarkMarkPos subtable
    CoverageTable *mark2Coverage;       // Offset to Base Mark Coverage table-from beginning of MarkMarkPos subtable
    UInt16 classCount;                  // Number of Combining Mark classes defined
    MarkArrayTable *mark1Array;         // Offset to MarkArray table for Mark1-from beginning of MarkMarkPos subtable
    Mark2ArrayTable *mark2Array;        // Offset to Mark2Array table for Mark2-from beginning of MarkMarkPos subtable

    LookupType lookupType() override {
        return LookupType::pMarkToMarkAttachment;
    }

    void write(Writer &writer) override {
        mark2Array->preset(classCount);

        writer.enter();

        writer.write(posFormat);
        writer.defer(mark1Coverage);
        writer.defer(mark2Coverage);
        writer.write(classCount);
        writer.defer(mark1Array);
        writer.defer(mark2Array);

        writer.exit();
    }
};

struct GPOS : public Table {
    UInt32 version;                     // Version of the GPOS table-initially = 0x00010000
    ScriptListTable *scriptList;        // Offset to ScriptList table-from beginning of GPOS table
    FeatureListTable *featureList;      // Offset to FeatureList table-from beginning of GPOS table
    LookupListTable *lookupList;        // Offset to LookupList table-from beginning of GPOS table

    void write(Writer &writer) override {
        writer.enter();

        writer.write(version);
        writer.defer(scriptList);
        writer.defer(featureList);
        writer.defer(lookupList);

        writer.exit();
    }
};

}
}
}

#endif
