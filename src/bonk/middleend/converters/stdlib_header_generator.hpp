#pragma once

#include "bonk/middleend/middleend.hpp"

namespace bonk {

struct StdlibFunction {
    MiddleEnd& middle_end;
    std::unique_ptr<TreeNodeBlockDefinition> function;

    StdlibFunction& parameter(std::string_view name, PrimitiveType type);
    StdlibFunction& return_type(PrimitiveType return_type);
    void attach(TreeNodeProgram* program);
};

class StdLibHeaderGenerator {
    MiddleEnd& middle_end;

  public:
    explicit StdLibHeaderGenerator(MiddleEnd& middle_end) : middle_end(middle_end) {
    }

    void generate(TreeNode* ast);

    StdlibFunction generate_stdlib_function(std::string_view name);

};

} // namespace bonk