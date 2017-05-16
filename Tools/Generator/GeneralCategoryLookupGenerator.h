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

#ifndef _SHEEN_FIGURE__GENERATOR__GENERAL_CATEGORY_LOOKUP_GENERATOR_H
#define _SHEEN_FIGURE__GENERATOR__GENERAL_CATEGORY_LOOKUP_GENERATOR_H

#include <map>
#include <memory>
#include <sstream>

#include <Parser/UnicodeData.h>

#include "Utilities/GeneralCategoryDetector.h"

namespace SheenFigure {
namespace Generator {

class GeneralCategoryLookupGenerator {
public:
    GeneralCategoryLookupGenerator(const Parser::UnicodeData &unicodeData);

    void setMainSegmentSize(size_t);
    void setBranchSegmentSize(size_t);

    void displayBidiClassesFrequency();

    void analyzeData();
    void generateFile(const std::string &directory);

private:
    typedef std::vector<uint8_t> UnsafeMainDataSet;
    typedef std::shared_ptr<UnsafeMainDataSet> MainDataSet;

    struct MainDataSegment {
        const size_t index;
        const MainDataSet dataset;

        MainDataSegment(size_t index, MainDataSet dataset);
        const std::string hintLine() const;
    };

    typedef std::vector<MainDataSegment *> UnsafeBranchDataSet;
    typedef std::shared_ptr<UnsafeBranchDataSet> BranchDataSet;

    struct BranchDataSegment {
        const size_t index;
        const BranchDataSet dataset;

        BranchDataSegment(size_t index, BranchDataSet dataset);
        const std::string hintLine() const;
    };

    const Utilities::GeneralCategoryDetector m_generalCategoryDetector;
    const uint32_t m_firstCodePoint;
    const uint32_t m_lastCodePoint;

    size_t m_mainSegmentSize;
    size_t m_branchSegmentSize;

    std::vector<MainDataSegment> m_dataSegments;
    std::vector<MainDataSegment *> m_dataReferences;

    std::vector<BranchDataSegment> m_branchSegments;
    std::vector<BranchDataSegment *> m_branchReferences;

    size_t m_dataSize;
    size_t m_mainIndexesSize;
    size_t m_branchIndexesSize;

    void collectMainData();
    void collectBranchData();
};

}
}

#endif
