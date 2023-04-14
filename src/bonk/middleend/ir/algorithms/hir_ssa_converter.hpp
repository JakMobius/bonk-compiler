#pragma once

#include <unordered_map>
#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRSSAConverter {
  public:
    HIRSSAConverter() {
    }

    void convert(HIRProgram& program);
    void convert(HIRProcedure& procedure);

  private:
    std::unordered_map<bonk::IRRegister, std::vector<bool>>
    get_blocks_set(bonk::HIRProcedure& procedure);
};

} // namespace bonk