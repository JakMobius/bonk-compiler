#pragma

#include <unordered_map>
#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRVariableIndexCompressor {
  public:
    HIRVariableIndexCompressor() {
    }

    bool compress(HIRProgram& program);
    std::unordered_map<bonk::IRRegister, bonk::IRRegister> compress(HIRProcedure& procedure);
};

} // namespace bonk