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

#ifndef __SHEEN_FIGURE__TESTER__OPEN_TYPE__BUILDER_H
#define __SHEEN_FIGURE__TESTER__OPEN_TYPE__BUILDER_H

#include <cstddef>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "Common.h"
#include "DataTypes.h"
#include "GSUB.h"
#include "GPOS.h"

namespace SheenFigure {
namespace Tester {
namespace OpenType {

typedef std::list<std::shared_ptr<void>> ObjectPool;

class Builder {
public:
    Builder();
    ~Builder();

    SingleSubstSubtable &createSingleSubst(const std::set<Glyph> glyphs, Int16 delta);
    SingleSubstSubtable &createSingleSubst(const std::map<Glyph, Glyph> glyphs);
    MultipleSubstSubtable &createMultipleSubst(const std::map<Glyph, const std::vector<Glyph>> glyphs);
    LigatureSubstSubtable &createLigatureSubst(const std::map<const std::vector<Glyph>, Glyph> glyphs);

    ChainContextSubtable &createChainContext(const std::vector<const std::vector<Glyph>> backtrack,
                                             const std::vector<const std::vector<Glyph>> input,
                                             const std::vector<const std::vector<Glyph>> lookahead,
                                             const std::vector<const std::pair<UInt16, UInt16>> lookups);

private:
    ObjectPool m_pool;

    template<class T, class... Args>
    T &createObject( Args&&... args);
    template<class T>
    T *createArray(size_t size);

    template<class InputIt, class Operation>
    Glyph *createGlyphs(InputIt begin, InputIt end, Operation operation);
    template<class Collection>
    Glyph *createGlyphs(const Collection &glyphs);

    CoverageTable &createCoverage(Glyph *glyphs, UInt16 count);
};

}
}
}

#endif
