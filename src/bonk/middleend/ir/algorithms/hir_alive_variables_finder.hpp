#pragma once

#include <vector>
#include "utils/dynamic_bitset.hpp"
#include "hir_cfg_walker.hpp"

namespace bonk {

class HIRAliveVariablesFinder {
  public:
    std::vector<DynamicBitSet> use;
    std::vector<DynamicBitSet> define;
    std::vector<DynamicBitSet> out;

    bool walk(bonk::HIRProcedure& procedure);

    DynamicBitSet get_out(bonk::HIRBaseBlock& block);
    DynamicBitSet get_in(bonk::HIRBaseBlock& block);

  private:
    void update_use_define(bonk::HIRBaseBlock& block);
};

}