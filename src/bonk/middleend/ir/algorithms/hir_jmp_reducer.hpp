#include "bonk/middleend/ir/hir.hpp"

namespace bonk {

class HIRJmpReducer {
    public:
    bool reduce(HIRProgram& program);
    bool reduce(HIRProcedure& procedure);
};

}