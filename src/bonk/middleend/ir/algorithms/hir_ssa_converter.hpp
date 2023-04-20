#pragma once

#include <unordered_map>
#include "bonk/middleend/ir/hir.hpp"
#include "hir_dominance_frontier_finder.hpp"
#include "utils/dynamic_bitset.hpp"

namespace bonk {

class HIRSSAConverter {
    struct HIRRenameContext {
        bonk::DynamicBitSet visited;
        std::vector<std::vector<int>> stack;
        std::vector<int> history;
        int counter = 0;
        HIRDominanceFrontierFinder& df_finder;

        explicit HIRRenameContext(HIRDominanceFrontierFinder& df_finder)
            : df_finder(df_finder), visited(df_finder.procedure.base_blocks.size()) {
            for (int i = 0; i < df_finder.procedure.used_registers; i++) {
                stack.emplace_back();
            }
        }

        int new_name(int variable);
        int top(int variable);
        void pop(int count);
    };

  public:
    HIRSSAConverter() {
    }

    void convert(HIRProgram& program);
    void convert(HIRProcedure& procedure);

  private:
    void insert_phi_functions(HIRProcedure& procedure, HIRDominanceFrontierFinder& df_finder);
    void rename_variables(HIRProcedure& procedure, HIRDominanceFrontierFinder& df_finder);
    void rename(bonk::HIRBaseBlock& block, HIRRenameContext& context);
};

} // namespace bonk