
#include "bonk/middleend/ir/hir.hpp"
namespace bonk {

class HIRJnzOptimizer {
public:
    bool optimize(HIRProgram& program);
    bool optimize(HIRProcedure& procedure);
};

}